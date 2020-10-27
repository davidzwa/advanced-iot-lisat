/*
 * Date  : 25 Jul 2020
 * Author: Amjad Yousef Majid
 * Email : amjad.y.majid@gmail.com
 */

#include <ti/sysbios/knl/Clock.h>
#include <ti/drivers/Timer.h>
#include "common.h"
#include "TDOA/freeRunningTimer.h"

uint32_t lastTicksLeft = 0;
uint32_t lastTicksRight = 0;
uint32_t currentTicksDiffLeft = 0;
uint32_t currentTicksDiffRight = 0;

#define DROP_SAMPLE_ABOVE 30000

void tachoLeftInterrupt(uint_least8_t index);
void tachoRightInterrupt(uint_least8_t index);

void initTachometerInterrupts() {
    initTimerTacho();
    startTimerTacho();

    // Setup the hooks for the tachometers ISRs
    GPIO_setCallback(TACHO_LEFT, tachoLeftInterrupt);
    GPIO_setCallback(TACHO_RIGHT, tachoRightInterrupt);

    lastTicksLeft = getTimerUsTacho(); // Clock_getTicks();
    lastTicksRight = getTimerUsTacho(); // Clock_getTicks();

    GPIO_enableInt(TACHO_LEFT);
    GPIO_enableInt(TACHO_RIGHT);
}

void tachoLeftInterrupt(uint_least8_t index) {
    uint32_t clockValue = getTimerUsTacho();
    if (clockValue < lastTicksLeft) {
        currentTicksDiffLeft = clockValue + (0xFFFF - lastTicksLeft);
    }
    else {
        currentTicksDiffLeft = clockValue - lastTicksLeft;
    }
    lastTicksLeft = clockValue;
}

void tachoRightInterrupt(uint_least8_t index) {
    uint32_t clockValue = getTimerUsTacho();
    if (clockValue < lastTicksRight) {
        currentTicksDiffRight = clockValue + (0xFFFF - lastTicksRight);
    }
    else {
        currentTicksDiffRight = clockValue - lastTicksRight;
    }
    lastTicksRight = clockValue;
}
