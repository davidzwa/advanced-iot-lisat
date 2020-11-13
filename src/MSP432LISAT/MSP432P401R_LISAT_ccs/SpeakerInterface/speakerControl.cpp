/*
 * speakerControl.cpp
 *
 *  Created on: Nov 10, 2020
 *      Author: s154384
 */

#include "speakerControl.h"
#include <System/kernelSingleTaskClock.h>

KernelSingleTaskClock* speakerTaskClock = new KernelSingleTaskClock();

// FSM state for single play sound cycle
SpeakerState state = IDLE_STATE;

void taskSpeakerCallback();

void initSpeakerTaskClock() {
    speakerTaskClock->setupClockTask(SPEAKER_CLOCK_TIMEOUT, SPEAKER_CLOCK_PERIOD_BUTTON, taskSpeakerCallback);
}

void startSpeakerTaskClock() {
    speakerTaskClock->startClockTask();
}

void taskSpeakerCallback()
{
    if (state == REWIND_PRESSED) {
        /* Release backward button and wait for sound to play out */
        GPIO_write(SPEAKER_BACKWARD_PIN, 1);
        state = REWIND_RELEASED;
        speakerTaskClock->stopClockTask();
        speakerTaskClock->setClockTimeout(SPEAKER_CLOCK_PERIOD_SOUND);
        speakerTaskClock->startClockTask();
    }
    else if (state == REWIND_RELEASED) {
        /* Press pause button */
        GPIO_write(SPEAKER_PAUSE_PIN, 0);
        state = PAUSE_PRESSED;
        speakerTaskClock->stopClockTask();
        speakerTaskClock->setClockTimeout(SPEAKER_CLOCK_PERIOD_BUTTON);
        speakerTaskClock->startClockTask();
    }
    else if (state == PAUSE_PRESSED) {
        /* Release pause button, cycle done */
        GPIO_write(SPEAKER_PAUSE_PIN, 1);
        speakerTaskClock->startClockTask();
        state = IDLE_STATE;
    }
    else {
        /* Wrong state */
        GPIO_write(LED_ERROR_2, 1);
    }
}

void taskPressPauseCallback()
{
    GPIO_write(SPEAKER_PAUSE_PIN, 1);
    speakerTaskClock->stopClockTask();
}

void taskPressBackwardCallback()
{
    GPIO_write(SPEAKER_BACKWARD_PIN, 1);
    speakerTaskClock->stopClockTask();
}

void speakerPressPause() {
    /* Set callback function for press pause functionality */
    speakerTaskClock->setClockCallback(taskPressPauseCallback);
    /* Set time before button is 'unpressed' (200ms) */
    speakerTaskClock->setClockTimeout(SPEAKER_CLOCK_PERIOD_BUTTON);
    /* Press button */
    GPIO_write(SPEAKER_PAUSE_PIN, 0);
    /* Wait for button to be unpressed */
    speakerTaskClock->startClockTask();
}

void speakerPressBackward() {
    speakerTaskClock->setClockCallback(taskPressBackwardCallback);
    speakerTaskClock->setClockTimeout(SPEAKER_CLOCK_PERIOD_BUTTON);
    GPIO_write(SPEAKER_BACKWARD_PIN, 0);
    speakerTaskClock->startClockTask();
}

void speakerPlaySound() {
    speakerTaskClock->setClockCallback(taskSpeakerCallback);
    speakerTaskClock->setClockTimeout(SPEAKER_CLOCK_PERIOD_BUTTON);
    state = REWIND_PRESSED;
    GPIO_write(SPEAKER_BACKWARD_PIN, 0);
    speakerTaskClock->startClockTask();
}
