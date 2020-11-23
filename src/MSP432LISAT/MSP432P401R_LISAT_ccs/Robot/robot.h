/*
 * Robot.h
 *
 *  Created on: Oct 15, 2020
 *      Author: david
 */

#include <System/periodicKernelTask.h>

#include "common.h"
#include "motorDriver.h"

#ifndef ROBOT_ROBOT_H_
#define ROBOT_ROBOT_H_

// Odometer ratio setup
#define DISC_LINE_COUNT 6
#define MOTOR_GEAR_RATIO 120
#define LINES_PER_REV (DISC_LINE_COUNT*MOTOR_GEAR_RATIO)

// Wheel geometry
#define WHEEL_BASE 140 //mm
#define DIAM_WHEEL 70 // mm
#define CIRCUMFERENCE_WHEEL (PI*DIAM_WHEEL)

// Tachometer interrupt-ticks per second
const float dist_per_rising_edge = CIRCUMFERENCE_WHEEL/LINES_PER_REV;
#define ACLK_COUNTS 32768

#define MIN_RPM_ERROR 30

static void RunControlLoop(UArg robot_this_pointer);

/*
 * Combine differentialRobot struct, methods and MotorDriver functions to position the robot or keep the robot speed in close-loop control.
 */
class Robot
{
public:
    Robot();
    void StartUp();
    void Stop();

    // Set or disable control
    void EnableDriveControl();
    void DisableDriveControl();
    bool IsControlEnabled();

    // Controlled or manual drive
    void ControlLoop(uint16_t time);
    void UpdateRobotPosition();
    void adjustRobotAngleThetaOffset(float angleOffset);
    // Calibration
    void TurnLeft(float angle);
    void TurnRight(float angle);
    void RunTachoCalibrations(int32_t* requestedRPMs, uint32_t* outCalibratedDutyCycles, int calibrationCount);

    // Public access for ease of use
    MotorDriver* motorDriver;
private:
    void _updateRobotCenterPosition(float deltaDistanceCenter);
    void _updateRobotAngleTheta(float deltaDistanceLeft, float deltaDistanceRight);
    bool _isDriving();
    uint32_t _reachMMPS(int32_t rpm, int maxRounds, int maxRPMError);

    PeriodicKernelTask* periodicControlTask = new PeriodicKernelTask();

    /* Sensor state */
    bool enabledAngleControl;
    float robotPositionX;
    float robotPositionY;
    float robotAngleTheta;
    float robotAngleThetaOffset = 0;
    float distanceTravelled;

    /* Controller state and parameters*/
    double E_i=0;
    float K_i = 2.6;
    float K_p = 330;
};

#endif /* ROBOT_ROBOT_H_ */
