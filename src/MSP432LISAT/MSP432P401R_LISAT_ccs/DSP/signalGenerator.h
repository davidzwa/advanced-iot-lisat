/*
 * signalGenerator.h
 *
 *  Created on: Nov 11, 2020
 *      Author: david
 */

#include "common.h"

#ifndef DSP_SIGNALGENERATOR_H_
#define DSP_SIGNALGENERATOR_H_

void generateSignatureChirp(int16_t* audioBuffer, uint16_t length);
void generateSignatureSine(int16_t* preambleRefBuffer, uint16_t period, uint16_t totalLength);

#endif /* DSP_SIGNALGENERATOR_H_ */
