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
private:
    uint32_t ApproximateRPM(int32_t rpm, int maxRounds, int maxRPMError);
};

#endif /* ROBOT_ROBOT_H_ */
