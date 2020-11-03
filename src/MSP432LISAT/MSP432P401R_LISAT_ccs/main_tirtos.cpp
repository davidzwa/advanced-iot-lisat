/*
 *  ======== main_tirtos.c ========
 */

#include <stdint.h>

/* POSIX Header files */
#include <pthread.h>

/* RTOS header files */
#include <ti/sysbios/BIOS.h>

/* Driver configuration */
#include "common.h"
#include <ti/drivers/GPIO.h>
#include <ti/drivers/Board.h>

#include "SerialInterface/serialESPListener_task.h"

extern void *mainThread(void *arg0);
extern void *loopESPSerial_task(void *arg0);

/* Stack size in bytes */
#define THREADSTACKSIZE          1024
#define SLAVE_THREADSTACKSIZE    256

/*
 *  ======== main ========
 */
int main(void)
{
    GPIO_init();

    pthread_t           mainThreadObject;
    pthread_attr_t      attrs;
    struct sched_param  priParam;
    pthread_t           espSerialThreadObject;
    pthread_attr_t      espThreadAttrs;
    struct sched_param  espPriParam;
    int                 retc;


    /* Call driver init functions */
    Board_init();

    /* Initialize the attributes structure with default values */
    pthread_attr_init(&attrs);
    pthread_attr_init(&espThreadAttrs);

    /* Set priority, detach state, and stack size attributes */
    priParam.sched_priority = 1;
    retc = pthread_attr_setschedparam(&attrs, &priParam);
    retc |= pthread_attr_setdetachstate(&attrs, PTHREAD_CREATE_DETACHED);
    retc |= pthread_attr_setstacksize(&attrs, THREADSTACKSIZE);
    if (retc != 0) {
        /* failed to set attributes */
        GPIO_write(LED_ERROR_2, 1);
        while (1) {}
    }
    // Create the thread
    retc = pthread_create(&mainThreadObject, &attrs, mainThread, NULL);
    if (retc != 0) {
        /* pthread_create() failed */
        GPIO_write(LED_ERROR_2, 1);
        while (1) {}
    }

    espPriParam.sched_priority = 2;
    retc = pthread_attr_setschedparam(&espThreadAttrs, &espPriParam);
    retc |= pthread_attr_setdetachstate(&espThreadAttrs, PTHREAD_CREATE_DETACHED);
    retc |= pthread_attr_setstacksize(&espThreadAttrs, SLAVE_THREADSTACKSIZE);
    if (retc != 0) {
        /* failed to set attributes */
        GPIO_write(LED_ERROR_2, 1);
        while (1) {}
    }
    retc = pthread_create(&espSerialThreadObject, &espThreadAttrs, loopESPSerial_task, NULL);
    if (retc != 0) {
        /* pthread_create() failed */
        GPIO_write(LED_ERROR_2, 1);
        while (1) {}
    }

    BIOS_start();

    return (0);
}
