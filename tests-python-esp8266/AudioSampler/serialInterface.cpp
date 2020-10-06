#include "serialInterface.h"
#include "defines.h"
#include <Arduino.h>

// States
bool transmitting = false;

void transmitSerialData(int *buffer, int length)
{
    transmitting = true;
    for (int16_t i = 0; i < length; i++)
    {
        transmitSerialValue(buffer[i]);
    }
    
#ifdef MEASURE_ADCTIMER_JITTER
    Serial.print("-- Done. Jitter: ");
    Serial.print(avg_jitter_us);
#endif

    Serial.print("Period(us): ");
    Serial.println(sampling_period_us);
    transmitting = false;
}

void transmitSerialValue(int value) {
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
