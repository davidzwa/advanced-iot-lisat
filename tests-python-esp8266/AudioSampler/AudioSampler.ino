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

// Adc & serial states
bool transmitting = false;

void user_init(void)
{
    // ADC timer
    initOsTimer(sampling_period_us);
    
    // LED as visual tool
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, LOW);

    // Start listening for wake-on-sound
    pinMode(micTriggerPin, INPUT_PULLUP);
    pinMode(wosModePin, OUTPUT); // https://www.puiaudio.com/media/SpecSheet/PMM-3738-VM1010-R.pdf
    digitalWrite(wosModePin, HIGH);   
}

void setup()
{
    WiFi.disconnect();
    Serial.begin(serial_baud_rate);
    user_init();
    
    // Prepare runtime
    enableMicTriggerInterrupts();
    initOsTimer(ADC_SAMPLING_PERIOD_US);
}

unsigned long now = millis();
void loop()
{
    // Current time
    if (startAdcSampling)
    {
        digitalWrite(ledPin, LOW);
    }
    else {
        digitalWrite(ledPin, HIGH);
    }

    processIncomingSerial();
    yield(); // or delay(0);
}
