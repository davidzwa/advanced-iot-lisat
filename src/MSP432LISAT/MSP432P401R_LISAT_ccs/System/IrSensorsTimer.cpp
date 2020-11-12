/*
 * IrSensorsTimer.cpp
 *
 *  Created on: Nov 12, 2020
 *      Author: Tomas
 */

#include "common.h"
#include <System/IrSensorsTimer.h>

void initIrTimer(Timer_CallBackFxn callback) {
    /*
     * Setting up the IR timer in one-shot mode
     *
     * Period is set dynamically at usage (irSensors.cpp)
     */
    Timer_Params_init(&irTimerParams);
    irTimerParams.periodUnits = Timer_PERIOD_US;
    irTimerParams.timerMode = Timer_ONESHOT_CALLBACK;
    irTimerParams.timerCallback = callback;
    irTimer = Timer_open(IR_LINE_TIMER32, &irTimerParams);
    if (irTimer == NULL) {
        /* Failed to initialized timer */
        GPIO_write(LED_ERROR_2, 1);
    }
}

void irTimerStart() {
    uint32_t status = Timer_start(irTimer);
    if (status != Timer_STATUS_SUCCESS) {
        GPIO_write(LED_ERROR_2, 1);
    }
}

void irTimerStop() {
    Timer_stop(irTimer);
}

void irTimerSetPeriodUs(uint32_t period_us) {
    uint32_t status = Timer_setPeriod(irTimer, Timer_PERIOD_US, period_us);
    if (status != Timer_STATUS_SUCCESS) {
        GPIO_write(LED_ERROR_2, 1);
    }
}

uint32_t getTimerCountsIr() {
    return Timer_getCount(irTimer);
}


