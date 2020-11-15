/*
 * bumpers.cpp
 *
 *  Created on: Oct 31, 2020
 *      Author: Tomas
 */

#include <Robot/irSensors.h>
#include <System/highSpeedTimer.h>
#include <System/kernelSingleTaskClock.h>

bool irCapsCharged = true;
KernelSingleTaskClock* irSensorsTaskClock = new KernelSingleTaskClock();

void changeSensorsIO(bool);
void chargeCapacitors();
void taskPerformIrReading();
void irTimerCallback();

void initIrTaskClock() {
    irSensorsTaskClock->setupClockTask(IRSENSORS_CLOCK_INITIAL_OFFSET, IRSENSORS_CLOCK_PERIOD, taskPerformIrReading);
}

void startIrTaskClock() {
    irSensorsTaskClock->startClockTask();
}

void taskPerformIrReading() {
    GPIO_write(LINE_IR_EVEN_BACKLIGHT, 1);
    GPIO_write(LINE_IR_ODD_BACKLIGHT, 1);
    changeSensorsIO(0);
    chargeCapacitors();
    irCapsCharged = true;
    setPeriodUsHighSpeedTimer(10);
    startHighSpeedTimer();
}

void changeSensorsIO(bool input) {
    GPIO_PinConfig config = input == 1 ? GPIO_CFG_IN_NOPULL : GPIO_CFG_OUT_STD;
    GPIO_setConfig(LINE_IR1_RIGHT, config);
    GPIO_setConfig(LINE_IR2_RIGHT, config);
    GPIO_setConfig(LINE_IR3_RIGHT, config);
    GPIO_setConfig(LINE_IR4_RIGHT, config);
    GPIO_setConfig(LINE_IR5_LEFT, config);
    GPIO_setConfig(LINE_IR6_LEFT, config);
    GPIO_setConfig(LINE_IR7_LEFT, config);
    GPIO_setConfig(LINE_IR8_LEFT, config);
}

void chargeCapacitors() {
    GPIO_write(LINE_IR1_RIGHT, 1);
    GPIO_write(LINE_IR2_RIGHT, 1);
    GPIO_write(LINE_IR3_RIGHT, 1);
    GPIO_write(LINE_IR4_RIGHT, 1);
    GPIO_write(LINE_IR5_LEFT, 1);
    GPIO_write(LINE_IR6_LEFT, 1);
    GPIO_write(LINE_IR7_LEFT, 1);
    GPIO_write(LINE_IR8_LEFT, 1);
}

void irTimerCallback() {
    // Turn on IR LEDs
    if (irCapsCharged) {
        /* Set light sensor pins as output and read white/black value */
        changeSensorsIO(1);
        irCapsCharged = false;
        /* Wait for 1000 us to read capacitor values */
        setPeriodUsHighSpeedTimer(1000);
        startHighSpeedTimer();
    } else {
        int values = GPIO_read(LINE_IR1_RIGHT);
        //GPIO_write(LED_BLUE_2, values);
        // Turn off IR LEDs to save power
        GPIO_write(LINE_IR_EVEN_BACKLIGHT, 0);
        GPIO_write(LINE_IR_ODD_BACKLIGHT, 0);
    }
}

