/*
 * IirFilter.cpp
 *
 *  Created on: Oct 10, 2020
 *      Author: david
 */

#include <dsp/iirFilter.h>
#include <arm_math.h>

IirFilter::IirFilter()
{
}

void IirFilter::InitFilterState(){
    arm_fir_init_f32(&this->S, NUM_TAPS, this->firCoeffs, this->firStateF32, CHUNK_LENGTH);
}

void IirFilter::FilterBuffer(float* buffer, float* outputBuffer)
{
    arm_fir_f32(&this->S, buffer, outputBuffer, CHUNK_LENGTH);
}

