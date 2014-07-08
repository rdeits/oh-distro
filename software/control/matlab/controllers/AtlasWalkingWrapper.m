classdef AtlasWalkingWrapper < DrakeSystem
  properties
    nq;
    nu;
    robot;
    input_map;
    controller_data;
    foot_contact_block;
    pd_plus_qp_block;
    velocity_int_block;
    qtraj_eval_block;
    footstep_plan_shift_block;
  end
  
  methods
    function obj = AtlasWalkingWrapper(r,controller_data,options)
      typecheck(r,'Atlas');
      typecheck(controller_data,'AtlasQPControllerData');
      
      input_frame = getStateFrame(r);
      output_frame = AtlasPosVelTorqueRef(r);
      
      force_controlled_joints = controller_data.force_controlled_joints;
      position_controlled_joints = controller_data.position_controlled_joints;

      gains = getAtlasGains();
      gains.k_q_p(force_controlled_joints) = 0;
      gains.k_q_i(force_controlled_joints) = 0;
      gains.k_qd_p(force_controlled_joints) = 0;
      gains.k_f_p(position_controlled_joints) = 0;
      gains.ff_f_d(position_controlled_joints) = 0;
      gains.ff_qd(position_controlled_joints) = 0;
      gains.ff_qd_d(position_controlled_joints) = 0;

      output_frame.updateGains(gains);
      
      obj = obj@DrakeSystem(0,0,input_frame.dim,output_frame.dim,true,true);
      obj = setInputFrame(obj,input_frame);
      obj = setOutputFrame(obj,output_frame);

      obj.nq = getNumDOF(r);
      obj.nu = getNumInputs(r);

      if nargin<3
        options = struct();
      end
      
      if isfield(options,'dt')
        typecheck(options.dt,'double');
        sizecheck(options.dt,[1 1]);
        dt = options.dt;
      else
        dt = 0.001;
      end
      obj = setSampleTime(obj,[dt;0]); % sets controller update rate
      

      % instantiate QP controller
      options.slack_limit = 30;
      options.w_qdd = 0.001*ones(obj.nq,1);
      options.W_kdot = 0*eye(3);
      options.w_grf = 0.0;
      options.w_slack = 0.005;
      options.debug = true;
      options.use_mex = true;
      options.contact_threshold = 0.01;
      options.output_qdd = true;
      options.solver = 0; % 0 fastqp, 1 gurobi
      options.input_foot_contacts = true;

      qp = QPController(r,{},controller_data,options);

      % cascade IK/PD block
      options.Kp = 50.0*ones(obj.nq,1);
      options.Kd = 8*ones(obj.nq,1);
      % options.Kp(findJointIndices(r,'hpz')) = 70.0;
      % options.Kd(findJointIndices(r,'hpz')) = 14.0;
      % options.Kd(findJointIndices(r,'kny')) = 13.0;
      % options.Kp(3) = 30.0;
      % options.Kd(3) = 12.0;
      % options.Kp(4:5) = 30.0;
      % options.Kd(4:5) = 12.0;
      % options.Kp(6) = 40.0;
      % options.Kd(6) = 12.0;

      pd = IKPDBlock(r,controller_data,options);
      ins(1).system = 1;
      ins(1).input = 1;
      ins(2).system = 1;
      ins(2).input = 2;
      ins(3).system = 1;
      ins(3).input = 3;
      ins(4).system = 2;
      ins(4).input = 1;
      ins(5).system = 2;
      ins(5).input = 3;
      outs(1).system = 2;
      outs(1).output = 1;
      outs(2).system = 2;
      outs(2).output = 2;
      obj.pd_plus_qp_block = mimoCascade(pd,qp,[],ins,outs);
      clear ins;
      
      options.use_error_integrator = true; % while we're still using position control in upper body
      obj.qtraj_eval_block = QTrajEvalBlock(r,controller_data,options);
      options.use_lcm = true;
      obj.foot_contact_block = FootContactBlock(r,controller_data,options);
      options.zero_ankles_on_contact = true;
      obj.velocity_int_block = VelocityOutputIntegratorBlock(r,options);
      obj.footstep_plan_shift_block = FootstepPlanShiftBlock(r,controller_data);

      controller_data.qd_int_state = zeros(obj.velocity_int_block.getStateFrame.dim,1);
      
      obj.robot = r;
      obj.input_map = getActuatedJoints(r);
      obj.controller_data = controller_data;
    end
   
    function y=output(obj,t,~,x)
      persistent pelvis_angv_filt
      
      if isempty(pelvis_angv_filt)
        pelvis_angv_filt = zeros(3,1);
      end
      
      % TODO: THIS IS TEMPORARY UNTIL MAURICE PUTS IT IN STATE SYNC
      alpha = 0.75;
      pelvis_angv_filt = alpha*pelvis_angv_filt + (1-alpha)*x(obj.nq+(4:6));
      x(obj.nq+(4:6)) = pelvis_angv_filt;
      
      % foot contact
      fc = output(obj.foot_contact_block,t,[],x);

      % footstep plan shift
      junk = output(obj.footstep_plan_shift_block,t,[],x);
      
      % qtraj eval
      q_des_and_x = output(obj.qtraj_eval_block,t,[],x);
      q_des = q_des_and_x(1:obj.nq);
      
      % IK/QP
      u_and_qdd = output(obj.pd_plus_qp_block,t,[],[q_des; x; fc; x; fc]);
      u=u_and_qdd(1:obj.nu);
      qdd=u_and_qdd(obj.nu+(1:obj.nq));

      % velocity integrator
      qd_int_state = obj.controller_data.qd_int_state;
      qd_int_state = mimoUpdate(obj.velocity_int_block,t,qd_int_state,x,qdd,fc);
      obj.controller_data.qd_int_state = qd_int_state;
      qd_err = mimoOutput(obj.velocity_int_block,t,qd_int_state,x,qdd,fc);
    
      force_ctrl_joints = obj.controller_data.force_controlled_joints;
      qddes = 0*qd_err;
      qddes(force_ctrl_joints) = qd_err(force_ctrl_joints);
      udes = 0*u;
      udes(force_ctrl_joints) = u(force_ctrl_joints);
 
      y = [q_des(obj.input_map); qddes; udes];
    end
  end
  
end