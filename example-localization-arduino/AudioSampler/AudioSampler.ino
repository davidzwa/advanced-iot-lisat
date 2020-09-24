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

// TODOS
// - [ ] Parameter setting over serial or wireless
// - [ ] Storing parameters on flash
// - [ ] Customizing LPF/HPF
// - [ ] Better ping-pong buffer mechanism
// - [ ] Bigger buffer > circular chunk sliding-window within buffer, f.e. 64 samples

// #define NO_BUFFER

#ifdef DEBUG
const int sampling_period_us = 50000; // 50ms, 20 Hz;
#define ADC_SAMPLES_COUNT 100
#else
const int sampling_period_us = 500; // 200us, 5 kHz;
#define ADC_SAMPLES_COUNT 1000
#endif

const int analogInPin = A0;
int analogBuffer[ADC_SAMPLES_COUNT];
int analogBuffer_transmit[ADC_SAMPLES_COUNT];
long lastMicros = 0;
int16_t bufPosition = 0;
const int serial_baud_rate = 1000000; // 2000000

// Jitter tracker (microseconds)
long avg_jitter_us = 0;

// States
bool transmitting = false;

os_timer_t samplingTimerUs;
long tickOccured = 0;
void timerCallback(void *pArg)
{
#ifdef NO_BUFFER
    int value = analogRead(analogInPin);
    // Serial.print("v =");
    Serial.println(value);
    // Serial.print("\t f =");
    Serial.println(bandpassEMA(value));
#else
    // Stop recording if buffer is full! (Probably need circular buffer...)
    if (bufPosition < ADC_SAMPLES_COUNT)
    {
        // Calculate jitter average (IIR filter)
        // Serial.println(abs(micros() - lastMicros - sampling_period_us)); // Steady 200 - 500 us jitter
        // avg_jitter_us = (avg_jitter_us * (bufPosition) + abs(micros() - lastMicros - sampling_period_us)) / (bufPosition + 1); // Resort to jitter calc

        // Store filtered the sensor value using ADC
        analogBuffer[bufPosition] = bandpassEMA(analogRead(analogInPin));
        bufPosition++;
    }
    // else
    // {
    //     Serial.println("!Buffer overflow");
    // }
#endif
    lastMicros = micros();
}

// HPF EMA
// https://www.norwegiancreations.com/2016/03/arduino-tutorial-simple-high-pass-band-pass-and-band-stop-filtering/
float EMA_a = 0.2; //initialization of EMA alpha
int EMA_S = 0;     //initialization of EMA S
int filterValueEMA(int analogValue)
{
    EMA_S = (EMA_a * analogValue) + ((1 - EMA_a) * EMA_S); // run the EMA
    return analogValue;                                    // - EMA_S; // calculate the high-pass signal
}

float EMA_a_low = 0.5; //initialization of EMA alpha
float EMA_a_high = 0.9;
int EMA_S_low = 0; //initialization of EMA S
int EMA_S_high = 0;
int bandpassEMA(int analogValue)
{
    EMA_S_low = (EMA_a_low * analogValue) + ((1 - EMA_a_low) * EMA_S_low); //run the EMA
    EMA_S_high = (EMA_a_high * analogValue) + ((1 - EMA_a_high) * EMA_S_high);
    return EMA_S_high - EMA_S_low; //find the band-pass
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
    tickOccured = false;
    user_init();
}

String incoming = ""; // for incoming serial string data
void checkIncomingSerial()
{
    if (Serial.available() > 0)
    {
        // read the incoming:
        incoming = Serial.readString();
        // say what you got:
        // Serial.print("RX:");
        Serial.println(incoming);

        incoming = "";
    }
}

void loop()
{
#ifndef NO_BUFFER
    // Transmitting = true => overflow? Shouldnt be possible in main routine.
    if (bufPosition >= ADC_SAMPLES_COUNT && transmitting == false)
    {
        memcpy(analogBuffer_transmit, analogBuffer, sizeof(analogBuffer));
        yield();

        transmitting = true;
        for (int16_t i = 0; i < ADC_SAMPLES_COUNT; i++)
        {
            Serial.print("[v]");
            Serial.print(".");
            Serial.println(analogBuffer_transmit[i]);
        }
        Serial.print("-- Done. Jitter: ");
        Serial.print(avg_jitter_us);
        Serial.print(" Period(us): ");
        Serial.print(sampling_period_us);
        Serial.print(" bufPosition last: ");
        Serial.println(bufPosition);

        bufPosition = 0;
        transmitting = false;
    }
    else if (transmitting)
    {
        Serial.print("!Re-transmitting ");
    }

    checkIncomingSerial();
#endif       // !NO_BUFFER
    yield(); // or delay(0);
}
