#include "externalInterrupts.h"
#ifdef MIC3_MEASURE_SETUP
#include "serialInterface.h"
#include "tdoaAlgorithm.h"

// Timer: Auxiliary variables
unsigned long lastTriggerMic1L = 0;
unsigned long lastTriggerMic2M = 0;
unsigned long lastTriggerMic3R = 0;

boolean startAdcSampling = false;

// CASPER's PLAYGROUND
long getCurrentPreciseTime()
{
    return micros();
}

void enableMicTriggerInterrupts()
{
    // https://randomnerdtutorials.com/interrupts-timers-esp8266-arduino-ide-nodemcu/
    attachInterrupt(digitalPinToInterrupt(mic1LTriggerPin), interruptMic1LTriggered, RISING);
    attachInterrupt(digitalPinToInterrupt(mic2MTriggerPin), interruptMic2MTriggered, RISING);
    attachInterrupt(digitalPinToInterrupt(mic3RTriggerPin), interruptMic3RTriggered, RISING);
}

void disableMicTriggerInterrupts()
{
    detachInterrupt(digitalPinToInterrupt(mic1LTriggerPin));
    detachInterrupt(digitalPinToInterrupt(mic2MTriggerPin));
    detachInterrupt(digitalPinToInterrupt(mic3RTriggerPin));
}

// Set the mic to a new Wake-on-sound cycle, sadly needs some delay
void resetWosMicMode()
{
    // We dont need to wait as long as we do this at the interrupt!
    // setNormalMode(MIC_LEFT);
    // setNormalMode(MIC_MID);
    // setNormalMode(MIC_RIGHT);
    // delay(50);
    setWosMode(MIC_LEFT);
    setWosMode(MIC_MID);
    setWosMode(MIC_RIGHT);
}

// Set the mic to continuous listening
void setNormalMicMode(MIC pinNumber)
{
    digitalWrite(pinNumber, LOW);
}

void setWosMode(MIC pinNumber)
{
    digitalWrite(pinNumber, HIGH);
}

void adcTimerDoneCallback()
{
    // Process data here
    // long minTriggerVal = min(min(lastTrigger, lastTrigger2), min(lastTrigger2, lastTrigger3));
    // Or not.

    int inputTdoaVector2D[2];
    inputTdoaVector2D[0] = lastTriggerMic2M - lastTriggerMic1L;
    inputTdoaVector2D[1] = lastTriggerMic3R - lastTriggerMic1L;
    float outputDirVector2D[2];
    TDOA_direction_estimation(inputTdoaVector2D, outputDirVector2D);

    transmittedData_t serialData = {
        lastTriggerMic1L,
        lastTriggerMic2M,
        lastTriggerMic3R,
        outputDirVector2D[0],
        outputDirVector2D[1],
        ADC_SAMPLES_COUNT,
        analogBuffer};

    transmitSerialData(&serialData);
    resetWosMicMode(); // Reset all mics: we are ready for a new round
    startAdcSampling = false;
}

ICACHE_RAM_ATTR void interruptMic1LTriggered()
{
#ifdef PRINT_TRIGGER_INTERRUPTS
    Serial.println("m3");
#endif
    lastTriggerMic1L = getCurrentPreciseTime();
    setNormalMicMode(MIC_LEFT); // Disable interrupt externally
}

ICACHE_RAM_ATTR void interruptMic2MTriggered()
{
#ifdef PRINT_TRIGGER_INTERRUPTS
    Serial.println("m2");
#endif
    lastTriggerMic2M = getCurrentPreciseTime();
    setNormalMicMode(MIC_MID); // Disable interrupt externally
}

ICACHE_RAM_ATTR void interruptMic3RTriggered()
{
#ifdef PRINT_TRIGGER_INTERRUPTS
    Serial.println("m3");
#endif
    lastTriggerMic3R = getCurrentPreciseTime();
    startAdcSampling = true;
    setNormalMicMode(MIC_RIGHT); // Disable interrupt externally
    startOsTimer(adcTimerDoneCallback);
}
#endif
