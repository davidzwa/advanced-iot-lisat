#include "common.h"
#include <ti/drivers/PWM.h>

#ifndef ROBOT_MOTOR_H_
#define ROBOT_MOTOR_H_

enum MOTOR_DIRECTION {
    BACKWARDS=1,
    FORWARDS=0
};

#define PERIOD_US 7000
#define SPEED_MAX 4000
class MotorDriver
{
public:
    MotorDriver();
    void Initialize();
    void Drive(int16_t speed_left, int16_t speed_right);
    void DriveLeft(uint16_t,int16_t curve);
    void DriveRight(uint16_t,int16_t curve);
    void DriveForwards(uint16_t);
    void DriveBackwards(uint16_t);
    void PowerUp();
    void PowerDown();
protected:
    void AwakeDriver();
    void SleepDriver();
    void CheckPWMStarted();
    void CheckPWMStopped();
    void SetMotorDirection(uint8_t, MOTOR_DIRECTION);
    void SetMotorSpeedLimited(PWM_Handle, uint16_t, uint16_t*);
private:
    /* Period and duty in microseconds */
    uint16_t pwmPeriod = PERIOD_US;
    uint16_t dutyLeft = 0;
    uint16_t dutyRight = 0;

    /* Sleep time in microseconds */
    uint32_t   time = 50000;
    PWM_Handle pwmLeft = NULL;
    PWM_Handle pwmRight = NULL;
    bool motorPowered = false;
    bool pwm1Started = false;
    bool pwm2Started = false;
    PWM_Params params;
};

#endif /* ROBOT_MOTOR_H_ */
