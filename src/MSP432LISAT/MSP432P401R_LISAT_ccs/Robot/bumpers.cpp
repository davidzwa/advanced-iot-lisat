/*
 * bumpers.cpp
 *
 *  Created on: Oct 31, 2020
 *      Author: Tomas
 */

#include <Robot/robot.h>
#include <Robot/bumpers.h>

void bumper0RightInterrupt(uint_least8_t index) {
    GPIO_write(LED_ERROR_2, 0);
}
void bumper1RightInterrupt(uint_least8_t index) {
    GPIO_write(LED_ERROR_2, 1);
}
void bumper2RightInterrupt(uint_least8_t index) {
    GPIO_toggle(LED_ERROR_2);
}
void bumper3LeftInterrupt(uint_least8_t index) {
    GPIO_toggle(LED_ERROR_2);
}
void bumper4LeftInterrupt(uint_least8_t index) {
    GPIO_toggle(LED_ERROR_2);
}
void bumper5LeftInterrupt(uint_least8_t index) {
    GPIO_toggle(LED_ERROR_2);
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

void initBumpers() {
    GPIO_setCallback(BUMPER_0_RIGHT, bumper0RightInterrupt);
    GPIO_setCallback(BUMPER_1_RIGHT, bumper1RightInterrupt);
    GPIO_setCallback(BUMPER_2_RIGHT, bumper2RightInterrupt);
    GPIO_setCallback(BUMPER_3_LEFT, bumper3LeftInterrupt);
    GPIO_setCallback(BUMPER_4_LEFT, bumper4LeftInterrupt);
    GPIO_setCallback(BUMPER_5_LEFT, bumper5LeftInterrupt);

}


