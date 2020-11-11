/*
 * kernelSingleTaskClock.cpp
 *
 *  Created on: Nov 11, 2020
 *      Author: david
 */

#include <System/kernelSingleTaskClock.h>

KernelSingleTaskClock::KernelSingleTaskClock()
{
    // TODO Auto-generated constructor stub

}

void taskDelayCallback(UArg arg0)
{
    UInt32 time;
    time = Clock_getTicks();
}

void KernelSingleTaskClock::setupClockHandler() {
//     Example Clock setup, which shows that the microcontroller timerA0 is used on 1ms per tick,
//     giving the following handler an update rate of 1kHz

//     https://e2e.ti.com/support/microcontrollers/msp430/f/166/t/611213?RTOS-MSP432-RTOS-SYS-BIOS-Runtime-clock
//     https://dev.ti.com/tirex/explore/node?node=ABPpE.IWPsTrdNbHg2sxgw__z-lQYNj__1.40.01.00

    Error_Block eb;
    Error_init(&eb);
    Clock_Params_init(&clockParams);
    clockParams.period = 5;
    clockParams.startFlag = FALSE;
    clockParams.arg = (UArg)0x5555;

    myClock = Clock_create(taskDelayCallback, 5000, &clockParams, &eb);
    if (myClock == NULL) {
        System_abort("Clock create failed");
    }
}

void KernelSingleTaskClock::scheduleSingleTask(uint32_t period) {
    Clock_setTimeout(myClock, period);
    Clock_start(myClock);
}
