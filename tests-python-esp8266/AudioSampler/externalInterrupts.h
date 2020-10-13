#include "pindefs.h"
#include "audioAdcTimer.h"

#ifndef EXTERNAL_INTERRUPTS__H
#define EXTERNAL_INTERRUPTS__H

#ifdef MIC3_MEASURE_SETUP
enum MIC
{
    MIC_LEFT = wosModePin1L,
    MIC_MID = wosModePin2M,
    MIC_RIGHT = wosModePin3R
};

extern boolean startAdcSampling;
extern unsigned long lastTriggerMic1;
extern unsigned long lastTriggerMic2;
extern unsigned long lastTriggerMic3;

// CASPER's PLAYGROUND
long getCurrentPreciseTime();
void resetWosMicMode();
void setWosMode(MIC micType);
void setNormalMicMode(MIC micType);
void enableMicTriggerInterrupts();
void disableMicTriggerInterrupts();

ICACHE_RAM_ATTR void interruptMic1LTriggered();
ICACHE_RAM_ATTR void interruptMic2MTriggered();
ICACHE_RAM_ATTR void interruptMic3RTriggered();
#endif 
#endif // !EXTERNAL_INTERRUPTS__H
