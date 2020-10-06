#include "serialInterface.h"
#include "defines.h"
#include <Arduino.h>

// States
bool transmitting = false;

void transmitEndSymbol()
{
    Serial.println("--Done");
}

void transmitSerialData(transmittedData_t *data)
{
    transmitting = true;
    for (int16_t i = 0; i < data->length; i++)
    {
        transmitSerialValue(data->buffer[i]);
    }

    transmitSerialParam("m1", data->deltaMic1);
    transmitSerialParam("m2", data->deltaMic2);
    transmitSerialParam("m3", data->deltaMic3);
#ifdef MEASURE_ADCTIMER_JITTER
    Serial.print("-- Done. Jitter: ");
    Serial.print(avg_jitter_us);
#endif
    transmitEndSymbol(); // End of data separator

    Serial.print("Period(us): ");
    Serial.println(sampling_period_us);
    transmitting = false;
}

// Parameters dont get sent when in STREAM mode
void transmitSerialParam(String identifier, int value)
{
#ifndef STREAM_PLOTTER
    Serial.print(identifier);
    Serial.print(".");
    Serial.println(value);
#endif
}

void transmitSerialValue(int value)
{
#ifdef STREAM_PLOTTER
    Serial.print("v = ");
    Serial.println(value);
#else
    Serial.print("v");
    Serial.print(".");
    Serial.println(value);
#endif
}

String incoming = "";
void processIncomingSerial()
{
    if (Serial.available() > 0)
    {
        incoming = Serial.readString();
        Serial.println(incoming);
        incoming = "";
    }
}
