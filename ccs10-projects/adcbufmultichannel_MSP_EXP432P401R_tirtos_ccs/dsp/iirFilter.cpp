/*
 * IirFilter.cpp
 *
 *  Created on: Oct 10, 2020
 *      Author: david
 */

#include <arm_math.h>
#include <dsp/iirFilter.h>

IirFilter::IirFilter()
{
}

void IirFilter::InitFilterState(){
    arm_fir_init_q15(&this->S, FILTER_TAP_NUM, filter_taps, this->firStateF32, CHUNK_LENGTH);
}

void IirFilter::FilterBuffer(int16_t* buffer, int16_t* outputBuffer)
{
    arm_fir_q15(&this->S, buffer, outputBuffer, CHUNK_LENGTH);
}

void IirFilter::ResetEMAState() {
    EMA_a_low = 0.1; //initialization of EMA alpha
    EMA_a_high = 0.8;
    EMA_S_low = 4000; //initialization of EMA S
    EMA_S_high = 4000;
}

void IirFilter::FilterEMABuffer(int16_t* buffer, int16_t* outputBuffer) {
    this->ResetEMAState();
    for(int i=0;i<CHUNK_LENGTH;i++) {
        outputBuffer[i] = FilterEMA(buffer[i]);
    }
}

int16_t IirFilter::FilterEMA(int16_t analogValue) {
    EMA_S_low = (EMA_a_low * analogValue) + ((1 - EMA_a_low) * EMA_S_low); //run the EMA
    EMA_S_high = (EMA_a_high * analogValue) + ((1 - EMA_a_high) * EMA_S_high);
    return EMA_S_high - EMA_S_low; //find the band-pass
}

