/*
 * bumpers.cpp
 *
 *  Created on: Oct 31, 2020
 *      Author: Tomas
 */

#include <Robot/robot.h>
#include <Robot/bumpers.h>
#include <ti/sysbios/knl/Clock.h>

Robot* bRobot;
long bumper_last_called = 0;

void bumperInterrupt(uint_least8_t index) {
    if (Clock_getTicks() - bumper_last_called < 1000) { // every second
        return;
    }
    switch(index) {
        case BUMPER_0_RIGHT:
            GPIO_toggle(LED_BLUE_0_GPIO);
            break;
        case BUMPER_1_RIGHT:
            GPIO_toggle(LED_BLUE_0_GPIO);
            break;
        case BUMPER_2_RIGHT:
            bRobot->Stop();
            GPIO_toggle(LED_BLUE_0_GPIO);
            break;
        case BUMPER_3_LEFT:
            bRobot->Stop();
            GPIO_toggle(LED_BLUE_0_GPIO);
            break;
        case BUMPER_4_LEFT:
            GPIO_toggle(LED_BLUE_0_GPIO);
            break;
        case BUMPER_5_LEFT:
            GPIO_toggle(LED_BLUE_0_GPIO);
            break;
    }
    bumper_last_called = Clock_getTicks();
}

void enableBumperInterrupts() {
    GPIO_enableInt(BUMPER_0_RIGHT);
    GPIO_enableInt(BUMPER_1_RIGHT);
    GPIO_enableInt(BUMPER_2_RIGHT);
    GPIO_enableInt(BUMPER_3_LEFT);
    GPIO_enableInt(BUMPER_4_LEFT);
    GPIO_enableInt(BUMPER_5_LEFT);
}

void disableBumperInterrupts() {
    GPIO_disableInt(BUMPER_0_RIGHT);
    GPIO_disableInt(BUMPER_1_RIGHT);
    GPIO_disableInt(BUMPER_2_RIGHT);
    GPIO_disableInt(BUMPER_3_LEFT);
    GPIO_disableInt(BUMPER_4_LEFT);
    GPIO_disableInt(BUMPER_5_LEFT);
}

void initBumpers(Robot* pRobot) {
    bRobot = pRobot;
    GPIO_setCallback(BUMPER_0_RIGHT, bumperInterrupt);
    GPIO_setCallback(BUMPER_1_RIGHT, bumperInterrupt);
    GPIO_setCallback(BUMPER_2_RIGHT, bumperInterrupt);
    GPIO_setCallback(BUMPER_3_LEFT, bumperInterrupt);
    GPIO_setCallback(BUMPER_4_LEFT, bumperInterrupt);
    GPIO_setCallback(BUMPER_5_LEFT, bumperInterrupt);

}


