#include "externalInterrupts.h"
#include "serialInterface.h"

// Timer: Auxiliary variables
extern unsigned long lastTriggerMic1 = 0;
extern unsigned long lastTriggerMic2 = 0;
extern unsigned long lastTriggerMic3 = 0;

boolean startAdcSampling = false;

// CASPER's PLAYGROUND
long getCurrentPreciseTime() {
    return micros();
}

void enableMicTriggerInterrupts() {
    // https://randomnerdtutorials.com/interrupts-timers-esp8266-arduino-ide-nodemcu/
    attachInterrupt(digitalPinToInterrupt(micTriggerPin), interruptMicTriggered, RISING);
    attachInterrupt(digitalPinToInterrupt(mic2TriggerPin), interruptMic2Triggered, RISING);
    attachInterrupt(digitalPinToInterrupt(mic3TriggerPin), interruptMic3Triggered, RISING);
}

void disableMicTriggerInterrupts() {
    detachInterrupt(digitalPinToInterrupt(micTriggerPin));
    detachInterrupt(digitalPinToInterrupt(mic2TriggerPin));
    detachInterrupt(digitalPinToInterrupt(mic3TriggerPin));
}

// Set the mic to a new Wake-on-sound cycle, sadly needs some delay
void resetWosMicModeSlow() {
    digitalWrite(wosModePin, LOW);
    delay(50);
    digitalWrite(wosModePin, HIGH);
}

// Set the mic to continuous listening
void setNormalMicMode()
{
    digitalWrite(wosModePin, LOW);
}

void adcTimerDoneCallback() {
    Serial.println("Done sampling!");
    
    // Process data here
    // long minTriggerVal = min(min(lastTrigger, lastTrigger2), min(lastTrigger2, lastTrigger3));
    
    transmitSerialData(analogBuffer, ADC_SAMPLES_COUNT);
    resetWosMicModeSlow();
    Serial.println("Reset WoS mode (50ms)!");
    startAdcSampling = false;
}

ICACHE_RAM_ATTR void interruptMicTriggered()
{
#ifdef PRINT_TRIGGER_INTERRUPTS
    Serial.println("m3");
#endif
    lastTriggerMic1 = getCurrentPreciseTime();
}

ICACHE_RAM_ATTR void interruptMic2Triggered()
{
#ifdef PRINT_TRIGGER_INTERRUPTS
    Serial.println("m2");
#endif
    lastTriggerMic2 = getCurrentPreciseTime();
}

ICACHE_RAM_ATTR void interruptMic3Triggered()
{
#ifdef PRINT_TRIGGER_INTERRUPTS
    Serial.println("m3");
#endif
    lastTriggerMic3 = getCurrentPreciseTime();
    startAdcSampling = true;
    startOsTimer(adcTimerDoneCallback);
}
