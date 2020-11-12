/*
 * bumpers.cpp
 *
 *  Created on: Oct 31, 2020
 *      Author: Tomas
 */

#include <Robot/robot.h>
#include <Robot/irSensors.h>
#include <ti/sysbios/knl/Clock.h>
#include <System/IrSensorsTimer.h>

Robot* irRobot;

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

bool ir_caps_charged = true;

void irPerformReading() {
    GPIO_write(LINE_IR_EVEN_BACKLIGHT, 1);
    GPIO_write(LINE_IR_ODD_BACKLIGHT, 1);
    changeSensorsIO(0);
    chargeCapacitors();
    ir_caps_charged = true;
    irTimerSetPeriodUs(10);
    irTimerStart();
}

void irTimerCallback() {
    // Turn on IR LEDs
    if (ir_caps_charged) {
        /* Set light sensor pins as output and read white/black value */
        changeSensorsIO(1);
        ir_caps_charged = false;
        /* Wait for 1000 us to read capacitor values */
        irTimerSetPeriodUs(1000);
        irTimerStart();
    } else {
        int values = GPIO_read(LINE_IR1_RIGHT);
        GPIO_write(LED_BLUE_2, values);
        // Turn off IR LEDs to save power
        GPIO_write(LINE_IR_EVEN_BACKLIGHT, 0);
        GPIO_write(LINE_IR_ODD_BACKLIGHT, 0);
    }
}

void initIrSensors(Robot* pRobot) {
    irRobot = pRobot;
    initIrTimer( (Timer_CallBackFxn) &irTimerCallback);
    irPerformReading();
}




