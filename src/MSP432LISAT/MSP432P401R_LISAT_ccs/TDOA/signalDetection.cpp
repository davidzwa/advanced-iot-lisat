/*
 * SignalDetection.cpp
 *
 *  Created on: Nov 10, 2020
 *      Author: david
 */

#include <TDOA/SignalDetection.h>

bool StupidDetectionBlackBox(int16_t* outputBuffer_filtered, int16_t length, int16_t rms) {
    return (rms > 1000);
}

SignalDetection::SignalDetection()
{
    // TODO Auto-generated constructor stub

}
