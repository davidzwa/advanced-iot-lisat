/*
 * irSensors.cpp
 *
 *  Created on: Oct 31, 2020
 *      Author: Tomas
 */

#include <Robot/irSensors.h>
#include <System/highSpeedTimer.h>
#include <System/periodicKernelTask.h>

bool irCapsCharged = true;
bool lineDetectionDebounce = false;
PeriodicKernelTask* irSensorsTaskClock = new PeriodicKernelTask();
uint8_t irReadingSum = 0;
uint8_t irReadingByte = 0b00000000;
sem_t lineDetectionSem;
Robot* irRobot;

void changeSensorsIO(bool);
void chargeCapacitors();
void taskPerformIrReading();
void irTimerCallback();

void initIrTaskClock(Robot* pRobot) {
    irRobot = pRobot;
    irSensorsTaskClock->setupClockTask(IRSENSORS_CLOCK_INITIAL_OFFSET, IRSENSORS_CLOCK_PERIOD, taskPerformIrReading);
}

void initLineDetectionSem() {
    sem_init(&lineDetectionSem, 0, 0);
}

void startIrTaskClock() {
    irSensorsTaskClock->startClockTask();

}

void stopIrTaskClock() {
    irSensorsTaskClock->stopClockTask();
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
    if (lineDetectionDebounce) {
        lineDetectionDebounce = false;
        stopIrTaskClock();
        irSensorsTaskClock->setClockTimeout(IRSENSORS_CLOCK_PERIOD);
        startIrTaskClock();
        return;
    }
    // Turn on IR LEDs
    if (irCapsCharged) {
        /* Set light sensor pins as output and read white/black value */
        changeSensorsIO(1);
        irCapsCharged = false;
        /* Wait for 1000 us to read capacitor values */
        setPeriodUsHighSpeedTimer(1000);
        startHighSpeedTimer();
    } else {
        /* Read values of reflectance sensors */
        irReadingSum = 0;
//        irReadingByte = 0b00000000;
        irReadingSum += GPIO_read(LINE_IR1_RIGHT);
        irReadingSum += GPIO_read(LINE_IR2_RIGHT);
        irReadingSum += GPIO_read(LINE_IR3_RIGHT);
        irReadingSum += GPIO_read(LINE_IR4_RIGHT);
        irReadingSum += GPIO_read(LINE_IR5_LEFT);
        irReadingSum += GPIO_read(LINE_IR6_LEFT);
        irReadingSum += GPIO_read(LINE_IR7_LEFT);
        irReadingSum += GPIO_read(LINE_IR8_LEFT);
//        irReadingByte |= GPIO_read(LINE_IR1_RIGHT);
//        irReadingByte |= GPIO_read(LINE_IR2_RIGHT) << 1;
//        irReadingByte |= GPIO_read(LINE_IR3_RIGHT) << 2;
//        irReadingByte |= GPIO_read(LINE_IR4_RIGHT) << 3;
//        irReadingByte |= GPIO_read(LINE_IR5_LEFT)  << 4;
//        irReadingByte |= GPIO_read(LINE_IR6_LEFT)  << 5;
//        irReadingByte |= GPIO_read(LINE_IR7_LEFT)  << 6;
//        irReadingByte |= GPIO_read(LINE_IR8_LEFT)  << 7;
        if (irReadingSum >= LINE_DETECTION_THRESHOLD && listenToLineDetection) {
            /* STOPLINE has been detected */
            listenToLineDetection = false;
            sem_post(&lineDetectionSem);

        }
//        else if (irReadingByte >= 96) {
//            /* TRACKLINE is off to the right */
//            //irRobot->adjustRobotAngleThetaOffset(1);
//        }
//        else if (irReadingByte <= 6) {
//            /* TRACKLINE is off to the left */
//            //irRobot->adjustRobotAngleThetaOffset(-1);
//        }
        /* TRACKLINE is in the middle */

        // Turn off IR LEDs to save power
        GPIO_write(LINE_IR_EVEN_BACKLIGHT, 0);
        GPIO_write(LINE_IR_ODD_BACKLIGHT, 0);
    }
}

void resetLineDetection() {
    lineDetectionDebounce = true;
    irReadingSum = 0;
    irSensorsTaskClock->setClockTimeout(LINE_DETECTION_DEBOUNCE);
}

