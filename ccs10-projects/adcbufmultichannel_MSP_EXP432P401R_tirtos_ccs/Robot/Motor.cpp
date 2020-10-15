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

    pwm1 = PWM_open(MOTOR_LEFT_PWM, &params);
    if (pwm1 == NULL) {
        /* CONFIG_PWM_0 did not open */
        while (1);
    }

    pwm2 = PWM_open(MOTOR_RIGHT_PWM, &params);
    if (pwm2 == NULL) {
        /* CONFIG_PWM_0 did not open */
        while (1);
    }
}

/**
 * Cut output power on the DC motors. Kill the direction signal and
 * Put the motor in sleep mode to save energy. Set the duty cycles to zero.
*/
void Motor::Stop(void) {
    PWM_stop(pwm1);
    PWM_stop(pwm2);
}

/**
 * Set speed from 0 to PWM_PERIOD where PWM_PERIOD is full power.
 */
void Motor::SetSpeed(int leftMotor, int rightMotor){
    dutyLeft=leftMotor;
    dutyRight=rightMotor;
    if (!pwm1Started) {
        PWM_start(pwm1);
        pwm1Started = true;
    }
    if (!pwm2Started) {
        PWM_start(pwm2);
        pwm2Started = true;
    }
    PWM_setDuty(pwm1, dutyLeft);
    PWM_setDuty(pwm2, dutyRight);
}

void Motor::IncrementSpeed(int leftMotor, int rightMotor){
    dutyLeft+=leftMotor;
    dutyRight+=rightMotor;
    if (dutyLeft > pwmPeriod || dutyRight > pwmPeriod) {
        dutyLeft = 0;
        dutyRight = 0;

        GPIO_toggle(MOTOR_LEFT_DIRECTION);
        GPIO_toggle(MOTOR_RIGHT_DIRECTION);
    }
    this->SetSpeed(dutyLeft, dutyRight);
}


//void Motor::DriveRight(uint32_t leftDutyCycle, uint32_t rightDutyCycle){
//
//}
//
//void Motor::DriveLeft(uint32_t leftDutyCycle, uint32_t rightDutyCycle){
//
//}
//
//void Motor::DriveBackward(uint32_t leftDutyCycle, uint32_t rightDutyCycle){
//
//}
//
//void Motor::DriveForward(uint32_t leftDutyCycle, uint32_t rightDutyCycle){
//
//}
