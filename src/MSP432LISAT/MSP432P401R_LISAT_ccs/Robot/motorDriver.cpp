#include <Robot/motorDriver.h>
#include "common.h"

MotorDriver::MotorDriver()
{
    // We init the PWM before the BIOS starts
}

void MotorDriver::Initialize() {
    PWM_Params_init(&params);
    params.dutyUnits = PWM_DUTY_US;
    params.dutyValue = 0;
    params.periodUnits = PWM_PERIOD_US;
    params.periodValue = pwmPeriod;

    pwmLeft = PWM_open(MOTOR_LEFT_PWM, &params);
    if (pwmLeft == NULL) {
        /* CONFIG_PWM_0 did not open */
        while (1);
    }

    pwmRight = PWM_open(MOTOR_RIGHT_PWM, &params);
    if (pwmRight == NULL) {
        /* CONFIG_PWM_0 did not open */
        while (1);
    }
    this->PowerDown();
}

/**
 * Start the pwm signals and awake the H-bridge driver
 */
void MotorDriver::PowerUp(void) {
    this->CheckPWMStarted();
    this->AwakeDriver();
    this->motorPowered = true;
}
void MotorDriver::AwakeDriver(void) {
    GPIO_write(MOTOR_LEFT_SLEEP, 1);
    GPIO_write(MOTOR_RIGHT_SLEEP, 1);
}

/**
 * Cut output power on the DC motors.
 * Put the motor in sleep mode to save energy.
 * Set the duty cycles to zero.
*/
void MotorDriver::PowerDown(void) {
    this->CheckPWMStopped();
    this->SleepDriver();
    this->motorPowered = false;
}
void MotorDriver::SleepDriver(void) {
    GPIO_write(MOTOR_LEFT_SLEEP, 0);
    GPIO_write(MOTOR_RIGHT_SLEEP, 0);
}

void MotorDriver::CheckPWMStarted(){
    if (!pwm1Started) {
        dutyLeft = 0;
        PWM_start(pwmLeft);
        pwm1Started = true;
    }
    if (!pwm2Started) {
        dutyRight = 0;
        PWM_start(pwmRight);
        pwm2Started = true;
    }
    PWM_setDuty(pwmLeft, dutyLeft);
    PWM_setDuty(pwmRight, dutyRight);
}

void MotorDriver::CheckPWMStopped(){
    if (pwm1Started) {
        dutyLeft = 0;
        PWM_stop(pwmLeft);
        pwm1Started = false;
    }
    if (pwm2Started) {
        dutyRight = 0;
        PWM_stop(pwmRight);
        pwm2Started = false;
    }
    PWM_setDuty(pwmLeft, dutyLeft);
    PWM_setDuty(pwmRight, dutyRight);
}

// Drive + rotate = curve. Rotate [-SPEED/2, SPEED/2]
void MotorDriver::DriveLeft(uint16_t speed, int16_t speed2){
    this->SetMotorSpeedLimited(this->pwmRight, speed, &(this->dutyRight));

    if (speed2 < 0) {
        this->SetMotorSpeedLimited(this->pwmLeft, -speed2, &(this->dutyLeft));
        this->SetMotorDirection(MOTOR_LEFT_DIRECTION, BACKWARDS);
    }
    else {
        this->SetMotorSpeedLimited(this->pwmLeft, speed2, &(this->dutyLeft));
        this->SetMotorDirection(MOTOR_LEFT_DIRECTION, FORWARDS);
    }
    this->SetMotorDirection(MOTOR_RIGHT_DIRECTION, FORWARDS);
}

// Drive + rotate = curve. Rotate [-SPEED/2, SPEED/2]
void MotorDriver::DriveRight(uint16_t speed, int16_t speed2){
    this->SetMotorSpeedLimited(this->pwmLeft, speed, &(this->dutyLeft));

    // Dia: 15cm
    // Circ: 47.12cm
    // Speed full:10cm/s
    // Rot 50%: 90 degr/s
    this->SetMotorDirection(MOTOR_LEFT_DIRECTION, FORWARDS);
    if (speed2 < 0) {
        this->SetMotorSpeedLimited(this->pwmRight, -speed2, &(this->dutyRight));
        this->SetMotorDirection(MOTOR_RIGHT_DIRECTION, BACKWARDS);
    }
    else {
        this->SetMotorSpeedLimited(this->pwmRight, speed2, &(this->dutyRight));
        this->SetMotorDirection(MOTOR_RIGHT_DIRECTION, FORWARDS);
    }
}

#define TURN_SPEED 200

void MotorDriver::SetupRotateLeft() {
    this->isRotating = true;
    this->SetMotorDirection(MOTOR_LEFT_DIRECTION, FORWARDS);
    this->SetMotorDirection(MOTOR_RIGHT_DIRECTION, BACKWARDS);
    this->SetMotorSpeedLimited(this->pwmLeft, TURN_SPEED, &(this->dutyLeft));
    this->SetMotorSpeedLimited(this->pwmRight, TURN_SPEED, &(this->dutyLeft));
}

void MotorDriver::SetupRotateRight() {
    this->isRotating = true;
    this->SetMotorDirection(MOTOR_LEFT_DIRECTION, BACKWARDS);
    this->SetMotorDirection(MOTOR_RIGHT_DIRECTION, FORWARDS);
    this->SetMotorSpeedLimited(this->pwmLeft, TURN_SPEED, &(this->dutyLeft));
    this->SetMotorSpeedLimited(this->pwmRight, TURN_SPEED, &(this->dutyLeft));
}

// Drive backward mode (SPEED_MAX: 1/2)
void MotorDriver::DriveBackwards(uint16_t speed){
    this->isRotating = false;
    this->speedSetpoint = speed;
    this->SetMotorDirection(MOTOR_LEFT_DIRECTION, BACKWARDS);
    this->SetMotorDirection(MOTOR_RIGHT_DIRECTION, BACKWARDS);
    this->SetMotorSpeedLimited(this->pwmLeft, speed, &(this->dutyLeft));
    this->SetMotorSpeedLimited(this->pwmRight, speed, &(this->dutyRight));
}

// Drive forward mode
void MotorDriver::DriveForwards(uint16_t speed) {
    this->isRotating = false;
    this->speedSetpoint = speed;
    this->SetMotorDirection(MOTOR_LEFT_DIRECTION, FORWARDS);
    this->SetMotorDirection(MOTOR_RIGHT_DIRECTION, FORWARDS);
    this->SetMotorSpeedLimited(this->pwmLeft, speed, &(this->dutyLeft));
    this->SetMotorSpeedLimited(this->pwmRight, speed, &(this->dutyRight));
}

void MotorDriver::SetControlAdjustment(uint16_t offset_left, uint16_t offset_right) {
    this->dutyLeftOffset = offset_left;
    this->dutyRightOffset = offset_right;
}

void MotorDriver::ControlUpdateDutyCycle() {
    this->SetMotorSpeedLimited(this->pwmLeft, this->speedSetpoint + this->dutyLeftOffset, &(this->dutyLeft));
    this->SetMotorSpeedLimited(this->pwmRight, this->speedSetpoint + this->dutyRightOffset, &(this->dutyRight));
}

void MotorDriver::ResetControlAdjustment() {
    this->dutyLeftOffset = 0;
    this->dutyRightOffset = 0;
}

void MotorDriver::Drive(int16_t speed_left, int16_t speed_right) {
    if (speed_left > 0) {
        this->SetMotorDirection(MOTOR_LEFT_DIRECTION, FORWARDS);
    }
    else {
        this->SetMotorDirection(MOTOR_LEFT_DIRECTION, BACKWARDS);
    }
    if (speed_right > 0) {
        this->SetMotorDirection(MOTOR_RIGHT_DIRECTION, FORWARDS);
    } else {
        this->SetMotorDirection(MOTOR_RIGHT_DIRECTION, BACKWARDS);
    }
    this->SetMotorSpeedLimited(this->pwmLeft, speed_left, &(this->dutyLeft)); // -this->dutyLeftOffset
    this->SetMotorSpeedLimited(this->pwmRight, speed_right, &(this->dutyRight)); //-this->dutyRightOffset
}

// Set motor direction: 1 forward, 0 backward
void MotorDriver::SetMotorDirection(uint8_t motorDirPin, MOTOR_DIRECTION direction){
    GPIO_write(motorDirPin, direction);
}

void MotorDriver::SetMotorSpeedLimited(PWM_Handle pwmHandle, uint16_t speed, uint16_t* dutyCycleStore) {
    this->CheckPWMStarted();
    if (speed < SPEED_MAX) {
        PWM_setDuty(pwmHandle, speed);
        *dutyCycleStore = speed;
    }
    else {
        this->PowerDown(); // Erroneous situation
        GPIO_write(LED_ERROR_2, 1);
    }
}

uint16_t MotorDriver::getSpeedSetpoint() {
    return this->speedSetpoint;
}

