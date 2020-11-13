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

void initHighSpeedTimer(Timer_CallBackFxn callback);
void StartHighSpeedTimer();
void StopHighSpeedTimer();
void SetPeriodUsHighSpeedTimer(uint32_t periodUs);
uint32_t getCountsHighSpeedTimer();

#endif /* SYSTEM_IRSENSORSTIMER_H_ */
