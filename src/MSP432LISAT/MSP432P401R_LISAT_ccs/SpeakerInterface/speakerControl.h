/*
 * speakerControl.h
 *
 *  Created on: Nov 10, 2020
 *      Author: s154384
 */

#ifndef SPEAKERINTERFACE_SPEAKERCONTROL_H_
#define SPEAKERINTERFACE_SPEAKERCONTROL_H_

#include "common.h"
#include <System/kernelSingleTaskClock.h>

void taskPressPauseCallback(UArg args);
void attachSpeakerTaskClockHandle(Clock_Handle* );
void speakerPressPause();
void speakerPressBackward();
void speakerPlaySound();


#endif /* SPEAKERINTERFACE_SPEAKERCONTROL_H_ */
