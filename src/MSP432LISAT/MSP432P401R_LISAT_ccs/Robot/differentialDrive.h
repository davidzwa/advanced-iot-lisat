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

typedef struct tachometer {
    volatile uint32_t delta_ticks;      // difference in ticks since last update
    volatile float delta_dis;           // distance crossed between to updates in meter
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

typedef struct differential_drive
{
    float base_len;  // the distance between the two wheels in meters
    pose_t * pose;
    wheel_t * right;
    wheel_t * left;
} differential_drive_t;

differential_drive_t* init_diff_drive();
void robot_position_update(differential_drive_t * robot);


#endif /* DATASTRUCTURE_INCLUDE_ROBOT_H_ */
