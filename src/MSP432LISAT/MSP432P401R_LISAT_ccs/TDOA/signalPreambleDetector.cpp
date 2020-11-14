/*
 * SignalDetection.cpp
 *
 *  Created on: Nov 10, 2020
 *      Author: david
 */

#include <TDOA/signalPreambleDetector.h>

bool StupidDetectionBlackBox(int16_t* outputBuffer_filtered, int16_t length, int16_t rms) {
    return (rms > 1100);
}

SignalPreambleDetector::SignalPreambleDetector()
{
    // TODO Auto-generated constructor stub

}
