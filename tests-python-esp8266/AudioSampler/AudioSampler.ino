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

#include <ESP8266WiFi.h> // Include the Wi-Fi library

const char *ssid = "Brus";          // The SSID (name) of the Wi-Fi network you want to connect to
const char *password = "Paaswoord"; // The password of the Wi-Fi network

// TODOS
// - [ ] Parameter setting over serial or wireless
// - [ ] Storing parameters on flash
// - [ ] Customizing LPF/HPF
// - [ ] Better ping-pong buffer mechanism
// - [ ] Bigger buffer > circular chunk sliding-window within buffer, f.e. 64 samples

#define NO_BUFFER
// #define PRINT_TRIGGER_INTERRUPTS

#ifdef DEBUG
const int sampling_period_us = 50000; // 50ms, 20 Hz;
#define ADC_SAMPLES_COUNT 100
#else
const int sampling_period_us = 500; // 200us, 5 kHz;
#define ADC_SAMPLES_COUNT 1000
#endif

const int ledPin = LED_BUILTIN;
#define timeSecondsMs 50
// https://randomnerdtutorials.com/esp8266-pinout-reference-gpios/
const int analogInPin = A0;
const int micTriggerPin = D5;
const int mic2TriggerPin = D6;
const int mic3TriggerPin = D7;
const int wosModePin = 16;
int analogBuffer[ADC_SAMPLES_COUNT];
int analogBuffer_transmit[ADC_SAMPLES_COUNT];
long lastMicros = 0;
int16_t bufPosition = 0;
const int serial_baud_rate = 250000; // 1000000; // 2000000

// Jitter tracker (microseconds)
long avg_jitter_us = 0;

// States
bool transmitting = false;

os_timer_t samplingTimerUs;
long tickOccured = 0;
bool averaging = false;
int numAveragedSamples = 0;
float averagedAmplitude = 0.0f;
long integratedAmplitude = 0;
int maxAmplitude = 0;
int minAmplitude = 100024;
void timerCallback(void *pArg)
{
#ifdef NO_BUFFER
    int value = analogRead(analogInPin);

    //https://forum.arduino.cc/index.php?topic=448426.0
    Serial.print(-300); // To freeze the lower limit
    Serial.print(" ");
    Serial.print(700); // To freeze the upper limit
    Serial.print(" ");
    // Serial.print("v =");
    if (averaging)
    {
        averagedAmplitude = ((float)value + numAveragedSamples * averagedAmplitude) / (numAveragedSamples + 1);
        integratedAmplitude += bandpassEMA(value);
        maxAmplitude = max(value, maxAmplitude);
        minAmplitude = min(value, minAmplitude);
    }
    Serial.println(value);
    Serial.println(" ");
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

float EMA_a_low = 0.6; //initialization of EMA alpha
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

// Timer: Auxiliary variables
unsigned long now = millis();
unsigned long lastTrigger = 0;
unsigned long lastTrigger2 = 0;
unsigned long lastTrigger3 = 0;
boolean startTimer = false;
// Checks if motion was detected, sets LED HIGH and starts a timer
ICACHE_RAM_ATTR void interruptMicTriggered()
{
    numAveragedSamples = 0;
    averagedAmplitude = 0.0f;
    integratedAmplitude = 0;
    maxAmplitude = 0;
    minAmplitude = 10024;
    averaging = true;
    digitalWrite(ledPin, HIGH);
    startTimer = true;
    lastTrigger = micros();
#ifdef PRINT_TRIGGER_INTERRUPTS
    Serial.println("m3");
#endif
}

ICACHE_RAM_ATTR void interruptMic2Triggered()
{
#ifdef PRINT_TRIGGER_INTERRUPTS
    Serial.println("m2");
#endif
    lastTrigger2 = micros();
}

ICACHE_RAM_ATTR void interruptMic3Triggered()
{
#ifdef PRINT_TRIGGER_INTERRUPTS
    Serial.println("m3");
#endif
    lastTrigger3 = micros();
}

void setup()
{
    WiFi.disconnect();

    Serial.begin(serial_baud_rate);
    tickOccured = false;
    user_init();

    // Set LED to LOW
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, LOW);

    // Start listening for wake-on-sound
    // https://www.puiaudio.com/media/SpecSheet/PMM-3738-VM1010-R.pdf
    pinMode(wosModePin, OUTPUT);
    digitalWrite(wosModePin, HIGH);

    pinMode(micTriggerPin, INPUT_PULLUP);
    // https://randomnerdtutorials.com/interrupts-timers-esp8266-arduino-ide-nodemcu/
    attachInterrupt(digitalPinToInterrupt(micTriggerPin), interruptMicTriggered, RISING);
    attachInterrupt(digitalPinToInterrupt(mic2TriggerPin), interruptMic2Triggered, RISING);
    attachInterrupt(digitalPinToInterrupt(mic3TriggerPin), interruptMic3Triggered, RISING);
}

String incoming = "";
void checkIncomingSerial()
{
    if (Serial.available() > 0)
    {
        incoming = Serial.readString();
        Serial.println(incoming);

        incoming = "";
    }
}

void loop()
{
    // Current time
    now = millis();
    // Turn off the LED after the number of seconds defined in the timeSeconds variable
    if (startTimer && (now - lastTrigger > (timeSecondsMs)))
    {
        long minTriggerVal = min(min(lastTrigger, lastTrigger2), min(lastTrigger2, lastTrigger3));
        if (lastTrigger - minTriggerVal < 1000 && lastTrigger2 - minTriggerVal < 1000 && lastTrigger3 - minTriggerVal < 1000)
        {
            // Serial.print(lastTrigger - minTriggerVal);
            // Serial.print(" ");
            // Serial.print(lastTrigger2 - minTriggerVal);
            // Serial.print(" ");
            // Serial.println(lastTrigger3 - minTriggerVal);
        }
        digitalWrite(ledPin, LOW);
        digitalWrite(wosModePin, LOW);
        delay(100);
        averaging = false;
        digitalWrite(wosModePin, HIGH);
#ifdef PRINT_TRIGGER_INTERRUPTS
        Serial.print(integratedAmplitude);
        Serial.print(" ");
        Serial.print(minAmplitude);
        Serial.print(" ");
        Serial.print(maxAmplitude);
        // Serial.print(" ");
        // Serial.print(maxAmplitude-minAmplitude);
        Serial.print(" ");
        Serial.println(averagedAmplitude); // Print the energy
#endif
        startTimer = false;
    }

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
