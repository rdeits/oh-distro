#ifndef LCM2ROSCONTROL_HPP
#define LCM2ROSCONTROL_HPP

#include <hardware_interface/joint_command_interface.h>
#include <hardware_interface/imu_sensor_interface.h>
#include <hardware_interface/force_torque_sensor_interface.h>
#include <controller_interface/controller.h>
#include <pluginlib/class_list_macros.h>
#include <ros/node_handle.h>

#include <lcm/lcm-cpp.hpp>
#include "lcmtypes/drc/joint_state_t.hpp"
#include "lcmtypes/drc/joint_angles_t.hpp"
#include "lcmtypes/drc/force_torque_t.hpp"
#include "lcmtypes/drc/six_axis_force_torque_array_t.hpp"
#include "lcmtypes/drc/six_axis_force_torque_t.hpp"
#include "lcmtypes/drc/atlas_command_t.hpp"
#include "lcmtypes/drc/robot_state_t.hpp"

#include "lcmtypes/drc/ins_t.hpp"

namespace valkyrie_translator
{

   typedef struct _joint_command {
    double position;
    double velocity;
    double effort;

    double k_q_p; // corresponds to kp_position in drcsim API
    double k_q_i; // corresponds to ki_position in drcsim API
    double k_qd_p; // corresponds to kp_velocity in drcsim API
    double k_f_p;
    double ff_qd; // maps to kd_position in drcsim API (there isnt an equivalent gain in the bdi api)
    double ff_qd_d;
    double ff_f_d;
    double ff_const;
   } joint_command;

   class LCM2ROSControl;

   /* Manages subscription for the LCM2ROSControl class. 
      Presently just a hack to resolve pluginlib compatibility issues... */
   class LCM2ROSControl_LCMHandler
   {
   public:
        LCM2ROSControl_LCMHandler(LCM2ROSControl& parent);
        virtual ~LCM2ROSControl_LCMHandler();
        void jointCommandHandler(const lcm::ReceiveBuffer* rbuf, const std::string &channel,
                               const drc::atlas_command_t* msg);
        void update();
   private:
        LCM2ROSControl& parent_;
        // If the subscription is created on LCM2ROSControl's lcm_ object, we see pluginlib
        // compatibility problems. Mysterious and scary...
        std::shared_ptr<lcm::LCM> lcm_;
   };
   
   class LCM2ROSControl : public controller_interface::Controller<hardware_interface::EffortJointInterface>
   {
   public:
        LCM2ROSControl();
        virtual ~LCM2ROSControl();

        void starting(const ros::Time& time);
        void update(const ros::Time& time, const ros::Duration& period);
        void stopping(const ros::Time& time);
        
        // Public so it can be modified by the LCMHandler. Should eventually create
        // a friend class arrangement to make this private again.
        std::map<std::string, joint_command> latest_commands;

   protected:        
        virtual bool initRequest(hardware_interface::RobotHW* robot_hw, 
                         ros::NodeHandle& root_nh, ros::NodeHandle& controller_nh, 
                         std::set<std::string>& claimed_resources) override;

   private:
        boost::shared_ptr<lcm::LCM> lcm_;
        std::shared_ptr<LCM2ROSControl_LCMHandler> handler_;

        std::map<std::string, hardware_interface::JointHandle> effortJointHandles;
        std::map<std::string, hardware_interface::ImuSensorHandle> imuSensorHandles;
        std::map<std::string, hardware_interface::ForceTorqueSensorHandle> forceTorqueHandles;

        ros::Time last_update;
   };
}
#endif