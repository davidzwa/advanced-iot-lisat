/*
 * speakerControl.cpp
 *
 *  Created on: Nov 10, 2020
 *      Author: s154384
 */

#include "speakerControl.h"

Clock_Handle* singleSpeakerTaskClockHandle;

void taskPressPauseCallback(UArg arg0)
{
    //disableBumperInterrupts();
    GPIO_write(SPEAKER_PAUSE_PIN, 1);
    Clock_stop(*singleSpeakerTaskClockHandle);
    //enableBumperInterrupts();
}

void speakerPressPause() {
    GPIO_write(SPEAKER_PAUSE_PIN, 0);
    Clock_start(*singleSpeakerTaskClockHandle);
}

void speakerPressBackward() {
    GPIO_write(SPEAKER_BACKWARD_PIN, 0);
    //todo: delay by clock
    GPIO_write(SPEAKER_BACKWARD_PIN, 1);
}

void speakerPlaySound() {
    speakerPressBackward();
    //todo: delay by clock
    speakerPressPause();
    //todo: wait for sound to finish and pause speaker afterwards
    //speakerPressPause();
}

void attachSpeakerTaskClockHandle(Clock_Handle* clockHandle) {
    singleSpeakerTaskClockHandle = clockHandle;
    Clock_setFunc(*singleSpeakerTaskClockHandle, taskPressPauseCallback, NULL);
}
