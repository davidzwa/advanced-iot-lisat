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
#ifdef MIC3_MEASURE_SETUP
    transmitSerialParam("M1", data->deltaMic1);
    transmitSerialParam("M2", data->deltaMic2);
    transmitSerialParam("M3", data->deltaMic3);
    transmitSerialParam("D1", data->tdoaDir1);
    transmitSerialParam("D2", data->tdoaDir2);
#ifdef MEASURE_ADCTIMER_JITTER
    transmitSerialParam("J", avg_jitter_us);
#endif
    transmitSerialParam("P", sampling_period_us);
#endif
    transmitEndSymbol(); // End of data separator
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
        // Serial.println(incoming); // MSP doesnt like garbage back at him
        incoming = "";
    }
}
