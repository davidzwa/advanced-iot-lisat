/*
 * highSpeedTimer.cpp
 *
 *  Created on: Nov 12, 2020
 *      Author: Tomas
 */

#include <System/highSpeedTimer.h>
#include "common.h"

void initHighSpeedTimer(Timer_CallBackFxn callback) {
    /*
     * Setting up the high-speed timer in one-shot mode
     *
     * Period is set dynamically at usage
     */
    Timer_Params_init(&highSpeedTimerParams);
    highSpeedTimerParams.periodUnits = Timer_PERIOD_US;
    highSpeedTimerParams.timerMode = Timer_ONESHOT_CALLBACK;
    highSpeedTimerParams.timerCallback = callback;
    highSpeedTimer = Timer_open(TIMER_HIGH_SPEED, &highSpeedTimerParams);
    if (highSpeedTimer == NULL) {
        /* Failed to initialized timer */
        GPIO_write(LED_ERROR_2, 1);
    }
}

void StartHighSpeedTimer() {
    uint32_t status = Timer_start(highSpeedTimer);
    if (status != Timer_STATUS_SUCCESS) {
        GPIO_write(LED_ERROR_2, 1);
    }
}

void StopHighSpeedTimer() {
    Timer_stop(highSpeedTimer);
}

void SetPeriodUsHighSpeedTimer(uint32_t period_us) {
    uint32_t status = Timer_setPeriod(highSpeedTimer, Timer_PERIOD_US, period_us);
    if (status != Timer_STATUS_SUCCESS) {
        GPIO_write(LED_ERROR_2, 1);
    }
}

uint32_t getCountsHighSpeedTimer() {
    return Timer_getCount(highSpeedTimer);
}


