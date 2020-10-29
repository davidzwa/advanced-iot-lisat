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

    this->wheel_distance_left = 0.0f;
    this->wheel_distance_right = 0.0f;
    this->distance_difference = 0.0f;
    this->robot_angle = 0.0f;
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

        // 250ms slow-down to lower speed, preventing two calibrations to be triggered after each other
        this->motorDriver->Drive(500,500);
        usleep(250000);
    }

    this->Stop();
}

int32_t SpeedToTicksPerInterrupt(int32_t speed_mmps) {
    return round(ACLK_COUNTS/(speed_mmps/dist_per_rising_edge));
}

float TicksToDistance(uint32_t num_isr_ticks) {
    return num_isr_ticks * dist_per_rising_edge;
}

// Spin up the wheels until the Tachometers reach the target RPM
uint32_t Robot::ApproximateRPM(int32_t speed_mmps, int maxRounds, int maxTicksError) {
    int16_t targetTicks = SpeedToTicksPerInterrupt(speed_mmps);

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

void Robot::DriveStraight() {
    // Implement drive straight control loop
}

void Robot::UpdateRobotDistance() {
    uint32_t numTicksLeft = getNuminterruptsLeft();
    TicksToDistance(numTicksLeft);
    TicksToDistance(numTicksRight);
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

void Robot::UpdateWheelDistances() {

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
