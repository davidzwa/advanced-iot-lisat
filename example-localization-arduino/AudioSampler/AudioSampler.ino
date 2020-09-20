// Source: https://www.switchdoc.com/2015/10/iot-esp8266-timer-tutorial-arduino-ide/
// 2nd source: https://www.esp8266.com/viewtopic.php?f=32&t=16194

// Intellisense
// https://stackoverflow.com/questions/51614871/why-wont-arduino-intellisense-work-in-vscode

extern "C"
{
#define USE_US_TIMER
#include "user_interface.h"
#include "osapi.h"
#define os_timer_arm_us(a, b, c) ets_timer_arm_new(a, b, c, 0)
}

#define ADC_SAMPLES_COUNT 1000
const int analogInPin = A0;
int analogBuffer[ADC_SAMPLES_COUNT];
int analogBuffer_transmit[ADC_SAMPLES_COUNT];
long lastMicros = 0;
int timeDiffBuffer[ADC_SAMPLES_COUNT];
int timeDiffBuffer_transmit[ADC_SAMPLES_COUNT];
int16_t bufPosition = 0;
const int sampling_period_us = 500;
const int serial_baud_rate = 1000000; // 2000000

// States
bool transmitting = false;

os_timer_t samplingTimerUs;
long tickOccured = 0;
void timerCallback(void *pArg)
{
    // Stop recording if buffer is full! (Probably need circular buffer...)
    if (bufPosition < ADC_SAMPLES_COUNT) {
        timeDiffBuffer[bufPosition] = micros() - lastMicros;
        analogBuffer[bufPosition] = analogRead(analogInPin);
        
        bufPosition++;
    }
    else {
        Serial.println("!Buffer overflow");
    }
    lastMicros = micros();
}

void user_init(void)
{
    lastMicros = micros();
    system_timer_reinit();
    os_timer_setfn(&samplingTimerUs, timerCallback, analogBuffer);
    os_timer_arm_us(&samplingTimerUs, sampling_period_us, true);
}

void setup()
{
    Serial.begin(serial_baud_rate);
    Serial.println();
    Serial.println();
    Serial.println("");
    Serial.println("--------------------------");
    Serial.println("ESP8266 Timer Test");
    Serial.println("--------------------------");
    tickOccured = false;
    user_init();
}

void loop()
{
    // Transmitting = true => overflow? Shouldnt be possible in main routine.
    if (bufPosition >= ADC_SAMPLES_COUNT && transmitting == false)
    {
        memcpy(analogBuffer_transmit, analogBuffer, sizeof(analogBuffer));
        memcpy(timeDiffBuffer_transmit, timeDiffBuffer, sizeof(timeDiffBuffer));
        yield();

        transmitting = true;
        for (int16_t i = 0; i < ADC_SAMPLES_COUNT; i++)
        {
            Serial.print("[i,t,v]:");
            Serial.print(i);
            Serial.print(".");
            Serial.print(timeDiffBuffer_transmit[i]);
            Serial.print(".");
            Serial.println(analogBuffer_transmit[i]);
        }
        Serial.print("-- Done. bufPosition was ");
        Serial.println(bufPosition);

        bufPosition = 0;
        transmitting = false;
    }
    else if (transmitting)
    {
        Serial.print("!Re-transmitting ");
    }
    yield(); // or delay(0);
}
