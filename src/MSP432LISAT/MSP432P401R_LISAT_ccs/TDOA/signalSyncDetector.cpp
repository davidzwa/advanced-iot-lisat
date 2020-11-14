/*
 * SignalDetection.cpp
 *
 *  Created on: Nov 10, 2020
 *      Author: david
 */

#include <TDOA/signalSyncDetector.h>

bool StupidDetectionBlackBox(int16_t* outputBuffer_filtered, int16_t length, int16_t rms) {
    return (rms > 1100);
}

SignalSyncDetector::SignalSyncDetector()
{
    // TODO Auto-generated constructor stub

}
