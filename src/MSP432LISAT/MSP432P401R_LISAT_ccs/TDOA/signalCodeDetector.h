/*
 * signalCodeDetector.h
 *
 *  Created on: Nov 12, 2020
 *      Author: david
 */

#ifndef TDOA_SIGNALCODEDETECTOR_H_
#define TDOA_SIGNALCODEDETECTOR_H_

struct valin_tdoa_input {
   int16_t tdoa1;
   int16_t tdoa2;
} TDOAs;

valin_tdoa_input processThreeLongBuffer(q15_t* micBuffer1L, q15_t* micBuffer2M, q15_t* micBuffer3R, uint16_t bufferLength,
                                        int16_t* refSignal, uint16_t refSignalLength);

#endif /* TDOA_SIGNALCODEDETECTOR_H_ */
