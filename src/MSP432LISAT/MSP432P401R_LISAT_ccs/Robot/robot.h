/*
 * Robot.h
 *
 *  Created on: Oct 15, 2020
 *      Author: david
 */

#include "common.h"
#include "differentialDrive.h"
#include "motorDriver.h"

#ifndef ROBOT_ROBOT_H_
#define ROBOT_ROBOT_H_

// Odometer ratio setup
#define DISC_LINE_COUNT 32
#define MOTOR_GEAR_RATIO 120
#define LINES_PER_REV (DISC_LINE_COUNT*MOTOR_GEAR_RATIO)

// Wheel geometry
#define DIAM_WHEEL 70 // mm
#define CIRCUMFERENCE_WHEEL (PI*DIAM_WHEEL)

// Tachometer interrupt-ticks per second
const float dist_per_rising_edge = CIRCUMFERENCE_WHEEL/LINES_PER_REV;
#define ACLK_COUNTS 32768

#define MIN_RPM_ERROR 30
/*
 * Combine differentialRobot struct, methods and MotorDriver functions to position the robot or keep the robot speed in close-loop control.
 */
class Robot
{
public:
    Robot();
    void StartUp();
    void Stop();
    void RunTachoCalibrations(int32_t* requestedRPMs, uint32_t* outCalibratedDutyCycles, int calibrationCount);
    MotorDriver* motorDriver;
protected:
    differential_drive_t* diffDrive;
    void UpdateWheelDistances();
    void UpdateRobotDistance();
private:
    uint32_t ApproximateRPM(int32_t rpm, int maxRounds, int maxRPMError);

    float wheel_distance_left;
    float wheel_distance_right;
    float distance_difference;
    float robot_angle;
};

#endif /* ROBOT_ROBOT_H_ */
