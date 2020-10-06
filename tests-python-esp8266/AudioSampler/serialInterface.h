#ifndef SERIAL_INTERFACE__H
#define SERIAL_INTERFACE__H

#include <Arduino.h>

extern bool transmitting;
String incoming = "";

void transmitSerialData(int* buffer, int length) {
    transmitting = true;
    for (int16_t i = 0; i < length; i++)
    {
        Serial.print("[v]");
        Serial.print(".");
        Serial.println(buffer[i]);
    }
}

void checkIncomingSerial()
{
    if (Serial.available() > 0)
    {
        incoming = Serial.readString();
        Serial.println(incoming);
        incoming = "";
    }
}

#endif // !SERIAL_INTERFACE__H
