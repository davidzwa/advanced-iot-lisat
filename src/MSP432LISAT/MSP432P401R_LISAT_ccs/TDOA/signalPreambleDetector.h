/*
 * SignalDetection.h
 *
 *  Created on: Nov 10, 2020
 *      Author: david
 */

#include <common.h>

#ifndef TDOA_SIGNALDETECTION_H_
#define TDOA_SIGNALDETECTION_H_

q15_t preprocessed_reference_preamble[PREAMBLE_REF_LENGTH];

bool stupidDetectionBlackBox(int16_t* outputBuffer_filtered, int16_t length, int16_t rms);
bool signalPreambleDetector(q15_t* mic_buffer, uint32_t* detection_history);

#endif /* TDOA_SIGNALDETECTION_H_ */
