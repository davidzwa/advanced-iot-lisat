/*
 * wakeOnSoundLocalization.h
 *
 *  Created on: Nov 11, 2020
 *      Author: david
 */

#include <common.h>

#ifndef TDOA_WAKEONSOUNDLOCALIZATION_H_
#define TDOA_WAKEONSOUNDLOCALIZATION_H_

// CASPER's TDOA PLAYGROUND
extern unsigned long lastTriggerMic1L;
extern unsigned long lastTriggerMic2M;
extern unsigned long lastTriggerMic3R;
float outputDirVector2D_plane_cutting[2];
float outputDirVector2D_valin[2];

uint32_t getCurrentPreciseTime(); // Timer difference method
void resetWosMicMode();
void setWosMode(MIC micType);
void setNormalMicMode(MIC micType);
void initInterruptCallbacks();
void enableMicTriggerInterrupts();
void disableMicTriggerInterrupts();
void interruptMic1LTriggered(uint_least8_t index);
void interruptMic2MTriggered(uint_least8_t index);
void interruptMic3RTriggered(uint_least8_t index);

#endif /* TDOA_WAKEONSOUNDLOCALIZATION_H_ */
