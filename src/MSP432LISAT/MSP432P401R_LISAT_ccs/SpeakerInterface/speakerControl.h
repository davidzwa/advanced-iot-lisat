/*
 * speakerControl.h
 *
 *  Created on: Nov 10, 2020
 *      Author: s154384
 */

#ifndef SPEAKERINTERFACE_SPEAKERCONTROL_H_
#define SPEAKERINTERFACE_SPEAKERCONTROL_H_

#include <System/periodicKernelTask.h>
#include "common.h"

typedef enum {
    IDLE_STATE,
    REWIND_PRESSED,
    REWIND_RELEASED,
    PAUSE_PRESSED,
} SpeakerState;

void initSpeakerTaskClock();
void startSpeakerTaskClock();
void taskPressPauseCallback();
void speakerPressPause();
void speakerPressBackward();
void speakerPlaySound();


#endif /* SPEAKERINTERFACE_SPEAKERCONTROL_H_ */
