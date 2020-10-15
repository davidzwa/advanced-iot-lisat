#include "common.h"
#include <Robot/Motor.h>

Motor::Motor()
{
    PWM_init();
}

void Motor::Initialize() {
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
void Motor::PowerUp(void) {
    this->CheckPWMStarted();
    this->AwakeDriver();
    this->motorPowered = true;
}
void Motor::AwakeDriver(void) {
    GPIO_write(MOTOR_LEFT_SLEEP, 1);
    GPIO_write(MOTOR_RIGHT_SLEEP, 1);
}

/**
 * Cut output power on the DC motors.
 * Put the motor in sleep mode to save energy.
 * Set the duty cycles to zero.
*/
void Motor::PowerDown(void) {
    this->CheckPWMStopped();
    this->SleepDriver();
    this->motorPowered = false;
}
void Motor::SleepDriver(void) {
    GPIO_write(MOTOR_LEFT_SLEEP, 0);
    GPIO_write(MOTOR_RIGHT_SLEEP, 0);
}

void Motor::CheckPWMStarted(){
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

void Motor::CheckPWMStopped(){
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

///**
// * Set speed from 0 to PWM_PERIOD where PWM_PERIOD is full power.
// */
//void Motor::IncrementSpeed(int leftMotor, int rightMotor){
//    dutyLeft+=leftMotor;
//    dutyRight+=rightMotor;
//    if (dutyLeft > pwmPeriod || dutyRight > pwmPeriod) {
//        dutyLeft = 0;
//        dutyRight = 0;
//
//        GPIO_toggle(MOTOR_LEFT_DIRECTION);
//        GPIO_toggle(MOTOR_RIGHT_DIRECTION);
//    }
//    this->SetSpeed(dutyLeft, dutyRight);
//}

// Drive + rotate = curve. Curve = 0-255
void Motor::DriveLeft(uint16_t speed, uint16_t curve){
    this->SetMotorSpeedLimited(this->pwmLeft, speed, &(this->dutyLeft));
    this->SetMotorSpeedLimited(this->pwmRight, speed, &(this->dutyRight));

    this->SetMotorDirection(MOTOR_LEFT_DIRECTION, BACKWARDS);
    this->SetMotorDirection(MOTOR_RIGHT_DIRECTION, FORWARDS);
}

// Drive + rotate = curve. Curve = 0-255
void Motor::DriveRight(uint16_t speed, uint16_t curve){
    this->SetMotorSpeedLimited(this->pwmLeft, speed, &(this->dutyLeft));
    this->SetMotorSpeedLimited(this->pwmRight, speed, &(this->dutyRight));

    this->SetMotorDirection(MOTOR_LEFT_DIRECTION, FORWARDS);
    this->SetMotorDirection(MOTOR_RIGHT_DIRECTION, BACKWARDS);
}

// Drive backward mode (SPEED_MAX: 1/2)
void Motor::DriveBackwards(uint16_t speed){
    this->SetMotorDirection(MOTOR_LEFT_DIRECTION, BACKWARDS);
    this->SetMotorDirection(MOTOR_RIGHT_DIRECTION, BACKWARDS);
    this->SetMotorSpeedLimited(this->pwmLeft, speed, &(this->dutyLeft));
    this->SetMotorSpeedLimited(this->pwmRight, speed, &(this->dutyRight));
}

// Drive forward mode
void Motor::DriveForwards(uint16_t speed){
    this->SetMotorDirection(MOTOR_LEFT_DIRECTION, FORWARDS);
    this->SetMotorDirection(MOTOR_RIGHT_DIRECTION, FORWARDS);
    this->SetMotorSpeedLimited(this->pwmLeft, speed, &(this->dutyLeft));
    this->SetMotorSpeedLimited(this->pwmRight, speed, &(this->dutyRight));
}

// Set motor direction: 1 forward, 0 backward
void Motor::SetMotorDirection(uint8_t motorDirPin, MOTOR_DIRECTION direction){
    GPIO_write(motorDirPin, direction);
}

void Motor::SetMotorSpeedLimited(PWM_Handle pwmHandle, uint16_t speed, uint16_t* dutyCycleStore) {
    this->CheckPWMStarted();
    if (speed < SPEED_MAX) {
        PWM_setDuty(pwmHandle, speed);
        *dutyCycleStore = speed;
    }
    else {
        this->PowerDown(); // Erroneous situation
    }
}

