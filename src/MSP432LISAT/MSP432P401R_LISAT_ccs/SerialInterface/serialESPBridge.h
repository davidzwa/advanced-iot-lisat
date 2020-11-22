#ifndef SERIALINTERFACE_UART_ESP_H_
#define SERIALINTERFACE_UART_ESP_H_

void initUARTESP();
void initWakeupSem();
void openUARTESP();
void waitUARTPacketInfinite();

#endif /* SERIALINTERFACE_UART_ESP_H_ */
