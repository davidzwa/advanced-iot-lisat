/*
 * irSensors.h
 *
 *  Created on: Oct 31, 2020
 *      Author: Tomas
 */

#ifndef ROBOT_IRSENSORS_H_
#define ROBOT_IRSENSORS_H_

#include "common.h"

void initLineDetectionSem();
void initIrTaskClock();
void startIrTaskClock();
void irTimerCallback();
bool checkStoplineDetected();
void resetLineDetection();

#endif /* ROBOT_IRSENSORS_H_ */
