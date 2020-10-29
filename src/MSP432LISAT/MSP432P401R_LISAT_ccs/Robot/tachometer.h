/**
 * Date  : 25 Jul 2020
 * Author: Amjad Yousef Majid
 * Email : amjad.y.majid@gmail.com
 */

#ifndef SYSTEM_INCLUDE_TACHOMETER_H_
#define SYSTEM_INCLUDE_TACHOMETER_H_

uint32_t getInterruptCountLeft();
uint32_t getInterruptCountRight();
void resetTachometerCountValues();

void disableCalibrationTachometers();
float calculateTicksPerInterruptLeft();
float calculateTicksPerInterruptRight();

void initTachometers();
void enableTachometerInterrupts();
void disableTachometerInterrupts();

#endif /* SYSTEM_INCLUDE_TACHOMETER_H_ */
