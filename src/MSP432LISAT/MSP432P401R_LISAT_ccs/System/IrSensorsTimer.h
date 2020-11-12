/*
 * IrSensorsTimer.h
 *
 *  Created on: Nov 12, 2020
 *      Author: Tomas
 */

#include <ti/drivers/Timer.h>

#ifndef SYSTEM_IRSENSORSTIMER_H_
#define SYSTEM_IRSENSORSTIMER_H_

Timer_Handle irTimer;
Timer_Params irTimerParams;

void initIrTimer(Timer_CallBackFxn callback);
void irTimerStart();
void irTimerStop();
void irTimerSetPeriodUs(uint32_t periodUs);
uint32_t getTimerCountsIr();

#endif /* SYSTEM_IRSENSORSTIMER_H_ */
