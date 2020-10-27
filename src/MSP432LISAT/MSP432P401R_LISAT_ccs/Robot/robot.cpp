/*
 * Robot.cpp
 *
 *  Created on: Oct 15, 2020
 *      Author: david
 */

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
    }

    this->Stop();
}

// Spin up the wheels until the Tachometers reach the target RPM
uint32_t Robot::ApproximateRPM(int32_t rpm, int maxRounds, int maxRPMError) {
    startCalibrationTachometers();
    int16_t initialDutyCycleLeft = 500;
    int16_t initialDutyCycleRight = 500;
    this->motorDriver->Drive(initialDutyCycleLeft, initialDutyCycleRight);
    sleep(1); // Spin up the drive

    int error = maxRPMError+1;
    int iter = 0;
    while(error > maxRPMError && iter < maxRounds) {
        float rpmLeft = 10*32000/calculateTicksPerInterruptLeft();
        float rpmRight = 10*32000/calculateTicksPerInterruptRight();
        startCalibrationTachometers();
        usleep(500000);
        if(isinf(rpmLeft)) {
            initialDutyCycleLeft += 100;
        }
        else {
            initialDutyCycleLeft = 450*rpm/rpmLeft;
        }
        if(isinf(rpmRight)) {
            initialDutyCycleRight += 100;
        } else {
            initialDutyCycleRight = 450*rpm/rpmRight;
        }

        this->motorDriver->Drive(initialDutyCycleLeft, initialDutyCycleRight);
        iter++;
    }

    disableCalibrationTachometers();
    return 0;
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
