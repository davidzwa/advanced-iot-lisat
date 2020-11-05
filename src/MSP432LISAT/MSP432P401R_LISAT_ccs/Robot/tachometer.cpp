/*
 * Base version: tachometer.c
 * Date  : 25 Jul 2020
 * Author: Amjad Yousef Majid
 * Email : amjad.y.majid@gmail.com
 *
 * New version: tachometer.cpp
 * Date  : 27 Oct 2020
 * Author: David J. Zwart
 * Email : davidzwa@gmail.com
 */

//#include <ti/sysbios/knl/Clock.h>
#include <ti/drivers/Timer.h>
#include "common.h"
#include "System/freeRunningTimer.h"

uint32_t lastTicksLeft = 0;
uint32_t lastTicksRight = 0;
uint32_t currentTicksDiffLeft = 0;
uint32_t currentTicksDiffRight = 0;

uint32_t numCalibrationInterruptsLeft = 0;
uint32_t numCalibrationInterruptsRight = 0;
uint32_t sumCalibrationTicksLeft = 0;
uint32_t sumCalibrationTicksRight = 0;
bool measureDistanceTachometers = false;

void tachoLeftInterrupt(uint_least8_t index);
void tachoRightInterrupt(uint_least8_t index);
uint32_t timerDifference(uint32_t lastTicks, uint32_t clockValue);

void resetTachometerCountValues() {
    numCalibrationInterruptsLeft = 0;
    numCalibrationInterruptsRight = 0;
    sumCalibrationTicksLeft = 0;
    sumCalibrationTicksRight = 0;
    measureDistanceTachometers = true;
}

void disableCalibrationTachometers() {
    measureDistanceTachometers = false;
}

// This function only makes sense when measureDistanceTachometers == true
uint32_t getInterruptCountLeft() {
    return numCalibrationInterruptsLeft;
}

// This function only makes sense when measureDistanceTachometers == true
uint32_t getInterruptCountRight() {
    return numCalibrationInterruptsRight;
}

float calculateTicksPerInterruptLeft() {
    if (measureDistanceTachometers == true && sumCalibrationTicksLeft > 0) {
        return (float)sumCalibrationTicksLeft/numCalibrationInterruptsLeft;
    }
    else {
        return 0.0f;
    }
}

float calculateTicksPerInterruptRight() {
    if (measureDistanceTachometers == true && sumCalibrationTicksRight > 0) {
        return (float)sumCalibrationTicksRight/numCalibrationInterruptsRight;
    }
    else {
        return 0.0f;
    }
}

void initTachometers() {
    // Setup the hooks for the tachometers ISRs
    GPIO_setCallback(TACHO_LEFT_B, tachoLeftInterrupt);
    GPIO_setCallback(TACHO_RIGHT_B, tachoRightInterrupt);
}

void enableTachometerInterrupts() {
    initTimerTacho();
    startTimerTacho();

    lastTicksLeft = getTimerUsTacho();
    lastTicksRight = getTimerUsTacho();

    GPIO_enableInt(TACHO_LEFT_B);
    GPIO_enableInt(TACHO_RIGHT_B);
}

void disableTachometerInterrupts() {
    stopTimerTacho();
    GPIO_disableInt(TACHO_LEFT_B);
    GPIO_disableInt(TACHO_RIGHT_B);
}

void tachoLeftInterrupt(uint_least8_t index) {
    uint32_t clockValue = getTimerUsTacho();
    currentTicksDiffLeft = timerDifference(lastTicksLeft, clockValue);
    if (measureDistanceTachometers) {
        sumCalibrationTicksLeft += currentTicksDiffLeft;
        numCalibrationInterruptsLeft++;
    }
    lastTicksLeft = clockValue;
}

void tachoRightInterrupt(uint_least8_t index) {
    uint32_t clockValue = getTimerUsTacho();
    currentTicksDiffRight = timerDifference(lastTicksRight, clockValue);
    if (measureDistanceTachometers) {
        sumCalibrationTicksRight += currentTicksDiffRight;
        numCalibrationInterruptsRight++;
    }
    lastTicksRight = clockValue;
}

uint32_t timerDifference(uint32_t lastTicks, uint32_t clockValue) {
    if (clockValue < lastTicks) {
        return clockValue + (0xFFFF - lastTicks);
    }
    else {
        return clockValue - lastTicks;
    }
}
