/*
 * bumpers.cpp
 *
 *  Created on: Oct 31, 2020
 *      Author: Tomas
 */

#include <Robot/irSensors.h>
#include <System/highSpeedTimer.h>
#include <System/kernelSingleTaskClock.h>

bool ir_caps_charged = true;
KernelSingleTaskClock* irSensorsTaskClock = new KernelSingleTaskClock();

void changeSensorsIO(bool);
void chargeCapacitors();
void taskPerformIrReading();

void initIrTaskClock() {
    irSensorsTaskClock->setupClockTask(IR_SENSORS_CLOCK_TIMEOUT, IR_SENSORS_CLOCK_PERIOD, taskPerformIrReading);
    //initIrTimer((Timer_CallBackFxn) &irTimerCallback); // do in main timer32 init
}

void startIrTaskClock() {
    irSensorsTaskClock->startClockTask();
}

void taskPerformIrReading() {
    GPIO_write(LINE_IR_EVEN_BACKLIGHT, 1);
    GPIO_write(LINE_IR_ODD_BACKLIGHT, 1);
    changeSensorsIO(0);
    chargeCapacitors();
    ir_caps_charged = true;
    SetPeriodUsHighSpeedTimer(10);
    StartHighSpeedTimer();
    GPIO_toggle(LED_BLUE_2_GPIO);
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
    if (ir_caps_charged) {
        /* Set light sensor pins as output and read white/black value */
        changeSensorsIO(1);
        ir_caps_charged = false;
        /* Wait for 1000 us to read capacitor values */
        SetPeriodUsHighSpeedTimer(1000);
        StartHighSpeedTimer();
    } else {
        int values = GPIO_read(LINE_IR1_RIGHT);
        //GPIO_write(LED_BLUE_2, values);
        // Turn off IR LEDs to save power
        GPIO_write(LINE_IR_EVEN_BACKLIGHT, 0);
        GPIO_write(LINE_IR_ODD_BACKLIGHT, 0);
    }
}





