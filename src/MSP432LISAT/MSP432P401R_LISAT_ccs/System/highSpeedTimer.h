/*
 * IrSensorsTimer.h
 *
 *  Created on: Nov 12, 2020
 *      Author: Tomas
 */

#include <ti/drivers/Timer.h>

#ifndef SYSTEM_IRSENSORSTIMER_H_
#define SYSTEM_IRSENSORSTIMER_H_

Timer_Handle highSpeedTimer;
Timer_Params highSpeedTimerParams;

void initHighSpeedTimer(void(*callback)());
void startHighSpeedTimer();
void stopHighSpeedTimer();
void setPeriodUsHighSpeedTimer(uint32_t periodUs);
uint32_t getCountsHighSpeedTimer();

#endif /* SYSTEM_IRSENSORSTIMER_H_ */
