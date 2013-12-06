'''
Created on Nov 13, 2013

@author: Twan, Maurice
'''

import time, math

import rospy
from handle_msgs.msg import HandleControl
from handle_msgs.msg import HandleSensors
from mit_helios_scripts.msg import MITIRobotHandState
from std_msgs.msg import Empty


from IRobotHandConfigParser import IRobotHandConfigParser


close_hand_motor_indices = range(3)
non_spread_motor_indices = range(4)
spread_motor_index = 4
finger_spread_ticks_to_radians = 2 * math.pi / 3072 # 3072 per 180 degrees as stated in HandleSensors.msg is wrong

# found using find_calibration_pose.
# standard deviations: {0: 22.865694828716666, 1: 13.806158046321215, 2: 47.531463263821365}
jig_pose = {0: 6433.3999999999996, 1: 6542.3000000000002, 2: 6297.6000000000004}

# standard deviations: {0: 59.872865306414063, 1: 110.36235771312609, 2: 80.058728443561989}
hand_closed_pose = {0: 8990.7999999999993, 1: 8931.5, 2: 9428.0}

hand_open_desired_pose = {0: 500, 1: 500, 2: 500}

def set_command_message_same_value(message, control_type, indices, value):
    values = dict((motor_index, value) for motor_index in indices)
    set_command_message(message, control_type, values)

def set_command_message(message, control_type, values):
    for motor_index in values.keys():
        message.valid[motor_index] = True
        message.type[motor_index] = control_type
        message.value[motor_index] = values[motor_index]

def loop_control(publisher, rate, max_time, control):
    start_time = rospy.get_time()

    while rospy.get_time() - start_time < max_time and not rospy.is_shutdown():
        command_message = HandleControl()
        control(command_message)
        publisher.publish(command_message)
        rate.sleep()

class IRobotHandController(object):

    def __init__(self, side):
        self.sensors = HandleSensors()
        self.config_parser = IRobotHandConfigParser(side)
        self.config_parser.load()
        
        ros_rate = 100.0  # todo: something smarter
        
        rospy.init_node("mit_irobot_hand_control")
        self.rate = rospy.Rate(ros_rate)
        self.command_publisher = rospy.Publisher("control", HandleControl)
        self.state_publisher = rospy.Publisher("sensors/mit_state", MITIRobotHandState)
        self.calibrate_publisher = rospy.Publisher("events/sensors/calibrate", Empty)
        self.raw_sensors_subscriber = rospy.Subscriber("sensors/raw", HandleSensors, self.sensor_data_callback)
#         self.calibrated_sensors_subscriber = rospy.Subscriber("sensors/calibrated", self.calibrated_sensor_data_callback)
#         rospy.on_shutdown(lambda self : self.exit())

    def sensor_data_callback(self, data):
        self.sensors = data
        self.motor_encoders_with_offset = [self.subtract_offset(self.sensors.motorHallEncoder[i], i) for i in close_hand_motor_indices]
        self.publish_hand_state(self.motor_encoders_with_offset, self.sensors.fingerSpread)

    def publish_hand_state(self, motor_encoders_with_offset, finger_spread_ticks):        
        state_message = MITIRobotHandState()
        state_message.proximalJointAngle = [IRobotHandController.estimate_proximal_joint_angle(motor_encoders_with_offset[i], i) for i in close_hand_motor_indices]
        state_message.distalJointAngle = [IRobotHandController.estimate_distal_joint_angle(motor_encoders_with_offset[i], i) for i in close_hand_motor_indices]
        state_message.fingerSpread = finger_spread_ticks_to_radians * finger_spread_ticks
        self.state_publisher.publish(state_message)

    """
    mapping found using mit_helios_scripts/matlab/irobot_hand_joint_angle_estimation.m
    """
    @staticmethod
    def estimate_proximal_joint_angle(motor_encoder_ticks, index):
        if index in range(2):
            if motor_encoder_ticks < 5.2530e3:
                estimated_angle = 0.5186e-3 * motor_encoder_ticks -0.1026
            else:
                estimated_angle = 0.0128e-3 * motor_encoder_ticks + 2.5542
        elif index == 2:
            if motor_encoder_ticks < 4.8574e3:
                estimated_angle =  0.5301e-3 * motor_encoder_ticks + 0.0002
            else:
                estimated_angle = -0.0056e-3 * motor_encoder_ticks + 2.6021
        else:
            raise RuntimeError('index not recognized')

        if estimated_angle < 0:
            estimated_angle = 0
        return estimated_angle

    """
    mapping found using mit_helios_scripts/matlab/irobot_hand_joint_angle_estimation.m
    """
    @staticmethod
    def estimate_distal_joint_angle(motor_encoder_ticks, index):
        if index in range(2):
            if motor_encoder_ticks < 5.5242e3:
                estimated_angle = -0.0186e-3 * motor_encoder_ticks + 0.3792
            else:
                estimated_angle = 0.5967e-3 * motor_encoder_ticks -3.0199
        elif index == 2:
            if motor_encoder_ticks < 4.9278e3:
                estimated_angle = -0.0505e-3 * motor_encoder_ticks + 0.6206
            else:
                estimated_angle = 0.490e-3 * motor_encoder_ticks -2.0430
        else:
            raise RuntimeError('index not recognized')
                    
        return estimated_angle

    @staticmethod
    def compute_proximal_joint_angle(proximal_joint_angle_ticks):
        ticks_to_radians = 2.0 * math.pi / 1024.0
        ret = proximal_joint_angle_ticks * ticks_to_radians
        return ret

    def zero_current(self):
        no_current_message = HandleControl()
        set_command_message_same_value(no_current_message, HandleControl.CURRENT, non_spread_motor_indices, 0)
        self.command_publisher.publish(no_current_message)

    def close_hand_current_control(self, finger_close_current, indices=close_hand_motor_indices):
        grasp_time = 5
        def control(command_message):
            set_command_message_same_value(command_message, HandleControl.CURRENT, indices, finger_close_current)
        
        loop_control(self.command_publisher, self.rate, grasp_time, control)
    
    def open_hand_angle_control(self):
        open_time = 5
        open_angle = 10
        def control(command_message):
            set_command_message_same_value(command_message, HandleControl.ANGLE, close_hand_motor_indices, open_angle)
        
        loop_control(self.command_publisher, self.rate, open_time, control)
    
    def open_hand_motor_excursion_control(self):
        open_time = 5
        self.motor_excursion_control_loop(hand_open_desired_pose, open_time)

    def motor_excursion_control_loop(self, desireds, duration):
        loop_control(self.command_publisher, self.rate, duration, lambda message : self.motor_excursion_control(message, desireds))

    def motor_excursion_control(self, message, open_hand_desireds):
        desireds_with_offset = dict((i, self.add_offset(open_hand_desireds[i], i)) for i in open_hand_desireds.keys())
        set_command_message(message, HandleControl.POSITION, desireds_with_offset)

    def motor_excursion_control_close_fraction(self, fraction, indices):
        if not (0 <= fraction <= 1):
            raise RuntimeError("Fraction not between 0 and 1: %s" % fraction)
        
        def compute_desired_angle(i):
            return hand_open_desired_pose[i] + fraction * (hand_closed_pose[i] - hand_open_desired_pose[i])
        
        desireds = dict((i, compute_desired_angle(i)) for i in indices)
        self.motor_excursion_control_loop(desireds, 2)

    def add_offset(self, motor_encoder_count, motor_index):
        return self.config_parser.get_motor_encoder_offset(motor_index) + motor_encoder_count

    def subtract_offset(self, motor_encoder_count, motor_index):
        return -self.config_parser.get_motor_encoder_offset(motor_index) + motor_encoder_count

    def spread_angle_control(self, angle_rad):
        ticks = angle_rad / finger_spread_ticks_to_radians
        pose = {spread_motor_index : ticks}
        self.motor_excursion_control_loop(pose, 2)

    def clear_config(self):
        self.config_parser.clear()

    @staticmethod
    def get_close_hand_motor_indices():
        return close_hand_motor_indices
    
    @staticmethod
    def get_non_spread_motor_indices():
        return non_spread_motor_indices

    def get_tendon_excursions_with_offset(self):
        return self.motor_encoders_with_offset

    def calibrate_motor_encoder_offsets_given_pose(self, calibration_pose):
        print "Calibrating: closing hand"
        self.close_hand_current_control(300, close_hand_motor_indices)
        wait_time = 2
        time.sleep(wait_time)
        print "Calibrating: setting offsets"
        for motor_index in close_hand_motor_indices:
            current_value = self.sensors.motorHallEncoder[motor_index]
            offset = current_value - calibration_pose[motor_index]
            self.config_parser.set_motor_encoder_offset(motor_index, offset)
        
        self.zero_current()
        self.config_parser.save()

    def calibrate_motor_encoder_offsets(self, in_jig):
        if in_jig:
            calibration_pose = jig_pose
        else:
            calibration_pose = hand_closed_pose
        
        self.calibrate_motor_encoder_offsets_given_pose(calibration_pose)

    def calibrate_tactile(self):
        print("Calibrating tactile sensors")
        self.calibrate_publisher.publish(Empty())
