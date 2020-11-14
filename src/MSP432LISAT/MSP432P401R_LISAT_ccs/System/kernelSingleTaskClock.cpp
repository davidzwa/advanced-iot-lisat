/*
 * kernelSingleTaskClock.cpp
 *
 *  Created on: Nov 11, 2020
 *      Author: david
 */

#include <System/kernelSingleTaskClock.h>
#include "common.h"

KernelSingleTaskClock::KernelSingleTaskClock()
{
    // TODO Auto-generated constructor stub
}

void KernelSingleTaskClock::setupClockTask(uint32_t timeout, uint16_t periodClockTicks, void(*callback)() ) {
/*   Examples for clock setup:   */
//     https://dev.ti.com/tirex/explore/node?node=ABPpE.IWPsTrdNbHg2sxgw__z-lQYNj__1.40.01.00
//     https://e2e.ti.com/support/microcontrollers/msp430/f/166/t/611213?RTOS-MSP432-RTOS-SYS-BIOS-Runtime-clock

    Error_Block eb;
    Error_init(&eb);
    Clock_Params_init(&clockParams);
    clockParams.period = periodClockTicks;
    clockParams.startFlag = FALSE;
    clockParams.arg = (UArg)0x5555;

    myClock = Clock_create((Clock_FuncPtr) callback, timeout, &clockParams, &eb);
    if (myClock == NULL) {
        GPIO_write(LED_ERROR_2, 1);
        System_abort("Clock create failed");
    }
    Clock_setTimeout(myClock, timeout); // Important: if this function is not explicitly called the clock will not start
}

void KernelSingleTaskClock::startClockTask() {
    Clock_start(myClock);
}

void KernelSingleTaskClock::stopClockTask() {
    Clock_stop(myClock);
}

void KernelSingleTaskClock::adjustClockCallback(void(*callback)()) {
    Clock_setFunc(myClock, (Clock_FuncPtr) callback, NULL);
}

void KernelSingleTaskClock::setClockTimeout(uint32_t timeout) {
    Clock_setTimeout(myClock, timeout);
}

void KernelSingleTaskClock::setClockPeriod(uint16_t period) {
    Clock_setPeriod(myClock, period);
}
