#include "pindefs.h"
#include "audioAdcTimer.h"

#ifndef EXTERNAL_INTERRUPTS__H
#define EXTERNAL_INTERRUPTS__H

extern boolean startAdcSampling;

// CASPER's PLAYGROUND
long getCurrentPreciseTime();

void enableMicTriggerInterrupts();
void disableMicTriggerInterrupts();

ICACHE_RAM_ATTR void interruptMicTriggered();
ICACHE_RAM_ATTR void interruptMic2Triggered();
ICACHE_RAM_ATTR void interruptMic3Triggered();

#endif // !EXTERNAL_INTERRUPTS__H
