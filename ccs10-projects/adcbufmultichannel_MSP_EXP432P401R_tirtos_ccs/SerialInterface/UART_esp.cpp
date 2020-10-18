#include <ti/drivers/UART.h>
#include <unistd.h>
#include <string.h>
#include "common.h"

char input;
char serialBuffer[64];
const char echoPrompt[] = "Response. Device OK.\r\n";
UART_Handle uart;
UART_Params uartParams;
sem_t uartbufSem; // Easy lock on receiving

// COMMANDS
const char INFO[] = "info";

int parsePacket(char* buffer) {
    return strncmp(buffer, INFO, 4);
}

void uartCallbactReceived(UART_Handle handle, void* buffer, size_t count) {
    sem_post(&uartbufSem);
}

void initUARTESP() {
    /* Create a UART with data processing off. */
    UART_Params_init(&uartParams);
    uartParams.writeDataMode = UART_DATA_TEXT;
    uartParams.readDataMode = UART_DATA_TEXT;
    uartParams.readReturnMode = UART_RETURN_NEWLINE;
    uartParams.readMode = UART_MODE_CALLBACK;
    uartParams.readEcho = UART_ECHO_OFF;
    uartParams.readCallback = uartCallbactReceived;
    uartParams.baudRate = 115200;

    sem_init(&uartbufSem, 0, 0);
}

void openUARTESP() {
    uart = UART_open(UART_DEBUG, &uartParams);

    if (uart == NULL) {
        /* UART_open() failed */
        while (1);
    }
}

void writeUARTInfinite() {
    UART_write(uart, echoPrompt, sizeof(echoPrompt));

    /* Loop forever echoing */
    while (1) {
        UART_read(uart, &serialBuffer, 64);
        sem_wait(&uartbufSem);

        int result = parsePacket(serialBuffer);
        if (result == 0) {
            UART_write(uart, echoPrompt, sizeof(echoPrompt));
        }
    }
}
