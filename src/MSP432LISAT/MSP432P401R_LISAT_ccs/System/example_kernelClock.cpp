/*
 * kernelClock.cpp
 *
 *  Created on: Oct 25, 2020
 *      Author: david
 */

//#include <xdc/std.h>
//#include <xdc/runtime/Error.h>
//#include <xdc/runtime/System.h>
//#include <ti/sysbios/knl/Clock.h>

/*
 *  ======== clk0Fxn =======
 */
//Void clk0Fxn(UArg arg0)
//{
//    UInt32 time;
//
//    time = Clock_getTicks();
//    System_printf("System time in clk0Fxn = %lu\n", (ULong)time);
//}

//void setupClockHandler() {
    // Example Clock setup, which shows that the microcontroller timerA0 is used on 1ms per tick,
    // giving the following handler an update rate of 1kHz

    // https://e2e.ti.com/support/microcontrollers/msp430/f/166/t/611213?RTOS-MSP432-RTOS-SYS-BIOS-Runtime-clock
    // https://dev.ti.com/tirex/explore/node?node=ABPpE.IWPsTrdNbHg2sxgw__z-lQYNj__1.40.01.00
//    Clock_Params clockParams;
//    Clock_Handle myClock;
//    Error_Block eb;
//    Error_init(&eb);
//    Clock_Params_init(&clockParams);
//    clockParams.period = 5;
//    clockParams.startFlag = TRUE;
//    clockParams.arg = (UArg)0x5555;
//    myClock = Clock_create(clk0Fxn, 5, &clockParams, &eb);
//    if (myClock == NULL) {
//        System_abort("Clock create failed");
//    }
//}
