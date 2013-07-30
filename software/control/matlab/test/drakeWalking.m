function drakeWalking(use_mex,use_bullet)

addpath(fullfile(getDrakePath,'examples','ZMP'));

num_steps = 5; % steps taken by robot
step_length = 0.5;
step_time = 1.0;

% construct robot model
options.floating = true;
options.dt = 0.002;
if (nargin>0) options.use_mex = use_mex;
else options.use_mex = true; end
if (nargin<2) 
  use_bullet = false; % test walking with the controller computing pairwise contacts using bullet
end

r = Atlas(strcat(getenv('DRC_PATH'),'/models/mit_gazebo_models/mit_robot_drake/model_minimal_contact_point_hands.urdf'),options);
r = removeCollisionGroupsExcept(r,{'heel','toe'});
r = compile(r);

% set initial state to fixed point
load(strcat(getenv('DRC_PATH'),'/control/matlab/data/atlas_fp.mat'));
xstar(1) = 0*randn();
xstar(2) = 0*randn();
r = r.setInitialState(xstar);

if use_bullet
  r_bullet = RigidBodyManipulator();
  r_bullet = addRobotFromURDF(r_bullet,strcat(getenv('DRC_PATH'),'/models/mit_gazebo_models/mit_robot_drake/model_minimal_contact_point_hands.urdf'),[0;0;0],[0;0;0],options);
  r_bullet = addRobotFromURDF(r_bullet,strcat(fullfile(getDrakePath,'systems','plants','test'),'/ground_plane.urdf'),[xstar(1);xstar(2);0],zeros(3,1),struct('floating',false));
  r_bullet = TimeSteppingRigidBodyManipulator(r_bullet,options.dt,options);
  r_bullet = removeCollisionGroupsExcept(r_bullet,{'heel','toe'});
  r_bullet = compile(r_bullet);
end

v = r.constructVisualizer;
v.display_dt = 0.05;

nq = getNumDOF(r);
nu = getNumInputs(r);

x0 = xstar;
q0 = x0(1:nq);
kinsol = doKinematics(r,q0);

% create desired ZMP trajectory
[zmptraj,lfoottraj,rfoottraj,support_times,supports] = ZMPandFootTrajectory(r,q0,num_steps,step_length,step_time);
zmptraj = setOutputFrame(zmptraj,desiredZMP);


if use_bullet
  for i=1:length(supports)
    supports{i}=supports{i}.setContactSurfaces(-ones(length(supports{i}.bodies),1));
  end
end

% construct ZMP feedback controller
com = getCOM(r,kinsol);
limp = LinearInvertedPendulum(com(3));
% get COM traj from desired ZMP traj
[c,V,comtraj] = ZMPtracker(limp,zmptraj,struct('use_tvlqr',false,'com0',com(1:2)));

ts = 0:0.1:zmptraj.tspan(end);
T = ts(end);

figure(2); 
clf; 
subplot(3,1,1); hold on;
fnplt(zmptraj(1));
fnplt(comtraj(1));
subplot(3,1,2); hold on;
fnplt(zmptraj(2));
fnplt(comtraj(2));
subplot(3,1,3); hold on;
fnplt(zmptraj);
fnplt(comtraj);

link_constraints(1) = struct('link_ndx', r.findLinkInd('r_foot'), 'pt', [0;0;0], 'min_traj', [], 'max_traj', [], 'traj', rfoottraj);
link_constraints(2) = struct('link_ndx', r.findLinkInd('l_foot'), 'pt', [0;0;0], 'min_traj', [], 'max_traj', [], 'traj', lfoottraj);

% compute s1,s2 derivatives for controller Vdot computation
s1dot = fnder(V.s1,1);
s2dot = fnder(V.s2,1);

ctrl_data = SharedDataHandle(struct(...
  'A',[zeros(2),eye(2); zeros(2,4)],...
  'B',[zeros(2); eye(2)],...
  'C',[eye(2),zeros(2)],...
  'Qy',eye(2),...
  'R',zeros(2),...
  'is_time_varying',true,...
  'S',V.S.eval(0),... % always a constant
  's1',V.s1,...
  's2',V.s2,...
  's1dot',s1dot,...
  's2dot',s2dot,...
  'x0',[zmptraj.eval(T);0;0],...
  'u0',zeros(2,1),...
  'comtraj',comtraj,...
  'link_constraints',link_constraints, ...
  'support_times',support_times,...
  'supports',[supports{:}],...
  'mu',1,...
  'ignore_terrain',false,...
  'y0',zmptraj));

% instantiate QP controller
options.dt = 0.004;
options.slack_limit = 30.0;
options.w = 0.01;
options.lcm_foot_contacts = false;
options.debug = false;

if use_bullet
  options.multi_robot = r_bullet;
end
qp = QPControlBlock(r,ctrl_data,options);
clear options;

sys = r;


% feedback QP controller with atlas
ins(1).system = 1;
ins(1).input = 1;
outs(1).system = 2;
outs(1).output = 1;
sys = mimoFeedback(qp,sys,[],[],ins,outs);
clear ins outs;

% feedback PD block 
pd = WalkingPDBlock(r,ctrl_data);
ins(1).system = 1;
ins(1).input = 1;
outs(1).system = 2;
outs(1).output = 1;
sys = mimoFeedback(pd,sys,[],[],ins,outs);
clear ins outs;

qt = QTrajEvalBlock(r,ctrl_data);
outs(1).system = 2;
outs(1).output = 1;
sys = mimoFeedback(qt,sys,[],[],[],outs);

S=warning('off','Drake:DrakeSystem:UnsupportedSampleTime');
output_select(1).system=1;
output_select(1).output=1;
sys = mimoCascade(sys,v,[],[],output_select);
warning(S);
traj = simulate(sys,[0 T],x0);
playback(v,traj,struct('slider',true));

err = 0; % x,y error
for i=1:length(ts)
  x=traj.eval(ts(i));
  q=x(1:getNumDOF(r)); 
  com(:,i)=getCOM(r,q);
  err = err + sum(abs(comtraj.eval(ts(i)) - com(1:2,i)));
end

figure(2);
subplot(3,1,1);
plot(ts,com(1,:),'r');
subplot(3,1,2);
plot(ts,com(2,:),'r');
subplot(3,1,3); hold on;
plot(com(1,:),com(2,:),'r');

err
if err > num_steps*0.5
  error('drakeWalking unit test failed: error is too large');
end

end