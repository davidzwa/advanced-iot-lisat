//#include <ti/drivers/UART.h>
//#include "common.h"
//
////char        input;
////const char  echoPrompt[] = "Echoing characters:\r\n";
////UART_Handle uart;
////UART_Params uartParams;
//
//void initUARTESP() {
//    /* Create a UART with data processing off. */
//    UART_Params_init(&uartParams);
//    uartParams.writeDataMode = UART_DATA_BINARY;
//    uartParams.readDataMode = UART_DATA_BINARY;
//    uartParams.readReturnMode = UART_RETURN_FULL;
//    uartParams.readEcho = UART_ECHO_OFF;
//    uartParams.baudRate = 115200;
//}
//
//void openUARTESP() {
//    uart = UART_open(CONFIG_TIMER_0_US_MEASURE, &uartParams);
//
//    if (uart == NULL) {
//        /* UART_open() failed */
//        while (1);
//    }
//}
//
//void writeUARTInfinite() {
//    UART_write(uart, echoPrompt, sizeof(echoPrompt));
//
//    /* Loop forever echoing */
////    while (1) {
////        UART_read(uart, &input, 1);
////        UART_write(uart, &input, 1);
////    }
//}
