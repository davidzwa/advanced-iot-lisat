/*
 * IrSensorsTimer.h
 *
 *  Created on: Nov 12, 2020
 *      Author: Tomas
 */

#ifndef SYSTEM_IRSENSORSTIMER_H_
#define SYSTEM_IRSENSORSTIMER_H_

#include <ti/drivers/Timer.h>

Timer_Handle IrTimer;
Timer_Params params32;

void initTimerIr();

uint32_t getTimerCountsIr();


#endif /* SYSTEM_IRSENSORSTIMER_H_ */
