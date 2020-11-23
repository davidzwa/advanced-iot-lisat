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
#include <ti/drivers/PWM.h>
#include <ti/drivers/Board.h>

#include "SerialInterface/serialESPListener_task.h"

extern void *mainThread(void *arg0);
extern void *threadLoopESPSerial(void *arg0);

/* Stack size in bytes */
#define THREADSTACKSIZE          1024

/*
 *  ======== main ========
 */
int main(void)
{
    pthread_t           thread;
    pthread_attr_t      attrs;
    struct sched_param  priParam;
    int                 retc;

    /* Call driver init functions */
    Board_init();

    /* Call driver init functions */
    ADCBuf_init();
    PWM_init();
    Timer_init();
    GPIO_init();
    UART_init();

    /* Initialize the attributes structure with default values */
    pthread_attr_init(&attrs);

    // -- Main Thread
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
    retc = pthread_create(&thread, &attrs, mainThread, NULL);
    if (retc != 0) {
        /* pthread_create() failed */
        GPIO_write(LED_ERROR_2, 1);
        while (1) {}
    }

#if MSP_ESP_ROBOT_MODE==1
    // -- Serial ESP Thread
    priParam.sched_priority = 2;
    if (retc != 0) {
        /* failed to set attributes */
        GPIO_write(LED_ERROR_2, 1);
        while (1) {}
    }
    retc = pthread_create(&thread, &attrs, threadLoopESPSerial, NULL);
    if (retc != 0) {
        /* pthread_create() failed */
        GPIO_write(LED_ERROR_2, 1);
        while (1) {}
    }
#endif

    BIOS_start();

    return (0);
}
