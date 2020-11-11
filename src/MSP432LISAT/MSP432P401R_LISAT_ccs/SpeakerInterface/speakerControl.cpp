/*
 * speakerControl.cpp
 *
 *  Created on: Nov 10, 2020
 *      Author: s154384
 */

#include "speakerControl.h"

void speakerPressPause() {
    GPIO_write(SPEAKER_PAUSE_PIN, 0);
    //todo: delay by clock
    GPIO_write(SPEAKER_PAUSE_PIN, 1);
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
