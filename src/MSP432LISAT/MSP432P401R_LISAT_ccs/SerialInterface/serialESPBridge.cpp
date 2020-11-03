#include <ti/drivers/UART.h>
#include <unistd.h>
#include <string.h>
#include "common.h"

char input;
char serialBuffer[64];
const char echoPrompt[] = "ack-MSP";
UART_Handle uart;
UART_Params uartParams;
sem_t uartbufSem; // Easy lock on receiving

// COMMANDS
const char INFO[] = "MSP!";

int parseHeader(char* buffer) {
    return strncmp(buffer, INFO, 4);
}

void uartCallbackReceived(UART_Handle handle, void* buffer, size_t count) {
    sem_post(&uartbufSem);
//    int result = parseHeader((char*)buffer);
//    if (result == 0) {
//
//    }
}

void initUARTESP() {
    /* Create a UART with data processing off. */
    UART_Params_init(&uartParams);
    uartParams.writeDataMode = UART_DATA_TEXT;
    uartParams.readDataMode = UART_DATA_TEXT;
    uartParams.readReturnMode = UART_RETURN_NEWLINE;
    uartParams.readMode = UART_MODE_CALLBACK;
    uartParams.readEcho = UART_ECHO_OFF;
    uartParams.readCallback = uartCallbackReceived;
    uartParams.baudRate = 115200;

    sem_init(&uartbufSem, 0, 0);
}

void openUARTESP() {
    uart = UART_open(COMM_ESP, &uartParams); // UART_DEBUG

    if (uart == NULL) {
        /* UART_open() failed */
        GPIO_write(LED_ERROR_2, 1);
        while (1);
    }
}

/*
 * A function to process UART continuously, which is only meant to be done in a task/co-routine context.
 */
void waitUARTPacketInfinite() {
    UART_write(uart, echoPrompt, sizeof(echoPrompt));

    while (1) {
        UART_read(uart, &serialBuffer, 64);
        sem_wait(&uartbufSem);

        int result = parseHeader(serialBuffer);
        if (result == 0) {
            GPIO_toggle(LED_TRIGGER_1);
        }
        else {
            // Drop the packet and
            GPIO_toggle(LED_ERROR_2);
        }
    }
}
