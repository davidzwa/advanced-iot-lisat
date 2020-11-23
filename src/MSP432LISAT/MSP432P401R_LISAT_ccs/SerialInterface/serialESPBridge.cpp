#include <ti/drivers/UART.h>
#include <unistd.h>
#include <string.h>
#include "common.h"
#include <stdio.h>
#include <string>

char input;
char serialBuffer[64];
const char echoPrompt[] = "ack-MSP";
UART_Handle uart;
UART_Params uartParams;
sem_t uartbufSem; // Easy lock on receiving
sem_t mqttWakeupSem;

void processCommand();

// COMMANDS
const char INFO[] = "MSP!";

int parseHeader(char* buffer) {
    return strncmp(buffer, INFO, 4);
}

void uartCallbackReceived(UART_Handle handle, void* buffer, size_t count) {
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
    uartParams.readCallback = uartCallbackReceived;
    uartParams.baudRate = 115200;

    sem_init(&uartbufSem, 0, 0);
}

void initWakeupSem() {
    sem_init(&mqttWakeupSem, 0, 0);
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
            GPIO_toggle(LED_GREEN_2);
            sem_post(&mqttWakeupSem);
            processCommand();
        }
        else {
            // Drop the packet and
            GPIO_toggle(LED_ERROR_2);
        }
    }
}


void processCommand() {
    char messageType = serialBuffer[4];
    switch(messageType) {
        case 'm': { // change mode
            std::string modeString;
            modeString += serialBuffer[5];
            int modeValue = stoi(modeString);
            if (modeValue == 0) {
                changeMode(INTER_DRIVING);
            }
            else if (modeValue == 1) {
                changeMode(FIND_WAITING);
            } else {
                GPIO_toggle(LED_ERROR_2);
            }
            break;
        }
        case 's': {// change speed, bracket required to initialize string
            std::string speedString;
            int i = 5; // start after message type buffer index
            while (serialBuffer[i] != 'x' && i < 15) { // speed value can have at most 15 - 5 = 10 digits, hence ( < 15 )
                speedString += serialBuffer[i];
                i++;
            }
            uint16_t speedValue = stoi(speedString);
            changeMotorSpeed(speedValue);
            break;
        }
        case 'r': {
            std::string rotateString;
            rotateString += serialBuffer[5];
            int rotateDirection = stoi(rotateString);
            if (rotateDirection == 0) {
                turnLeft(90);
            }
            else if (rotateDirection == 1) {
                turnRight(90);
            } else {
                GPIO_toggle(LED_ERROR_2);
            }
            break;
        }
        case 'a': // abort robots
            panicStop();
            break;
        default:
            //GPIO_toggle(LED_ERROR_2);
    }
}
