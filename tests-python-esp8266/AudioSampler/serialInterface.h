#ifndef SERIAL_INTERFACE__H
#define SERIAL_INTERFACE__H

#include <Arduino.h>

struct transmittedData_t
{
    int deltaMic1;
    int deltaMic2;
    int deltaMic3;
    float tdoaDir1;
    float tdoaDir2;
    int length;
    int *buffer;
    // Add any metadata here
};

extern bool transmitting;

void transmitSerialData(transmittedData_t *data);
void transmitSerialParam(String identifier, int value);
void transmitSerialValue(int value);

void processIncomingSerial();

#endif // !SERIAL_INTERFACE__H
