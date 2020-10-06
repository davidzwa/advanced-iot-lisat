// Intellisense
// https://stackoverflow.com/questions/51614871/why-wont-arduino-intellisense-work-in-vscode

#include "pindefs.h"
#include "defines.h"
#include "audioAdcTimer.h"
#include "externalInterrupts.h"
#include "serialInterface.h"

#ifdef MEASURE_ADCTIMER_JITTER
extern long avg_jitter_us;
#endif

// States
bool transmitting = false;

void user_init(void)
{
    // ADC timer
    initOsTimer(sampling_period_us);
    
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, LOW);

    // Start listening for wake-on-sound
    pinMode(wosModePin, OUTPUT); // https://www.puiaudio.com/media/SpecSheet/PMM-3738-VM1010-R.pdf
    digitalWrite(wosModePin, HIGH);
    pinMode(micTriggerPin, INPUT_PULLUP);
    // https://randomnerdtutorials.com/interrupts-timers-esp8266-arduino-ide-nodemcu/
    attachInterrupt(digitalPinToInterrupt(micTriggerPin), interruptMicTriggered, RISING);
    attachInterrupt(digitalPinToInterrupt(mic2TriggerPin), interruptMic2Triggered, RISING);
    attachInterrupt(digitalPinToInterrupt(mic3TriggerPin), interruptMic3Triggered, RISING);
}

void setup()
{
    WiFi.disconnect();
    Serial.begin(serial_baud_rate);
    user_init();
}

void loop()
{
    // Current time
    now = millis();
    // Turn off the LED after the number of seconds defined in the timeSeconds variable
    if (startTimer && (now - lastTrigger > (timeSecondsMs)))
    {
        // long minTriggerVal = min(min(lastTrigger, lastTrigger2), min(lastTrigger2, lastTrigger3));
        digitalWrite(ledPin, LOW);
        digitalWrite(wosModePin, LOW);
        delay(100);
        digitalWrite(wosModePin, HIGH);
        startTimer = false;
    }

#ifndef NO_BUFFER
    if (transmitting == false)
    {

#ifdef MEASURE_ADCTIMER_JITTER
        Serial.print("-- Done. Jitter: ");
        Serial.print(avg_jitter_us);
#endif
        Serial.print(" Period(us): ");
        Serial.print(sampling_period_us);
        transmitting = false;
    }
    checkIncomingSerial();

#endif       // !NO_BUFFER
    yield(); // or delay(0);
}
