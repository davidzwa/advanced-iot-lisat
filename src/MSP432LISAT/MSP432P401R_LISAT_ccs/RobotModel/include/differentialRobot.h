/*
 * Date  : 25 Jul 2020
 * Author: Amjad Yousef Majid
 * Email : amjad.y.majid@gmail.com
 */

#include <stdint.h>

#ifndef DATASTRUCTURE_INCLUDE_ROBOT_H_
#define DATASTRUCTURE_INCLUDE_ROBOT_H_

#define RADIUS .035                    // wheel radius in meter
#define TICKS_PER_REV 360              // ticks per wheel revolution
#define L .14                          // distance between the two wheels in meter
// #define dis_per_tick  .61 //(2 * _PI * R)/N [distance in meter]
#define TICK_DIS_NUMERATOR 61          // to avoid float operation split the fraction 0.61
#define TICK_DIS_DENOMINATOR 100000    // consequently you much multiply and then divide

typedef struct ir_distances{
    uint32_t ir_left;
    uint32_t ir_center;
    uint32_t ir_right;

}ir_distance_t;

typedef struct tachometer{
    volatile float delta_dis;     // distance crossed between to updates in meter
    volatile int32_t ticks;
    volatile int32_t prev_ticks;
} tachometer_t;

typedef struct pose
{
    volatile float x;
    volatile float y;
    volatile double theta;
} pose_t;

typedef struct wheel
{
    float radius;
    uint32_t ticks_per_rev;
    tachometer_t * tachometer;
} wheel_t;

typedef struct differential_robot
{
    float base_len;  // the distance between the two wheels in meters
    ir_distance_t * ir_distance;
    pose_t * pose;
    wheel_t * right;
    wheel_t * left;
} differential_robot_t;

differential_robot_t* robot_init();
void robot_position_update(differential_robot_t * robot);


#endif /* DATASTRUCTURE_INCLUDE_ROBOT_H_ */
