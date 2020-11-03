/*
 * main_espSerial_task.cpp
 *
 *  Created on: Nov 3, 2020
 *      Author: david
 */

#include "serialESPListener_task.h"

#include <ti/drivers/UART.h>

#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Semaphore.h>

void loopESPSerial_task(void *arg0) {
    UART_init();
    initUARTESP();
    openUARTESP();

    waitUARTPacketInfinite();
}
