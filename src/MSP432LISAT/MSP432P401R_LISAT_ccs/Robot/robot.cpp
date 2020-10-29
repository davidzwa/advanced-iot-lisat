/*
 * Robot.cpp
 *
 *  Created on: Oct 15, 2020
 *      Author: david
 */

#include <math.h>
#include <Robot/robot.h>
#include <Robot/tachometer.h>

Robot::Robot()
{
    this->diffDrive = init_diff_drive();
    this->motorDriver = new MotorDriver();
    initTachometers();
}

void Robot::StartUp() {
    this->motorDriver->Initialize();
    this->motorDriver->PowerUp();
    enableTachometerInterrupts();
}

void Robot::RunTachoCalibrations(int32_t* requestedRPMs, uint32_t* outCalibratedDutyCycles, int calibrationCount) {
    this->Stop();

    for(int i=0; i<calibrationCount; i++) {
        int32_t targetRPM = requestedRPMs[i];
        outCalibratedDutyCycles[i] = this->ApproximateRPM(targetRPM, 10000, 50);
        this->motorDriver->Drive(500,500);
        usleep(250000); // 250ms slow-down
    }

    this->Stop();
}

int32_t SpeedToTicks(int32_t speed_mmps) {
    return round(ACLK_COUNTS/(speed_mmps/dist_per_rising_edge));
}

// Spin up the wheels until the Tachometers reach the target RPM
uint32_t Robot::ApproximateRPM(int32_t speed_mmps, int maxRounds, int maxTicksError) {
    int16_t targetTicks = SpeedToTicks(speed_mmps);

    startCalibrationTachometers();
    int16_t initialDutyCycleLeft = 500;
    int16_t initialDutyCycleRight = 500;
    this->motorDriver->Drive(initialDutyCycleLeft, initialDutyCycleRight);

    int iter = 0;
    const int diff = 10;
    const int iter_delay = 25;
    while(iter < maxRounds) {
        usleep(iter_delay*1000); // Spin up the drive in 50ms
        float ticksAvgLeft = calculateTicksPerInterruptLeft();
        float ticksAvgRight = calculateTicksPerInterruptRight();
        startCalibrationTachometers();

        if (abs(ticksAvgLeft - targetTicks) < maxTicksError && abs(ticksAvgRight - targetTicks) < maxTicksError) {
            break;
        }

        if(isinf(ticksAvgLeft) || ticksAvgLeft == 0.0f || ticksAvgRight > targetTicks) {
            initialDutyCycleLeft += diff;
        }
        else {
            initialDutyCycleLeft -= diff;
        }
        if(isinf(ticksAvgRight) || ticksAvgRight == 0.0f || ticksAvgRight > targetTicks) {
            initialDutyCycleRight += diff;
        } else {
            initialDutyCycleRight -= diff;
        }

        this->motorDriver->Drive(initialDutyCycleLeft, initialDutyCycleRight);
        iter++;
    }

    disableCalibrationTachometers();
    return initialDutyCycleLeft;
}

void Robot::Stop() {
    this->motorDriver->DriveForwards(0);
}

//void Robot::RunTest() {
    //        motors->DriveBackwards(4000); sleep(1);
    //        motors->DriveForwards(0); sleep(1);

    //        motors->DriveLeft(4000, 2000); sleep(1);
    //        motors->DriveRight(4000, 1000); sleep(1);

    //        motors->DriveLeft(4000, -500); sleep(1);
    //        motors->DriveRight(4000, -500); sleep(1);
    //
    //        motors->DriveRight(4000, -4000); sleep(1);
    //        motors->DriveRight(4000, 4000); sleep(1);
//}
