/*
 * wakeOnSoundLocalization.cpp
 *
 *  Created on: Nov 11, 2020
 *      Author: david
 */

#include <Example/wakeOnSoundLocalization.h>
#include "System/freeRunningTimer.h"

// WoS measurements
unsigned long lastTriggerMic1L = 0;
bool mic1LTriggered = false;
unsigned long lastTriggerMic2M = 0;
bool mic2MTriggered = false;
unsigned long lastTriggerMic3R = 0;
bool mic3RTriggered = false;
bool timerStarted = false;

/* usage example:
 *  resetWosMicMode();
 *  initInterruptCallbacks();
 *  enableMicTriggerInterrupts();
 */
//void adcBufCompleted() {
    //    if(mic1LTriggered && mic2MTriggered && mic3RTriggered)
    //    {
    //        int inputTDOAVector2D[2];
    //        inputTDOAVector2D[0] = lastTriggerMic1L - lastTriggerMic2M;
    //        inputTDOAVector2D[1] = lastTriggerMic1L - lastTriggerMic3R;
    //        TDOA_direction_estimation(inputTDOAVector2D, outputDirVector2D_valin);
    //
    //        unsigned long inputTOAVector[3] = {lastTriggerMic1L, lastTriggerMic2M, lastTriggerMic3R};
    //        plane_cutting_direction_estimation(inputTOAVector, outputDirVector2D_plane_cutting);
    //    }
    //    else {
    //        GPIO_toggle(LED_ERROR_2);
    //    }
    //    stopTimerIfStarted();
    //    enableMicTriggerInterrupts();
    //    resetWosMicMode(); // Reset all mics: we are ready for a new round

    //    if(mic1LTriggered && mic2MTriggered && mic3RTriggered)
    //    {
    //             /* post adcbuf semaphore */
    //    }
    //    mic1LTriggered = false;
    //    mic2MTriggered = false;
    //    mic3RTriggered = false;
//}

void startTimerIfStopped() {
    if (timerStarted == false) {
        startTimerTacho();
        timerStarted = true;
    }
}

void stopTimerIfStarted() {
    if (timerStarted == true) {
        stopTimerTacho();
        timerStarted = false;
    }
}

// CASPER's PLAYGROUND
uint32_t getCurrentPreciseTime()
{
    // Timer example
    // https://dev.ti.com/tirex/explore/node?node=AKyVym.I2F89E.1HEd4gnA__z-lQYNj__LATEST
    return getTimerUsTacho(); // return time in us
}

void initInterruptCallbacks() {
    GPIO_setCallback(MIC1L_D_OUT_INTRPT, interruptMic1LTriggered);
    GPIO_setCallback(MIC2M_D_OUT_INTRPT, interruptMic2MTriggered);
    GPIO_setCallback(MIC3R_D_OUT_INTRPT, interruptMic3RTriggered);
}

void enableMicTriggerInterrupts()
{
    GPIO_clearInt(MIC1L_D_OUT_INTRPT);
    GPIO_clearInt(MIC2M_D_OUT_INTRPT);
    GPIO_clearInt(MIC3R_D_OUT_INTRPT);
    GPIO_enableInt(MIC1L_D_OUT_INTRPT);
    GPIO_enableInt(MIC2M_D_OUT_INTRPT);
    GPIO_enableInt(MIC3R_D_OUT_INTRPT);
}

void disableMicTriggerInterrupts()
{
    GPIO_disableInt(MIC1L_D_OUT_INTRPT);
    GPIO_disableInt(MIC2M_D_OUT_INTRPT);
    GPIO_disableInt(MIC3R_D_OUT_INTRPT);
}

// Set the mic to a new Wake-on-sound cycle
// Dont need delay, because we use ADCBuf fixed timing for this (but it needs delay normally).
void resetWosMicMode()
{
    setWosMode(MIC_LEFT);
    setWosMode(MIC_MID);
    setWosMode(MIC_RIGHT);
}

// Set the mic to continuous listening
void setNormalMicMode(MIC pinNumber)
{
    GPIO_write(pinNumber, 0);
}

void setWosMode(MIC pinNumber)
{
    GPIO_write(pinNumber, 1);
}

void interruptMic1LTriggered(uint_least8_t index)
{
    if (mic1LTriggered == true) {
        GPIO_toggle(LED_ERROR_2);
        return;
    }
    startTimerIfStopped();
    lastTriggerMic1L = getCurrentPreciseTime();
    mic1LTriggered = true;
    setNormalMicMode(MIC_LEFT); // Disable interrupt externally
    GPIO_disableInt(MIC1L_D_OUT_INTRPT);
}

void interruptMic2MTriggered(uint_least8_t index)
{
    if (mic2MTriggered == true) {
        GPIO_toggle(LED_ERROR_2);
        return;
    }
    startTimerIfStopped();
    lastTriggerMic2M = getCurrentPreciseTime();
    mic2MTriggered = true;
    setNormalMicMode(MIC_MID); // Disable interrupt externally
    GPIO_disableInt(MIC2M_D_OUT_INTRPT);
}

void interruptMic3RTriggered(uint_least8_t index)
{
    if (mic3RTriggered == true) {
        GPIO_toggle(LED_ERROR_2);
        return;
    }
    startTimerIfStopped();
    lastTriggerMic3R = getCurrentPreciseTime();
    mic3RTriggered = true;
    setNormalMicMode(MIC_RIGHT); // Disable interrupt externally
    GPIO_disableInt(MIC3R_D_OUT_INTRPT);

    GPIO_write(LED_GREEN_2, 1);
//    openADCBuf(); // This is taken by the normal mode micLocalization functionality
}

