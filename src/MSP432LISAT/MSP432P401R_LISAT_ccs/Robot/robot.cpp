/*
 * Robot.cpp
 *
 *  Created on: Oct 15, 2020
 *      Author: david
 */

#include <math.h>
#include <Robot/robot.h>
#include <Robot/tachometer.h>

/*
 * Static calculation utilities
 */
int32_t SpeedToTicksPerInterrupt(int32_t speed_mmps) {
    return round(ACLK_COUNTS/(speed_mmps/dist_per_rising_edge));
}

float TicksToDistance(uint32_t num_isr_ticks) {
    return num_isr_ticks * dist_per_rising_edge;
}

float WheelDistanceToRobotDisplacement(float deltaDistanceWheelRight, float deltaDistanceWheelLeft){
    return (deltaDistanceWheelRight + deltaDistanceWheelLeft)/2;
}

static void RunControlLoop(UArg this_pointer) {
    Robot* robot = (Robot*)this_pointer;
    robot->ControlLoop(Clock_getTicks());
}

/*
 * Class methods
 */
Robot::Robot()
{
    this->motorDriver = new MotorDriver();
    initTachometers();

    this->robotPositionX = 0.0f;
    this->robotPositionY = 0.0f;
    this->robotAngleTheta = 0.0f;
    this->distanceTravelled = 0.0f;
}

void Robot::StartUp() {
    this->motorDriver->Initialize();
    this->motorDriver->PowerUp();
    enableTachometerInterrupts();
}

void Robot::EnableDriveControl() {
    this->enabledAngleControl = true;
    this->periodicControlTask->setupClockMethod(CONTROL_LOOP_INITIAL_OFFSET, CONTROL_LOOP_PERIOD, RunControlLoop, this);
    this->periodicControlTask->startClockTask();
}

bool Robot::IsControlEnabled() {
    return this->enabledAngleControl;
}

void Robot::DisableDriveControl() {
    this->periodicControlTask->stopClockTask();
    this->Stop(); // Avoid spinning endlessly
    this->enabledAngleControl = false;
}

void Robot::ControlLoop(uint16_t time) {
    if (this->motorDriver->getSpeedSetpoint() <= 0 ) { // hack to ignore increasing tachometers while robot is stationary
        return;
    }
    this->UpdateRobotPosition();
//    if (isRotating) {
//
//    }
    // As UpdateRobotPosition is based on tacho interrupts, we can expect the position to be updated
    float heading_error = robotAngleTheta;
    float err = atan2(sin(heading_error), cos(heading_error));
    this->E_i+=err;
    float U_i = this->K_i * E_i;
    float U_p = this->K_p * err;
    float w = U_p + U_i;
    this->motorDriver->SetControlAdjustment(w, -w);
    this->motorDriver->ControlUpdateDutyCycle();
    GPIO_toggle(LED_GREEN_2_GPIO);
}

void Robot::RunTachoCalibrations(int32_t* requestedRPMs, uint32_t* outCalibratedDutyCycles, int calibrationCount) {
    this->Stop();

    for(int i=0; i<calibrationCount; i++) {
        int32_t targetRPM = requestedRPMs[i];
        outCalibratedDutyCycles[i] = this->_reachMMPS(targetRPM, 10000, 50);

        // 250ms slow-down to lower speed, preventing two calibrations to be triggered after each other
        this->motorDriver->Drive(500,500);
        usleep(250000);
    }
    this->Stop();
}


// Spin up the wheels until the Tachometers reach the target RPM
uint32_t Robot::_reachMMPS(int32_t speed_mmps, int maxRounds, int maxTicksError) {
    int16_t targetTicks = SpeedToTicksPerInterrupt(speed_mmps);

    resetTachometerCountValues();
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
        resetTachometerCountValues();

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

    resetTachometerCountValues();
    return initialDutyCycleLeft;
}

void Robot::TurnLeft(float angle) {
    this->robotAngleThetaOffset += angle;
    this->motorDriver->SetupRotateLeft();
}

void Robot::TurnRight(float angle) {
    this->robotAngleThetaOffset -= angle;
    this->motorDriver->SetupRotateRight();
}

void Robot::Stop() {
    this->motorDriver->DriveForwards(0);
}


void Robot::UpdateRobotPosition() {
    // Fetch ISR ticks and reset the corresponding counters
    uint32_t numTicksLeft = getInterruptCountLeft();
    uint32_t numTicksRight = getInterruptCountRight();
    resetTachometerCountValues();

    // Process wheel distances to get center point displacement
    float displacementLeft = TicksToDistance(numTicksLeft);
    float displacementRight = TicksToDistance(numTicksRight);
    float displacementCenter = WheelDistanceToRobotDisplacement(displacementLeft, displacementRight);
    
    this->_updateRobotCenterPosition(displacementCenter);
    this->_updateRobotAngleTheta(displacementLeft, displacementRight);
}

void Robot::_updateRobotCenterPosition(float deltaDistanceCenter) {
    this->robotPositionX += deltaDistanceCenter * cos(this->robotAngleTheta);
    this->robotPositionY += deltaDistanceCenter * sin(this->robotAngleTheta);
    this->distanceTravelled += deltaDistanceCenter;
}

void Robot::_updateRobotAngleTheta(float deltaDistanceLeft, float deltaDistanceRight) {
    float tempTheta = this->robotAngleTheta;
    float deltaTheta = (float)(deltaDistanceRight - deltaDistanceLeft)/WHEEL_BASE;
    tempTheta += deltaTheta;
    this->robotAngleTheta = atan2(sin(tempTheta), cos(tempTheta));
}

void Robot::adjustRobotAngleThetaOffset(float angleOffset) {
    this->robotAngleThetaOffset += angleOffset;
}

bool Robot::_isDriving() {
//    return (this->motorDriver->)
    return true;
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
