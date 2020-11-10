/*
 * speakerControl.cpp
 *
 *  Created on: Nov 10, 2020
 *      Author: s154384
 */

#include "common.h"

void speakerPressPause() {
    GPIO_write(SPEAKER_PAUSE_PIN, 0);
    sleep(1);
    GPIO_write(SPEAKER_PAUSE_PIN, 1);
}

void speakerPressBackward() {
    GPIO_write(SPEAKER_BACKWARD_PIN, 0);
    usleep(2);
    GPIO_write(SPEAKER_BACKWARD_PIN, 1);
}

void speakerPlaySound() {
    speakerPressBackward();
    usleep(1);
    speakerPressPause();
    //todo: wait for sound to finish and pause speaker afterwards
    //speakerPressPause();
}
