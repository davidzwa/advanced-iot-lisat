/*
 * irSensors.h
 *
 *  Created on: Oct 31, 2020
 *      Author: Tomas
 */

#ifndef ROBOT_IRSENSORS_H_
#define ROBOT_IRSENSORS_H_

#include "common.h"
#include <Robot/robot.h>
#include <System/kernelSingleTaskClock.h>
#include <System/IrSensorsTimer.h>

void initIrSensors(Robot* robot);
void irTimerCallback();
void attachIrSensorsTaskClockHandle(Clock_Handle* clock_handle);

#endif /* ROBOT_IRSENSORS_H_ */
