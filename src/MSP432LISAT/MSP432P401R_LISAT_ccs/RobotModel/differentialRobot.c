/*
 * Date: 25 Jul 2020
 * Author: Amjad Yousef Majid
 * Email: amjad.y.majid@gmail.com
 */

#include "differentialRobot.h"
//#include "printf.h"
#include <math.h>
#define MAX_IR 800 // max ir distances in mm

//uint16_t const  data_len_robot_pose_debug = 200;
//uint16_t static cntr_robot_pose_debug=0;
//uint32_t static robot_x[data_len_robot_pose_debug]={0};
//uint32_t static robot_y[data_len_robot_pose_debug]={0};
//uint32_t static robot_theta[data_len_robot_pose_debug]={0};

// initialize the robot data structure
tachometer_t right_tachometer = {0,0,0};
tachometer_t left_tachometer = {0,0,0};
pose_t pose = {0,0,0};  // initial position of the robot
ir_distance_t ir_distance = {MAX_IR,MAX_IR,MAX_IR};

wheel_t right_wheel;
wheel_t left_wheel;
differential_robot_t robot;

differential_robot_t* robot_init()
{
	// initialize right wheel
    right_wheel.radius = RADIUS;
    right_wheel.ticks_per_rev = 0;
    right_wheel.tachometer = &right_tachometer;
    // initialize left wheel
    left_wheel.radius = RADIUS;
    left_wheel.ticks_per_rev = 0;
    left_wheel.tachometer = &left_tachometer;

    robot.base_len = L;
    robot.right = &right_wheel;
    robot.left = &left_wheel;
    robot.pose = &pose;
    robot.ir_distance = &ir_distance;
   
    return &robot;
}


float _robot_distance_update_mm(float d_r, float d_l){
	return (d_r + d_l)/2;
}

void _wheel_distance_update_mm( tachometer_t * tachometer){
	// calculate the distance traveled since the last update
    int32_t ticks = tachometer->ticks;
	float delta_ticks = ticks - tachometer->prev_ticks;
	// update the previous tachometer ticks
	tachometer->prev_ticks = ticks;
	tachometer->delta_dis = (TICK_DIS_NUMERATOR * delta_ticks) / TICK_DIS_DENOMINATOR;  //distance in meter
}

void robot_position_update(differential_robot_t * robot){
    _wheel_distance_update_mm( robot->right->tachometer );
    _wheel_distance_update_mm( robot->left->tachometer );

    float d_r = robot->right->tachometer->delta_dis;
    float d_l = robot->left->tachometer->delta_dis;
	float d_c = _robot_distance_update_mm(d_r, d_l);

	float x = robot->pose->x;
	float y = robot->pose->y;
	float theta = robot->pose->theta;

	x += d_c * cosf(theta);
	y += d_c * sinf(theta);
	float delta_theta = (float)(d_r - d_l)/robot->base_len;
	theta += delta_theta;

	robot->pose->x = x;
	robot->pose->y = y;
	robot->pose->theta = atan2f( sinf(theta), cosf(theta));

//	if(cntr_robot_pose_debug >=data_len_robot_pose_debug)
//	{
//		cntr_robot_pose_debug=0;
//	}
//	robot_x[cntr_robot_pose_debug] = x;
//	robot_y[cntr_robot_pose_debug] = y;
//	robot_theta[cntr_robot_pose_debug]= robot->pose->theta;
//	cntr_robot_pose_debug++;

}













