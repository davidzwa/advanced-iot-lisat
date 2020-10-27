/*
 * Date: 25 Jul 2020
 * Author: Amjad Yousef Majid
 * Email: amjad.y.majid@gmail.com
 */

#include <math.h>
#include <Robot/differentialDrive.h>

// initialize the robot data structure
tachometer_t right_tachometer = {0,0};
tachometer_t left_tachometer = {0,0};
pose_t pose = {0,0,0};  // initial position of the robot

wheel_t right_wheel;
wheel_t left_wheel;
differential_drive_t robot;

differential_drive_t* init_diff_drive()
{
	// Initialize right wheel
    right_wheel.radius = RADIUS;
    right_wheel.ticks_per_rev = 0;
    right_wheel.tachometer = &right_tachometer;
    // Initialize left wheel
    left_wheel.radius = RADIUS;
    left_wheel.ticks_per_rev = 0;
    left_wheel.tachometer = &left_tachometer;

    robot.base_len = L;
    robot.right = &right_wheel;
    robot.left = &left_wheel;
    robot.pose = &pose;
   
    return &robot;
}

float _robot_distance_update_mm(float d_r, float d_l){
	return (d_r + d_l)/2;
}

void _wheel_distance_update_mm(tachometer_t * tachometer){
	// Cast the delta_ticks to float
	float delta_ticks = (float)tachometer->delta_ticks;

	// Update the tachometer distance by Fixed-Point shift
	tachometer->delta_dis = (TICK_DIS_NUMERATOR * delta_ticks) / TICK_DIS_DENOMINATOR;  // distance in meter
}

void robot_position_update(differential_drive_t * robot){
    _wheel_distance_update_mm(robot->right->tachometer);
    _wheel_distance_update_mm(robot->left->tachometer);

    float d_r = robot->right->tachometer->delta_dis;
    float d_l = robot->left->tachometer->delta_dis;
    float d_c = _robot_distance_update_mm(d_r, d_l);

    float x = robot->pose->x;
    float y = robot->pose->y;
    float theta = robot->pose->theta;

    x += d_c * cos(theta);
    y += d_c * sin(theta);
    float delta_theta = (float)(d_r - d_l)/robot->base_len;
    theta += delta_theta;

    robot->pose->x = x;
    robot->pose->y = y;
    robot->pose->theta = atan2(sin(theta), cos(theta));
}
