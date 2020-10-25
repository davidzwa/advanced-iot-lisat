/*
 *  ======== main_tirtos.c ========
 */

#include <stdint.h>

/* POSIX Header files */
#include <pthread.h>

//#include <xdc/std.h>
//#include <xdc/runtime/Error.h>
//#include <xdc/runtime/System.h>

/* RTOS header files */
#include <ti/sysbios/BIOS.h>
//#include <ti/sysbios/knl/Clock.h>

/* Driver configuration */
#include <ti/drivers/Board.h>

extern void *mainThread(void *arg0);

/* Stack size in bytes */
#define THREADSTACKSIZE    1024

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

/*
 *  ======== main ========
 */
int main(void)
{
    pthread_t           thread;
    pthread_attr_t      attrs;
    struct sched_param  priParam;
    int                 retc;

    // Example Clock setup
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

    /* Call driver init functions */
    Board_init();

    /* Initialize the attributes structure with default values */
    pthread_attr_init(&attrs);

    /* Set priority, detach state, and stack size attributes */
    priParam.sched_priority = 1;
    retc = pthread_attr_setschedparam(&attrs, &priParam);
    retc |= pthread_attr_setdetachstate(&attrs, PTHREAD_CREATE_DETACHED);
    retc |= pthread_attr_setstacksize(&attrs, THREADSTACKSIZE);
    if (retc != 0) {
        /* failed to set attributes */
        while (1) {}
    }

    retc = pthread_create(&thread, &attrs, mainThread, NULL);
    if (retc != 0) {
        /* pthread_create() failed */
        while (1) {}
    }

    BIOS_start();

    return (0);
}
