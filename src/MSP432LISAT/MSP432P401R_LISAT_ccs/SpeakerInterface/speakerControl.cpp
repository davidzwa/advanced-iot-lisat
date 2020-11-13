/*
 * speakerControl.cpp
 *
 *  Created on: Nov 10, 2020
 *      Author: s154384
 */

#include "speakerControl.h"

Clock_Handle* speakerClockHandle;

// FSM state for single play sound cycle
int speaker_state = 0;

void taskSpeakerCallback(UArg arg0)
{
    if (speaker_state == 0) {
        /* Release backward button and wait for sound to play out */
        GPIO_write(SPEAKER_BACKWARD_PIN, 1);
        speaker_state = 1;
        Clock_stop(*speakerClockHandle);
        Clock_setTimeout(*speakerClockHandle, SOUND_PLAY_DURATION);
        Clock_start(*speakerClockHandle);
    }
    else if (speaker_state == 1) {
        /* Press pause button */
        GPIO_write(SPEAKER_PAUSE_PIN, 0);
        speaker_state = 2;
        Clock_stop(*speakerClockHandle);
        Clock_setTimeout(*speakerClockHandle, 200);
        Clock_start(*speakerClockHandle);
    }
    else if (speaker_state == 2) {
        /* Release pause button, cycle done */
        GPIO_write(SPEAKER_PAUSE_PIN, 1);
        Clock_stop(*speakerClockHandle);
        speaker_state = 0;
    }
}

void taskPressPauseCallback(UArg arg0)
{
    GPIO_write(SPEAKER_PAUSE_PIN, 1);
    Clock_stop(*speakerClockHandle);
}

void taskPressBackwardCallback(UArg arg0)
{
    GPIO_write(SPEAKER_BACKWARD_PIN, 1);
    Clock_stop(*speakerClockHandle);
}

void speakerPressPause() {
    /* Set callback function for press pause functionality */
    Clock_setFunc(*speakerClockHandle, taskPressPauseCallback, NULL);
    /* Set time before button is 'unpressed' (200ms) */
    Clock_setTimeout(*speakerClockHandle, 200);
    /* Press button */
    GPIO_write(SPEAKER_PAUSE_PIN, 0);
    /* Wait for button to be unpressed */
    Clock_start(*speakerClockHandle);
}

void speakerPressBackward() {
    Clock_setFunc(*speakerClockHandle, taskPressBackwardCallback, NULL);
    Clock_setTimeout(*speakerClockHandle, 200);
    GPIO_write(SPEAKER_BACKWARD_PIN, 0);
    Clock_start(*speakerClockHandle);
}

void speakerPlaySound() {
    Clock_setFunc(*speakerClockHandle, taskSpeakerCallback, NULL);
    Clock_setTimeout(*speakerClockHandle, 200);
    speaker_state = 0;
    GPIO_write(SPEAKER_BACKWARD_PIN, 0);
    Clock_start(*speakerClockHandle);
}

void attachSpeakerTaskClockHandle(Clock_Handle* clockHandle) {
    speakerClockHandle = clockHandle;
    Clock_setFunc(*speakerClockHandle, taskSpeakerCallback, NULL);
}
