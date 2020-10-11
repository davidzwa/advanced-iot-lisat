#include <stdint.h>
#include "common.h"

#ifndef SERIAL_INTERFACE__H
#define SERIAL_INTERFACE__H

/* Display Driver Handle */
Display_Handle display;

struct transmittedData_t
{
    int deltaMic1;
    int deltaMic2;
    int deltaMic3;
    float tdoaDir1;
    float tdoaDir2;
    int length;
    int16_t *buffer;
    // Add any metadata here
};

extern bool transmitting;

void transmitSerialData(transmittedData_t *data);
void transmitSerialParam(char identifier, char identifier2, int value);
void transmitSerialValue(int value);

void processIncomingSerial();

#endif // !SERIAL_INTERFACE__H
