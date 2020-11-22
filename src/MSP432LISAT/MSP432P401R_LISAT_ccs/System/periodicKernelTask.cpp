/*
 * kernelSingleTaskClock.cpp
 *
 *  Created on: Nov 11, 2020
 *      Author: david
 */

#include <System/periodicKernelTask.h>
#include "common.h"

PeriodicKernelTask::PeriodicKernelTask()
{

}

void PeriodicKernelTask::setupClockMethod(uint32_t timeout, uint16_t periodClockTicks, void(*callback)(UArg), void* this_pointer) {
    Error_Block eb;
    Error_init(&eb);
    Clock_Params_init(&clockParams);
    clockParams.period = periodClockTicks;
    clockParams.startFlag = FALSE;
    clockParams.arg = (UArg)this_pointer;

    myClock = Clock_create((Clock_FuncPtr) callback, timeout, &clockParams, &eb);
    if (myClock == NULL) {
        GPIO_write(LED_ERROR_2, 1);
        System_abort("Clock create failed");
    }
    Clock_setTimeout(myClock, timeout); // Important: if this function is not explicitly called the clock will not start
}

void PeriodicKernelTask::setupClockTask(uint32_t timeout, uint16_t periodClockTicks, void(*callback)()) {
/*   Examples for clock setup:   */
//     https://dev.ti.com/tirex/explore/node?node=ABPpE.IWPsTrdNbHg2sxgw__z-lQYNj__1.40.01.00
//     https://e2e.ti.com/support/microcontrollers/msp430/f/166/t/611213?RTOS-MSP432-RTOS-SYS-BIOS-Runtime-clock

    Error_Block eb;
    Error_init(&eb);
    Clock_Params_init(&clockParams);
    clockParams.period = periodClockTicks;
    clockParams.startFlag = FALSE;

    myClock = Clock_create((Clock_FuncPtr) callback, timeout, &clockParams, &eb);
    if (myClock == NULL) {
        GPIO_write(LED_ERROR_2, 1);
        System_abort("Clock create failed");
    }
    Clock_setTimeout(myClock, timeout); // Important: if this function is not explicitly called the clock will not start
}

void PeriodicKernelTask::startClockTask() {
    Clock_start(myClock);
}

void PeriodicKernelTask::stopClockTask() {
    Clock_stop(myClock);
}

void PeriodicKernelTask::adjustClockCallback(void(*callback)()) {
    Clock_setFunc(myClock, (Clock_FuncPtr) callback, NULL);
}

void PeriodicKernelTask::setClockTimeout(uint32_t timeout) {
    Clock_setTimeout(myClock, timeout);
}

void PeriodicKernelTask::setClockPeriod(uint16_t period) {
    Clock_setPeriod(myClock, period);
}
