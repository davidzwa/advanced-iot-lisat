/*
 * IrSensorsTimer.cpp
 *
 *  Created on: Nov 12, 2020
 *      Author: Tomas
 */

#include "common.h"
#include <System/IrSensorsTimer.h>

void initTimerIrParams(){
    Timer_Params_init(&params32);
    params32.periodUnits = Timer_PERIOD_COUNTS;
    params32.timerMode = Timer_ONESHOT_CALLBACK;
}

void initTimerIr() {
    /*
     * Setting up the timer in FREE RUNNING mode, allowing us to share it across multiple tasks.
     */
    initTimerIrParams();
    IrTimer = Timer_open(IR_LINE_TIMER32, &params32);
    if (IrTimer == NULL) {
        /* Failed to initialized timer */
        GPIO_write(LED_BLUE_2_GPIO, 1);
    }
    Timer_start(IrTimer);
}


uint32_t getTimerCountsIr() {
    return Timer_getCount(IrTimer);
}


