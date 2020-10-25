/*
 * Date  : 25 Jul 2020
 * Author: Amjad Yousef Majid
 * Email : amjad.y.majid@gmail.com
 */

#include <ti/sysbios/knl/Clock.h>
#include "common.h"

uint32_t lastTicksLeft = 0;
uint32_t lastTicksRight = 0;
uint32_t currentTicksDiffLeft = 0;
uint32_t currentTicksDiffRight = 0;

#define DROP_SAMPLE_ABOVE 100

void tachoLeftInterrupt(uint_least8_t index);
void tachoRightInterrupt(uint_least8_t index);

void initTachometerInterrupts() {
    // Setup the hooks for the tachometers ISRs
    GPIO_setCallback(TACHO_LEFT, tachoLeftInterrupt);
    GPIO_setCallback(TACHO_RIGHT, tachoRightInterrupt);

    lastTicksLeft = Clock_getTicks();
    lastTicksRight = Clock_getTicks();

    GPIO_enableInt(TACHO_LEFT);
    GPIO_enableInt(TACHO_RIGHT);
}

void tachoLeftInterrupt(uint_least8_t index) {
    uint32_t clockValue = Clock_getTicks();
    if (clockValue - lastTicksLeft < DROP_SAMPLE_ABOVE) {
        currentTicksDiffLeft = clockValue - lastTicksLeft;
    }
    else {
        currentTicksDiffLeft = 0;
    }
    lastTicksLeft = clockValue;
}

void tachoRightInterrupt(uint_least8_t index) {
    uint32_t clockValue = Clock_getTicks();
    if (clockValue - lastTicksRight < DROP_SAMPLE_ABOVE) {
        currentTicksDiffRight = clockValue - lastTicksRight;
    } else {
        currentTicksDiffRight = 0;
    }
    lastTicksRight = clockValue;
}
