
/*
 * Refer to "LegOdometry_LCM_Handler.h" for comments on the purpose of each function and member, comments here in the .cpp relate
 * to more development issues and specifics. The user only need worry about the definitions and descriptions given in the header file,
 * assuming a good software design was done.
 * d fourie
 * 3/24/2013
*/

#include <iostream>
#include <exception>
//#include <stdio.h>
//#include <inttypes.h>

#include "LegOdometry_LCM_Handler.hpp"
#include "QuaternionLib.h"

using namespace TwoLegs;
using namespace std;

LegOdometry_Handler::LegOdometry_Handler(boost::shared_ptr<lcm::LCM> &lcm_, command_switches* commands):
        _finish(false), lcm_(lcm_) {
	// Create the object we want to use to estimate the robot's pelvis position
	// In this case its a two legged vehicle and we use TwoLegOdometry class for this task
	
	_switches = commands;
	filter_joints_vector_size_set = false;
	
	std::cout << "Switches value for listen to LCM trues is: " << _switches->lcm_read_trues << std::endl;
	
	// There used to be polymorphism here, but that soldier was abandoned for an easier and more cumbersome maneuver
	//for (int i=0;i<FILTER_ARR;i++) {_filter[i] = &lpfilter[i];}
	
	 _botparam = bot_param_new_from_server(lcm_->getUnderlyingLCM(), 0);
	 _botframes= bot_frames_get_global(lcm_->getUnderlyingLCM(), _botparam);
	
	first_get_transforms = true;
	ratecounter = 0;
	local_to_head_vel_diff.setSize(3);
	local_to_head_acc_diff.setSize(3);
	local_to_head_rate_diff.setSize(3);
	
	rate_changer.setDesiredPeriod_us(0,4500);


#ifdef LOG_LEG_TRANSFORMS
	for (int i=0;i<4;i++) {
		// left vel, right vel, left rate, right rate
		pelvis_to_feet_speed[i].setSize(3);
	}
	for (int i=0;i<12;i++) {
		pelvis_to_feet_transform[i]=0.; // left vel, right vel, left rate, right rate
	}
#endif

	if (_switches->lcm_add_ext) {
	  _channel_extension = "";
	} else {
		_channel_extension = "_VO";
	}
	
	if(!lcm_->good())
	  return;
	
	model_ = boost::shared_ptr<ModelClient>(new ModelClient(lcm_->getUnderlyingLCM(), 0));
	
#ifdef VERBOSE_DEGUG
	std::cout << "LegOdometry_handler is now subscribing to LCM messages: " << "TRUE_ROBOT_STATE, " << "TORSO_IMU" << std::endl; 
#endif
	
	lcm_->subscribe("TRUE_ROBOT_STATE",&LegOdometry_Handler::robot_state_handler,this);
	if (_switches->do_estimation) {
		lcm_->subscribe("TORSO_IMU",&LegOdometry_Handler::torso_imu_handler,this);
	}

	// TODO -- the logging of joint commands was added quickly and is therefore added as a define based inclusion. if this is to stay, then proper dynamic size coding must be done
#ifdef LOG_28_JOINT_COMMANDS
	lcm_->subscribe("JOINT_COMMANDS", &LegOdometry_Handler::joint_commands_handler,this);
#endif
	
	/*
	if (_switches->lcm_read_trues) {
		// now we can listen to the true values of POSE_HEAD
		lcm_->subscribe("POSE_HEAD_TRUE", &LegOdometry_Handler::pose_head_true_handler, this);
	}
	*/
	
	// Parse KDL tree
	  if (!kdl_parser::treeFromString(  model_->getURDFString() ,tree)){
	    std::cerr << "ERROR: Failed to extract kdl tree from xml robot description" << std::endl;
	    return;
	  }
	  
	  fksolver_ = boost::shared_ptr<KDL::TreeFkSolverPosFull_recursive>(new KDL::TreeFkSolverPosFull_recursive(tree));
	  
	stillbusy = false;
	
	// This is for viewing results in the collections_viewer. check delete of new memory
	lcm_viewer = lcm_create(NULL);
	
	poseplotcounter = 0;
	collectionindex = 101;
	_obj = new ObjectCollection(1, std::string("Objects"), VS_OBJ_COLLECTION_T_POSE3D);
	_obj_leg_poses = new ObjectCollection(1, std::string("Objects"), VS_OBJ_COLLECTION_T_POSE3D);
	_link = new LinkCollection(2, std::string("Links"));
	
	firstpass = 1;//LowPassFilter::getTapSize(); the filter now initializes internally from the first sample -- only a single outside state from the user is required, i.e. transparent
	
	pulse_counter = 0;

	time_avg_counter = 0;
	elapsed_us = 0.;

#ifdef LOG_28_JOINT_COMMANDS
	for (int i=0;i<NUMBER_JOINTS;i++) {
	  joint_commands[i] = 0.;
	}
#endif

	_leg_odo = new TwoLegOdometry(_switches->log_data_files);
//#if defined( DISPLAY_FOOTSTEP_POSES ) || defined( DRAW_DEBUG_LEGTRANSFORM_POSES )
  if (_switches->draw_footsteps) {
	_viewer = new Viewer(lcm_viewer);
  }
	//#endif
    state_estimate_error_log.Open(_switches->log_data_files,"true_estimated_states.csv");
	joint_data_log.Open(_switches->log_data_files,"joint_data.csv");
	return;
}

LegOdometry_Handler::~LegOdometry_Handler() {
	
	  state_estimate_error_log.Close();
	  joint_data_log.Close();


	//delete model_;
	delete _leg_odo;
	delete _obj;
	delete _obj_leg_poses;
	delete _link;
	
	joint_lpfilters.clear();
	
	lcm_destroy(lcm_viewer); //destroy viewer memory at executable end
	delete _viewer;
	
	// Not sure if the pointers must be deleted here, this may try and delete a pointer to a static memory location -- therefore commented
	//delete _botframes;
	//delete _botparam;
	
	cout << "Everything Destroyed in LegOdometry_Handler::~LegOdometry_Handler()" << endl;
	return;
}

void LegOdometry_Handler::setupLCM() {
	
//	_lcm = lcm_create(NULL);
	// TODO
	// robot_pose_channel = "TRUE_ROBOT_STATE";
	// drc_robot_state_t_subscribe(_lcm, robot_pose_channel, TwoLegOdometry::on_robot_state_aux, this);
	
	
	return;
}

void LegOdometry_Handler::InitializeFilters(const int num_filters) {
	
	for (int i=0;i<num_filters;i++) {
		LowPassFilter member;
		joint_lpfilters.push_back(member);
	}
}
			
// obsolete
void LegOdometry_Handler::run(bool testingmode) {
	
	// TODO
	cout << "LegOdometry_Handler::run(bool) is NOT finished yet." << endl;
	
	if (testingmode)
	{
		cout << "LegOdometry_Handler::run(bool) in tesing mode." << endl;
		
		for (int i = 0 ; i<10 ; i++)
		{
			_leg_odo->CalculateBodyStates_Testing(i);
			
		}
		
	}
	else
	{
		cout << "Attempting to start lcm_handle loop..." << endl;
		
		try
		{
			// This is the highest reference point for the
			//This is in main now...
			//while(0 == lcm_->handle());
		    
		}
		catch (exception& e)
		{
			cout << "LegOdometry_Handler::run() - Oops something went wrong when we tried to listen and respond to a new lcm message:" << endl;
			cout << e.what() << endl;
			
		}
	}
	
	return;
}

// To be moved to a better abstraction location
void LegOdometry_Handler::DetermineLegContactStates(long utime, float left_z, float right_z) {
	// The idea here is to determine the contact state of each foot independently
	// to enable better initialization logic when the robot is stating up, or stading up after falling down
	_leg_odo->updateSingleFootContactStates(utime, left_z, right_z);
}

void LegOdometry_Handler::ParseFootForces(const drc::robot_state_t* msg, double &left_force, double &right_force) {
	// TODO -- This must be updated to use naming and not numerical 0 and 1 for left to right foot isolation
	
#ifdef TRY_FOOT_FORCE_MAP_FIND

	// using a map to find the forces each time is not the most efficient way, but its flexible and useful for when we need to change to using forces from the hands when climbing ladders
	// can optimize here if required, but the overhead for this is expected to be reasonable
	map<string, double> foot_forces;
	for (int i=0;i<msg->contacts.num_contacts;i++) {
		foot_forces.insert(make_pair(msg->contacts.id[i], msg->contact_force[i]));
	}

	map<string, double >::iterator contact_lf;
	map<string, double >::iterator contact_rf;

	contact_lf=cartpos_out.find("l_foot");
	contact_rf=cartpos_out.find("r_foot");

	left_force = lpfilter[0].processSample(contact_lf->second);
	right_force = lpfilter[1].processSample(contact_rf->second);

#else

	left_force = lpfilter[0].processSample(msg->contacts.contact_force[0].z);
	right_force = lpfilter[1].processSample(msg->contacts.contact_force[1].z);

#endif

}


void LegOdometry_Handler::robot_state_handler(	const lcm::ReceiveBuffer* rbuf, 
												const std::string& channel, 
												const  drc::robot_state_t* _msg) {

	clock_gettime(CLOCK_REALTIME, &before);
	//std::cout << before.tv_nsec << ", " << spare.tv_nsec << std::endl;
	spare_time = (double)(static_cast<long long>(before.tv_nsec) - static_cast<long long>(spare.tv_nsec));


	// The intention is to build up the information inside these messages and pass them out on LCM to who ever needs to consume them
	// The estimated state message variables are created here for two main reasons:
	// 1. Timing of the messaging sending is slaved to the reception of joint angle measurements
	// 2. After an estimation iteration these state variables go out of scope, preventing stale data to be carried over to the next iteration of this code (short of memory errors -- which must NEVER happen)
	// This is the main core memory of the state estimation process. These values are the single point of interface with the LCM cloud -- method of odometry will have their own state memory,
	// and should always be managed as such.
	drc::robot_state_t est_msgout;
	bot_core::pose_t est_headmsg;
	Eigen::Isometry3d left;
	Eigen::Isometry3d right;
	bool legchangeflag;
	
	int joints_were_updated=0;

	double left_force, right_force;
	ParseFootForces(_msg, left_force, right_force);

	DetermineLegContactStates((long)_msg->utime,left_force,right_force); // should we have a separate foot contact state classifier, which is not embedded in the leg odometry estimation process
	if (_switches->publish_footcontact_states) {
	  PublishFootContactEst(_msg->utime);
	}
	
#ifdef TRUE_ROBOT_STATE_MSG_AVAILABLE
	// maintain a true pelvis position for drawing of the foot
	Eigen::Quaterniond true_pelvis_q(_msg->origin_position.rotation.w, _msg->origin_position.rotation.x, _msg->origin_position.rotation.y, _msg->origin_position.rotation.z);
	Eigen::Isometry3d true_pelvis(true_pelvis_q);
	true_pelvis.translation().x() = _msg->origin_position.translation.x;
	true_pelvis.translation().y() = _msg->origin_position.translation.y;
	true_pelvis.translation().z() = _msg->origin_position.translation.z;
#endif

	// Here we start populating the estimated robot state data
	est_msgout = *_msg;

	if (_switches->do_estimation){
		// Timing profile. This is the midway point
		//clock_gettime(CLOCK_REALTIME, &mid);
		
		int joints_were_updated;
		map<string, double> jointpos_in;
		joints_were_updated = getJoints(_msg, &jointpos_in);

		// Here the rate change is propagated into the rest of the system
		if (joints_were_updated==1) {
			getTransforms_FK(_msg->utime, jointpos_in, left,right);

			// TODO -- Initialization before the VRC..
			if (firstpass>0)
			{
				firstpass--;// = false;
				_leg_odo->ResetWithLeftFootStates(left,right,true_pelvis);
			}

			legchangeflag = _leg_odo->UpdateStates(_msg->utime, left, right, left_force, right_force);
			UpdateHeadStates(&est_msgout, &est_headmsg);

			//clock_gettime(CLOCK_REALTIME, &threequat);

			PublishEstimatedStates(_msg, &est_msgout);
			PublishHeadStateMsgs(&est_headmsg);

	#ifdef TRUE_ROBOT_STATE_MSG_AVAILABLE
			// True state messages will ont be available during the VRC and must be removed accordingly
			PublishPoseBodyTrue(_msg);
	#endif
	#ifdef LOG_28_JOINT_COMMANDS
		   for (int i=0;i<16;i++) {
			   measured_joint_effort[i] = _msg->measured_effort[i];
		   }
	#endif

			if (_switches->log_data_files) {
				LogAllStateData(_msg, &est_msgout);
			}
		}// end of the reduced rate portion

    }//do estimation

	if (_switches->draw_footsteps) {
		DrawDebugPoses(left, right, _leg_odo->getPelvisState(), legchangeflag);
	}

 
   clock_gettime(CLOCK_REALTIME, &after);
   double elapsed;
	/*elapsed = static_cast<long>(mid.tv_nsec) - static_cast<long>(before.tv_nsec);
	elapsed_us = elapsed/1000.;
	std::cout << "0.50, " << elapsed_us << ", ";// << std::endl;
	
	elapsed = static_cast<long>(threequat.tv_nsec) - static_cast<long>(before.tv_nsec);
	elapsed_us = elapsed/1000.;
	std::cout << "0.75, " << elapsed_us << ", ";// << std::endl;
	*/
	
   if (_switches->print_computation_time) {
		elapsed = (double)(static_cast<long long>(after.tv_nsec) - static_cast<long long>(before.tv_nsec));
		elapsed_us += elapsed*1.E-3;
		spare_us += spare_time*1.E-3;
		int time_avg_wind = 1000;
		if (time_avg_counter >= time_avg_wind) {
			elapsed_us = elapsed_us/((double)time_avg_wind);
			spare_us = spare_us/((double)time_avg_wind);
			std::cout << "AVG computation time: [" << elapsed_us << " us]" << std::endl;//, with [" << spare_us << " us] spare" << std::endl;
			spare_us = 0;
			elapsed_us = 0.;
			time_avg_counter = 0;
		}
		time_avg_counter++;
   }

  clock_gettime(CLOCK_REALTIME, &spare);
}

void LegOdometry_Handler::DrawDebugPoses(const Eigen::Isometry3d &left, const Eigen::Isometry3d &right, const Eigen::Isometry3d &true_pelvis, const bool &legchangeflag) {

#ifdef DRAW_DEBUG_LEGTRANSFORM_POSES
	// This adds a large amount of computation by not clearing the list -- not optimal, but not worth fixing at the moment

	DrawLegPoses(left, right, true_pelvis);
	// This sendCollection call will be overwritten by the one below -- moved here after testing of the forward kinematics
	_viewer->sendCollection(*_obj_leg_poses, true);
#endif

	if (legchangeflag)
	{
		//std::cout << "LEGCHANGE\n";
		addIsometryPose(collectionindex,_leg_odo->getPrimaryInLocal());
		collectionindex++;
		addIsometryPose(collectionindex,_leg_odo->getPrimaryInLocal());
		collectionindex++;
		_viewer->sendCollection(*_obj, true);
	}

	_viewer->sendCollection(*_obj_leg_poses, true);

}

void LegOdometry_Handler::PublishEstimatedStates(const drc::robot_state_t * msg, drc::robot_state_t * est_msgout) {
	
	/*
		if (((!pose_initialized_) || (!vo_initialized_))  || (!zheight_initialized_)) {
	    std::cout << "pose or vo or zheight not initialized, refusing to publish EST_ROBOT_STATE\n";
	    return;
	  }
	  */

	drc::position_3d_t origin;
	drc::twist_t twist;
    Eigen::Quaterniond true_q;
    Eigen::Vector3d E_true;
    Eigen::Vector3d E_est;
    bot_core::pose_t pose;
	
	Eigen::Isometry3d currentPelvis   = _leg_odo->getPelvisState();
	Eigen::Vector3d   velocity_states = _leg_odo->getPelvisVelocityStates();
	Eigen::Vector3d   local_rates     = _leg_odo->getLocalFrameRates();
	
	// estimated orientation 
    Eigen::Quaterniond output_q(currentPelvis.linear()); // This is worth checking again
    
    true_q.w() = msg->origin_position.rotation.w;
    true_q.x() = msg->origin_position.rotation.x;
    true_q.y() = msg->origin_position.rotation.y;
    true_q.z() = msg->origin_position.rotation.z;


    pose.utime  =msg->utime;
    
    for (int i=0; i<3; i++) {
      pose.vel[i] =velocity_states(i);
      pose.rotation_rate[i] = local_rates(i);
    }
  
  // True or estimated position
  if (_switches->use_true_z) {
	pose.pos[0] = msg->origin_position.translation.x;
	pose.pos[1] = msg->origin_position.translation.y;
	pose.pos[2] = msg->origin_position.translation.z;

	pose.orientation[0] =true_q.w();
	pose.orientation[1] =true_q.x();
	pose.orientation[2] =true_q.y();
	pose.orientation[3] =true_q.z();

	origin.translation.x = msg->origin_position.translation.x;
	origin.translation.y = msg->origin_position.translation.y;
	origin.translation.z = msg->origin_position.translation.z;

	origin.rotation.w = msg->origin_position.rotation.w;
	origin.rotation.x = msg->origin_position.rotation.x;
	origin.rotation.y = msg->origin_position.rotation.y;
	origin.rotation.z = msg->origin_position.rotation.z;

	twist.linear_velocity.x = msg->origin_twist.linear_velocity.x; //local_to_body_lin_rate_(0);
	twist.linear_velocity.y = msg->origin_twist.linear_velocity.y; //local_to_body_lin_rate_(1);
	twist.linear_velocity.z = msg->origin_twist.linear_velocity.z; //local_to_body_lin_rate_(2);

	twist.angular_velocity.x = msg->origin_twist.angular_velocity.x;
	twist.angular_velocity.y = msg->origin_twist.angular_velocity.y;
	twist.angular_velocity.z = msg->origin_twist.angular_velocity.z;
  } else {
	pose.pos[0] =currentPelvis.translation().x();
	pose.pos[1] =currentPelvis.translation().y();
	pose.pos[2] =currentPelvis.translation().z();

	pose.orientation[0] =output_q.w();
	pose.orientation[1] =output_q.x();
	pose.orientation[2] =output_q.y();
	pose.orientation[3] =output_q.z();

	origin.translation.x = currentPelvis.translation().x();
	origin.translation.y = currentPelvis.translation().y();
	origin.translation.z = currentPelvis.translation().z();

	origin.rotation.w = output_q.w();
	origin.rotation.x = output_q.x();
	origin.rotation.y = output_q.y();
	origin.rotation.z = output_q.z();

	twist.linear_velocity.x = velocity_states(0);//msg->origin_twist.linear_velocity.x;//velocity_states(0);
	twist.linear_velocity.y = velocity_states(1);//msg->origin_twist.linear_velocity.y;//velocity_states(1);
	twist.linear_velocity.z = velocity_states(2);

	twist.angular_velocity.x = local_rates(0);
	twist.angular_velocity.y = local_rates(1);
	twist.angular_velocity.z = local_rates(2);
  }

  // EST is TRUE with sensor estimated position
  
  //msgout = *msg;
  est_msgout->origin_position = origin;
  est_msgout->origin_twist = twist;

  lcm_->publish("EST_ROBOT_STATE" + _channel_extension, est_msgout);
  lcm_->publish("POSE_BODY" + _channel_extension,&pose);

	/*
	// TODO -- remove this pulse train, only for testing
	// now add a 40Hz pulse train to the true robot state
	if (msg->utime*1.E-3 - pulse_time_ >= 2000) {
		pulse_time_ = msg->utime*1.E-3;
		pulse_counter = 16;
	}
	if (pulse_counter>0) {
		origin.translation.x = origin.translation.x + 0.003;
		origin.translation.y = origin.translation.y + 0.003;

		twist.linear_velocity.x = twist.linear_velocity.x + 0.03;
		twist.linear_velocity.y = twist.linear_velocity.y + 0.1;
		pulse_counter--;
	}*/
}

// TODO -- remember that this is to be depreciated
// This function will not be available during the VRC, as the true robot state will not be known
void LegOdometry_Handler::PublishPoseBodyTrue(const drc::robot_state_t * msg) {

	// Infer the Robot's head position from the ground truth root world pose
	bot_core::pose_t pose_msg;
	pose_msg.utime = msg->utime;
	pose_msg.pos[0] = msg->origin_position.translation.x;
	pose_msg.pos[1] = msg->origin_position.translation.y;
	pose_msg.pos[2] = msg->origin_position.translation.z;
	pose_msg.orientation[0] = msg->origin_position.rotation.w;
	pose_msg.orientation[1] = msg->origin_position.rotation.x;
	pose_msg.orientation[2] = msg->origin_position.rotation.y;
	pose_msg.orientation[3] = msg->origin_position.rotation.z;

	lcm_->publish("POSE_BODY_TRUE", &pose_msg);
}

void LegOdometry_Handler::PublishHeadStateMsgs(const bot_core::pose_t * msg) {

  lcm_->publish("POSE_HEAD" + _channel_extension, msg);

  return;
}

void LegOdometry_Handler::UpdateHeadStates(const drc::robot_state_t * msg, bot_core::pose_t * l2head_msg) {

	Eigen::Vector3d local_to_head_vel;
	Eigen::Vector3d local_to_head_acc;
	Eigen::Vector3d local_to_head_rate;

	Eigen::Isometry3d local_to_head;

	int status;
	  double matx[16];
	  Eigen::Isometry3d body_to_head;
	  status = bot_frames_get_trans_mat_4x4_with_utime( _botframes, "head", "body", msg->utime, matx);
	  for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
		  body_to_head(i,j) = matx[i*4+j];
		}
	  }

	  //std::cout << body_to_head.translation().transpose() << " is b2h\n";
	  Eigen::Isometry3d pelvis;
	  Eigen::Quaterniond q(msg->origin_position.rotation.w, msg->origin_position.rotation.x, msg->origin_position.rotation.y, msg->origin_position.rotation.z);
	  // TODO -- remember this flag

	  pelvis.setIdentity();
	  pelvis.translation().x() = msg->origin_position.translation.x;
	  pelvis.translation().y() = msg->origin_position.translation.y;
	  pelvis.translation().z() = msg->origin_position.translation.z;
	  pelvis.rotate(q);

		  //std::cout << truebody.translation().transpose() << " is tb\n";

	  local_to_head = pelvis * body_to_head;
		  //std::cout << local_to_head.translation().transpose() << " is l2h\n\n";

	  // now we need the linear and rotational velocity states -- velocity and accelerations are computed wiht the first order differential

	  local_to_head_vel = local_to_head_vel_diff.diff(msg->utime, local_to_head.translation());
	  local_to_head_acc = local_to_head_acc_diff.diff(msg->utime, local_to_head_vel);
	  local_to_head_rate = local_to_head_rate_diff.diff(msg->utime, InertialOdometry::QuaternionLib::C2e(local_to_head.linear()));

	// estimate the rotational velocity of the head
	Eigen::Quaterniond l2head_rot(local_to_head.linear());

	l2head_msg->utime = msg->utime;

	l2head_msg->pos[0] = local_to_head.translation().x();
	l2head_msg->pos[1] = local_to_head.translation().y();
	l2head_msg->pos[2] = local_to_head.translation().z();

	l2head_msg->orientation[0] = l2head_rot.w();
	l2head_msg->orientation[1] = l2head_rot.x();
	l2head_msg->orientation[2] = l2head_rot.y();
	l2head_msg->orientation[3] = l2head_rot.z();
	l2head_msg->vel[0]=local_to_head_vel(0);
	l2head_msg->vel[1]=local_to_head_vel(1);
	l2head_msg->vel[2]=local_to_head_vel(2);
	l2head_msg->rotation_rate[0]=local_to_head_rate(0);//is this the correct ordering of the roll pitch yaw
	l2head_msg->rotation_rate[1]=local_to_head_rate(1);// Maurice has it the other way round.. ypr
	l2head_msg->rotation_rate[2]=local_to_head_rate(2);
	l2head_msg->accel[0]=local_to_head_acc(0);
	l2head_msg->accel[1]=local_to_head_acc(1);
	l2head_msg->accel[2]=local_to_head_acc(2);

	return;
}

void LegOdometry_Handler::LogAllStateData(const drc::robot_state_t * msg, const drc::robot_state_t * est_msgout) {

  // Logging csv file with true and estimated states
  stringstream ss (stringstream::in | stringstream::out);

  // The true states are
  stateMessage_to_stream(msg, ss);
  stateMessage_to_stream(est_msgout, ss);

  // adding timestamp a bit late, sorry
  ss << msg->utime << ", ";

  // Adding the foot contact forces
  ss << msg->contacts.contact_force[0].z << ", "; // left
  ss << msg->contacts.contact_force[1].z << ", "; // right

  // Active foot is
  ss << (_leg_odo->getActiveFoot() == LEFTFOOT ? "0" : "1") << ", ";

  // The single foot contact states are also written to file for reference -- even though its published by a separate processing using this same class.
  ss << _leg_odo->leftContactStatus() << ", ";
  ss << _leg_odo->rightContactStatus() << ", "; // 30

	#ifdef LOG_28_JOINT_COMMANDS
		for (int i=0;i<NUMBER_JOINTS;i++) {
		  ss << joint_commands[i] << ", "; //31-58
		}

		for (int i=0;i<16;i++) {
			ss << joint_positions[i] << ", "; //59-74
		}

	   for (int i=0;i<16;i++) {
		   ss << measured_joint_effort[i] << ", ";//75-90
	   }
	#endif
	#ifdef LOG_LEG_TRANSFORMS
	   // left vel, right vel, left rate, right rate
	   for (int i=0;i<12;i++) {
		   ss << pelvis_to_feet_transform[i] << ", ";//91-102
	   }
	#endif

   for (int i=0;i<16;i++) {
		ss << filtered_joints[i] << ", "; //103-118
	}

	ss <<std::endl;

	state_estimate_error_log << ss.str();

}

// Push the state values in a drc::robot_state_t message type to the given stringstream
void LegOdometry_Handler::stateMessage_to_stream(const drc::robot_state_t *msg, stringstream &ss) {

	Eigen::Quaterniond q(msg->origin_position.rotation.w, msg->origin_position.rotation.x, msg->origin_position.rotation.y, msg->origin_position.rotation.z);
	Eigen::Vector3d E;

	E = InertialOdometry::QuaternionLib::q2e(q);

	ss << msg->origin_position.translation.x << ", ";
	ss << msg->origin_position.translation.y << ", ";
	ss << msg->origin_position.translation.z << ", ";

	ss << msg->origin_twist.linear_velocity.x << ", ";
	ss << msg->origin_twist.linear_velocity.y << ", ";
	ss << msg->origin_twist.linear_velocity.z << ", ";

	ss << E(0) << ", ";
	ss << E(1) << ", ";
	ss << E(2) << ", ";

	ss << msg->origin_twist.angular_velocity.x << ", ";
	ss << msg->origin_twist.angular_velocity.y << ", ";
	ss << msg->origin_twist.angular_velocity.z << ", ";

	return;
}

void LegOdometry_Handler::torso_imu_handler(	const lcm::ReceiveBuffer* rbuf, 
												const std::string& channel, 
												const  drc::imu_t* msg) {
	
	double rates[3];
	double angles[3];
	Eigen::Quaterniond q(msg->orientation[0],msg->orientation[1],msg->orientation[2],msg->orientation[3]);
	
	Eigen::Vector3d E;
	E = InertialOdometry::QuaternionLib::q2e(q);
	
	for (int i=0;i<3;i++) {
	  rates[i] = lpfilter[i+2].processSample(msg->angular_velocity[i]); // +2 since the foot force values use the first two filters
	  angles[i] = lpfilter[i+5].processSample(E(i));
	}
	
	//Eigen::Vector3d rates_b(msg->angular_velocity[0],msg->angular_velocity[1],msg->angular_velocity[2]);
	Eigen::Vector3d rates_b(rates[0], rates[1], rates[2]);
	q = InertialOdometry::QuaternionLib::e2q(E);
			
	_leg_odo->setOrientationTransform(q, rates_b);
	
	return;
}

#ifdef LOG_28_JOINT_COMMANDS
void LegOdometry_Handler::joint_commands_handler(const lcm::ReceiveBuffer* rbuf, const std::string& channel, const  drc::joint_command_t* msg) {
	// TODO -- 28 actuated joints are hard coded, this must be changed -- but subject to the LOG_28_JOINT_COMMANDS define for the time being
	for (int i=0;i<NUMBER_JOINTS;i++) {
	  joint_commands[i] = msg->effort[i];
	}
}
#endif

/*
void LegOdometry_Handler::pose_head_true_handler(	const lcm::ReceiveBuffer* rbuf, 
													const std::string& channel, 
													const bot_core_pose_t* msg) {

	
}
*/

void LegOdometry_Handler::PublishFootContactEst(int64_t utime) {
	drc::foot_contact_estimate_t msg_contact_est;
	
	msg_contact_est.utime = utime;
	
	// TODO -- Convert this to use the enumerated types from inside the LCM message
	msg_contact_est.detection_method = DIFF_SCHMITT_WITH_DELAY;
	
	msg_contact_est.left_contact = _leg_odo->leftContactStatus();
	msg_contact_est.right_contact = _leg_odo->rightContactStatus();
	
	lcm_->publish("FOOT_CONTACT_ESTIMATE",&msg_contact_est);
}

void LegOdometry_Handler::drawLeftFootPose() {
	//LinkCollection link(2, std::string("Links"));
	
	//addIsometryPose(_leg_odo->getLeftInLocal());
	
	//addIsometryPose(98, _leg_odo->left_to_pelvis);
	
	//TODO - male left_to_pelvis and other private members in TwoLegOdometry class with get functions of the same name, as is done with Eigen::Isometry3d .translation() and .rotation()
	
	addIsometryPose(97, _leg_odo->left_to_pelvis);
	addIsometryPose(98, _leg_odo->left_to_pelvis);
	
	addIsometryPose(87, _leg_odo->pelvis_to_left);
	addIsometryPose(88, _leg_odo->pelvis_to_left);
	
	//InertialOdometry::QuaternionLib::printEulerAngles("drawLeftFootPose()", _leg_odo->pelvis_to_left);
			
}

void LegOdometry_Handler::drawRightFootPose() {
	//LinkCollection link(2, std::string("Links"));
	
	//addIsometryPose(_leg_odo->getLeftInLocal());
	
	
	addIsometryPose(99,_leg_odo->right_to_pelvis);
	addIsometryPose(100,_leg_odo->right_to_pelvis);
	addIsometryPose(89,_leg_odo->pelvis_to_right);
	addIsometryPose(90,_leg_odo->pelvis_to_right);
	
	//std::cout << "adding right foot pose" << std::endl;
}

void LegOdometry_Handler::drawSumPose() {
	//addIsometryPose(95,_leg_odo->add(_leg_odo->left_to_pelvis,_leg_odo->pelvis_to_right));
	//addIsometryPose(96,_leg_odo->add(_leg_odo->left_to_pelvis,_leg_odo->pelvis_to_right));
	
	addIsometryPose(93,_leg_odo->getSecondaryInLocal());
	addIsometryPose(94,_leg_odo->getSecondaryInLocal());
}


void LegOdometry_Handler::addIsometryPose(int objnumber, const Eigen::Isometry3d &target) {
  
  Eigen::Vector3d E;
  InertialOdometry::QuaternionLib::q2e(Eigen::Quaterniond(target.linear()),E);
  _obj->add(objnumber, isam::Pose3d(target.translation().x(),target.translation().y(),target.translation().z(),E(2),E(1),E(0)));
}

// Four Isometries must be passed -- representing pelvisto foot and and foot to pelvis transforms
void LegOdometry_Handler::DrawLegPoses(const Eigen::Isometry3d &left, const Eigen::Isometry3d &right, const Eigen::Isometry3d &true_pelvis) {
  
	Eigen::Isometry3d target[4];

	target[0] = left;
	target[1] = right;
	target[2] = left.inverse();
	target[3] = right.inverse();

  Eigen::Vector3d E;
  Eigen::Isometry3d added_vals[2];
  
  Eigen::Isometry3d back_from_feet[2];
  
  //clear the list to prevent memory growth
  _obj_leg_poses->clear();
	
  for (int i=0;i<2;i++) {
    added_vals[i] = TwoLegOdometry::add(true_pelvis, target[i]); // this is the same function that is used by TwoLegOdometry to accumulate Isometry transforms
    InertialOdometry::QuaternionLib::q2e(Eigen::Quaterniond(added_vals[i].linear()),E);
    _obj_leg_poses->add(50+i, isam::Pose3d(added_vals[i].translation().x(),added_vals[i].translation().y(),added_vals[i].translation().z(),E(2),E(1),E(0)));
    
    back_from_feet[i] = TwoLegOdometry::add(added_vals[i], target[i+2]);
    InertialOdometry::QuaternionLib::q2e(Eigen::Quaterniond(back_from_feet[i].linear()),E);
    _obj_leg_poses->add(50+i+2, isam::Pose3d(back_from_feet[i].translation().x(),back_from_feet[i].translation().y(),back_from_feet[i].translation().z(),E(2),E(1),E(0)));
  }
  
}

// this function may be depreciated soon
void LegOdometry_Handler::addFootstepPose_draw() {
	std::cout << "Drawing pose for foot: " << (_leg_odo->getActiveFoot() == LEFTFOOT ? "LEFT" : "RIGHT") << std::endl; 
	_obj->add(collectionindex, isam::Pose3d(_leg_odo->getPrimaryInLocal().translation().x(),_leg_odo->getPrimaryInLocal().translation().y(),_leg_odo->getPrimaryInLocal().translation().z(),0,0,0));	
	collectionindex = collectionindex + 1;
}

int LegOdometry_Handler::getJoints(const drc::robot_state_t * msg, map<string, double> *_jointpos_in) {
  
  if (filtered_joints.capacity() != msg->num_joints || !filter_joints_vector_size_set) {
	  filter_joints_vector_size_set = true;
	  filtered_joints.resize(msg->num_joints);
	  std::cout << "Automatically changing the capacity of the filtered joints\n";
  }

  // 1. Solve for Forward Kinematics:
	_link_tfs.clear();

	// call a routine that calculates the transforms the joint_state_t* msg.


	if (first_get_transforms) {
		first_get_transforms = false;
		InitializeFilters((int)msg->num_joints);
	}
	//stringstream ss (stringstream::in | stringstream::out);

	double alljoints[msg->num_joints];


	for (uint i=0; i< (uint) msg->num_joints; i++) {
		// Keep joint positions in local memory
		alljoints[i] = msg->joint_position[i];
	}

	int updatedjoints;

	updatedjoints = filterJointPositions(msg->utime, msg->num_joints, alljoints);
	// The filtered joint positions have been placed back in all joints variable

	/*
	for (int i=0;i<msg->num_joints;i++) {
		filtered_joints[i] = alljoints[i];
	}
	*/

	for (uint i=0; i< (uint) msg->num_joints; i++) { //cast to uint to suppress compiler warning
	  // TODO -- This is to be generalized
	  if (i<16) {
		  joint_positions[i] = msg->joint_position[i];
	  }

	  // want to filter the joint position measurements here
	  // The idea is to use the integral and derivative trick here
	  // a new function is to be created for this

	  switch (3) {
		  case 1:
			_jointpos_in->insert(make_pair(msg->joint_name[i], joint_lpfilters.at(i).processSample(msg->joint_position[i])));
			break;
		  case 2:
			// not using filters on the joint position measurements
			_jointpos_in->insert(make_pair(msg->joint_name[i], msg->joint_position[i]));//skipping the filters
			break;
		  case 3:
			_jointpos_in->insert(make_pair(msg->joint_name[i], filtered_joints[i])); // The rate has been reduced to sample periods greater than 4500us and filtered with integral/rate/diff
			break;
	  }
	}

	return updatedjoints;
}

void LegOdometry_Handler::getTransforms_FK(const unsigned long long &u_ts, const map<string, double> &jointpos_in, Eigen::Isometry3d &left, Eigen::Isometry3d &right) {

	bool kinematics_status;
	bool flatten_tree=true; // determines absolute transforms to robot origin, otherwise relative transforms between joints.

	map<string, drc::transform_t > cartpos_out;

	kinematics_status = fksolver_->JntToCart(jointpos_in,cartpos_out,flatten_tree);
    
    //bot_core::rigid_transform_t tf;
    //KDL::Frame T_body_head;
    
    map<string, drc::transform_t >::iterator transform_it_lf;
    map<string, drc::transform_t >::iterator transform_it_rf;
    
    transform_it_lf=cartpos_out.find("l_foot");
    transform_it_rf=cartpos_out.find("r_foot");
    
    //T_body_head = KDL::Frame::Identity();
	  if(transform_it_lf!=cartpos_out.end()){// fk cart pos exists
		// This gives us the translation from body to left foot
		  /*
#ifdef VERBOSE_DEBUG
	    std::cout << " LEFT: " << transform_it_lf->second.translation.x << ", " << transform_it_lf->second.translation.y << ", " << transform_it_lf->second.translation.z << std::endl;
#endif
	    
	    //std::cout << "ROTATION.x: " << transform_it->second.rotation.x << ", " << transform_it->second.rotation.y << std::endl;
	     *
	     */
	  }else{
	    std::cout<< "fk position does not exist" <<std::endl;
	  }
	  
	  if(transform_it_lf!=cartpos_out.end()){// fk cart pos exists
/*
#ifdef VERBOSE_DEBUG
  	    std::cout << "RIGHT: " << transform_it_rf->second.translation.x << ", " << transform_it_rf->second.translation.y << ", " << transform_it_rf->second.translation.z << std::endl;
#endif
  	    transform_it_rf->second.rotation;
  	    */
	  }else{
        std::cout<< "fk position does not exist" << std::endl;
  	  }

	  Eigen::Vector3d E_;
	  
	  // quaternion scale and vector ordering seems to be correct
	  Eigen::Quaterniond  leftq(transform_it_lf->second.rotation.w, transform_it_lf->second.rotation.x,transform_it_lf->second.rotation.y,transform_it_lf->second.rotation.z);
	  Eigen::Quaterniond rightq(transform_it_rf->second.rotation.w, transform_it_rf->second.rotation.x,transform_it_rf->second.rotation.y,transform_it_rf->second.rotation.z);
	  
	  //Eigen::Quaterniond tempq;
	  //Eigen::Matrix<double,3,3> leftC, rightC;
	  //tempq.setIdentity();
	  
	  if (false) {

		  left.translation().x() = transform_it_lf->second.translation.x;
		  left.translation().y() = transform_it_lf->second.translation.y;
		  left.translation().z() = transform_it_lf->second.translation.z;

		  right.translation().x() = transform_it_rf->second.translation.x;
		  right.translation().y() = transform_it_rf->second.translation.y;
		  right.translation().z() = transform_it_rf->second.translation.z;

		  left.rotate(leftq); // with quaternion
		  right.rotate(rightq);

		  //left.rotate(leftC); // with rotation matrix
	  } else {
		  left.translation() << transform_it_lf->second.translation.x, transform_it_lf->second.translation.y, transform_it_lf->second.translation.z;
		  right.translation() << transform_it_rf->second.translation.x, transform_it_rf->second.translation.y, transform_it_rf->second.translation.z;

		  // TODO -- confirm the use of transpose() convert the rotation matrix into the correct frae, as this may be in the q2C function..
		  left.linear() = InertialOdometry::QuaternionLib::q2C(leftq).transpose(); // note Isometry3d.rotation() is still marked as "experimental"
		  right.linear() = InertialOdometry::QuaternionLib::q2C(rightq).transpose();
	  }



#ifdef LOG_LEG_TRANSFORMS
	  // The idea here is to push all the required data to a single array [pelvis_to_feet_transform], which is to be logged in publishstate method

	  Eigen::Vector3d tempvar;
	  int i;

	  tempvar = pelvis_to_feet_speed[0].diff(u_ts,left.translation());
	  // left vel, right vel, left rate, right rate
	  for (i=0;i<3;i++) {
		  pelvis_to_feet_transform[i] = tempvar(i); // left vel, right vel, left rate, right rate
	  }
	  tempvar = pelvis_to_feet_speed[1].diff(u_ts,right.translation());
	  // left vel, right vel, left rate, right rate
	  for (i=0;i<3;i++) {
		  pelvis_to_feet_transform[3+i] = tempvar(i); // left vel, right vel, left rate, right rate
	  }
	  tempvar = pelvis_to_feet_speed[2].diff(u_ts,InertialOdometry::QuaternionLib::C2e(left.rotation()));
	  // left vel, right vel, left rate, right rate
	  for (i=0;i<3;i++) {
		  pelvis_to_feet_transform[6+i] = tempvar(i); // left vel, right vel, left rate, right rate
	  }
	  tempvar = pelvis_to_feet_speed[3].diff(u_ts,InertialOdometry::QuaternionLib::C2e(right.rotation()));
	  // left vel, right vel, left rate, right rate
	  for (i=0;i<3;i++) {
		  pelvis_to_feet_transform[9+i] = tempvar(i); // left vel, right vel, left rate, right rate
	  }

#endif


	  // Matt says
	  // try sysprof (apt-get install sysprof) system profiler
	  // valgrind (memory leaks, memory bounds checking)
	  // valgrind --leak-check=full --track-origin=yes --output-fil=path_to_output.txt
	  

}

int LegOdometry_Handler::filterJointPositions(const unsigned long long &ts, const int &num_joints, double alljoints[]) {
	// we also need to consider the rate change. This will have to happen in or around this function
	int returnval=0;
	Eigen::VectorXd int_vals(num_joints);
	Eigen::VectorXd diff_vals(num_joints);

	std::cout << " | " << std::fixed << alljoints[5];
	// Integrate to lose noise, but keep information
	int_vals = joint_integrator.integrate(ts, num_joints, alljoints);
	std::cout << " i: " << int_vals(5);


	// we are looking for a 200Hz process -- 5ms
	if (rate_changer.checkNewRateTrigger(ts)) {
		diff_vals = joint_pos_filter.diff(ts, int_vals);
		//joint_integrator.reset_in_time();

		for (int i=0;i<num_joints;i++) {
			filtered_joints[i] = diff_vals(i);
		}

		std::cout << ", after: " << filtered_joints[5] << "\n";
		returnval  = 1;
	}

	return returnval;
}

void LegOdometry_Handler::terminate() {
	std::cout << "Closing and cleaning out LegOdometry_Handler object\n";
	
	_leg_odo->terminate();
}

