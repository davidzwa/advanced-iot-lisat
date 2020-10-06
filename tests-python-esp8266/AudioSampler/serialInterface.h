#ifndef SERIAL_INTERFACE__H
#define SERIAL_INTERFACE__H

#include <Arduino.h>

void transmitSerialData(int *buffer, int length);
void processIncomingSerial();


#endif // !SERIAL_INTERFACE__H
