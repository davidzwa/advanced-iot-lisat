#ifndef SERIAL_INTERFACE__H
#define SERIAL_INTERFACE__H

#include <Arduino.h>

extern bool transmitting;

void transmitSerialData(int *buffer, int length);
void transmitSerialValue(int value);

void processIncomingSerial();


#endif // !SERIAL_INTERFACE__H
