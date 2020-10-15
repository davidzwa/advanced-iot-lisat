#include <ti/drivers/PWM.h>

#ifndef ROBOT_MOTOR_H_
#define ROBOT_MOTOR_H_

#define PERIOD_US 10000
class Motor
{
public:
    Motor();
    void Initialize();
    void SetSpeed(int left, int right);
    void IncrementSpeed(int left, int right);
    void Stop();
//    DriveRight(uint32_t leftDutyCycle, uint32_t rightDutyCycle);
    void MotorStop();
private:
    /* Period and duty in microseconds */
    uint16_t pwmPeriod = PERIOD_US;
    uint16_t dutyLeft = 0;
    uint16_t dutyRight = 0;

    /* Sleep time in microseconds */
    uint32_t   time = 50000;
    PWM_Handle pwm1 = NULL;
    PWM_Handle pwm2 = NULL;
    bool pwm1Started = false;
    bool pwm2Started = false;
    PWM_Params params;
};

#endif /* ROBOT_MOTOR_H_ */
