/*
 * Date  : 25 Jul 2020
 * Author: Amjad Yousef Majid
 * Email : amjad.y.majid@gmail.com
 */

#include <ti/sysbios/knl/Clock.h>
#include "common.h"

void tachoLeftInterrupt(uint_least8_t index);
void tachoRightInterrupt(uint_least8_t index);

void initTachometerInterrupts() {
    // Setup the hooks for the tachometers ISRs
    GPIO_setCallback(TACHO_LEFT, tachoLeftInterrupt);
    GPIO_setCallback(TACHO_RIGHT, tachoRightInterrupt);
}

void tachoLeftInterrupt(uint_least8_t index) {
    uint32_t currentTicks = Clock_getTicks();
}

void tachoRightInterrupt(uint_least8_t index) {
    uint32_t currentTicks = Clock_getTicks();
}
