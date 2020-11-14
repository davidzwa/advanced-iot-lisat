/*
 * SignalDetection.h
 *
 *  Created on: Nov 10, 2020
 *      Author: david
 */

#include <common.h>

#ifndef TDOA_SIGNALDETECTION_H_
#define TDOA_SIGNALDETECTION_H_

bool StupidDetectionBlackBox(int16_t* outputBuffer_filtered, int16_t length, int16_t rms);

class SignalPreambleDetector
{
public:
    SignalPreambleDetector();
};

#endif /* TDOA_SIGNALDETECTION_H_ */
