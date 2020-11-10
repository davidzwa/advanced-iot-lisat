/*
 * Filter.cpp
 *
 *  Created on: Oct 10, 2020
 *      Author: david
 */

#include <arm_math.h>
#include <DSP/filter.h>

Filter::Filter()
{
}

void Filter::InitFilterState(){
    arm_fir_init_q15(&this->S, FILTER_TAP_NUM, filter_taps, this->firStateF32, ADCBUFFERSIZE);
}

void Filter::FilterBuffer(int16_t* buffer, int16_t* outputBuffer)
{
    arm_fir_q15(&this->S, buffer, outputBuffer, ADCBUFFERSIZE);
}

void Filter::ResetEMAState(int16_t initialValue) {
    EMA_a_low = 0.1; //initialization of EMA alpha
    EMA_a_high = 0.8;
    EMA_S_low = initialValue; //initialization of EMA S
    EMA_S_high = initialValue;
}

void Filter::FilterEMABuffer(int16_t* buffer, int16_t* outputFilteredBuffer, uint16_t length) {
    this->ResetEMAState(buffer[0]);
    for(int i=0;i<length;i++) {
        int16_t analogValue = buffer[i];
        outputFilteredBuffer[i] = FilterEMA(analogValue);
    }
}

int16_t Filter::FilterEMA(int16_t analogValue) {
    EMA_S_low = (float)(EMA_a_low * analogValue) + ((1 - EMA_a_low) * EMA_S_low); //run the EMA
    EMA_S_high = (float)(EMA_a_high * analogValue) + ((1 - EMA_a_high) * EMA_S_high);
    return EMA_S_high - EMA_S_low; //find the band-pass
}

