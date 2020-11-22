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

void Filter::ResetEMAState(int16_t initialValue) {
    // initialization of EMA alpha
    EMA_a_low = 0.1f;
    EMA_a_high = 0.8f;
    // initialization of EMA S
    EMA_S_low = (float)initialValue;
    EMA_S_high = (float)initialValue;
}

void Filter::FilterEMABuffer(uint16_t* buffer, int16_t* outputFilteredBuffer, uint16_t length) {
    int16_t bufferState = buffer[0];
    this->ResetEMAState(bufferState);
    for(uint16_t i=0;i<length;i++) {
        outputFilteredBuffer[i] = FilterEMA(buffer[i]);
    }
}

int16_t Filter::FilterEMA(int16_t analogValue) {
    EMA_S_low = (EMA_a_low * analogValue) + ((1 - EMA_a_low) * EMA_S_low); //run the EMA
    EMA_S_high = (EMA_a_high * analogValue) + ((1 - EMA_a_high) * EMA_S_high);
    return (int16_t)(EMA_S_high - EMA_S_low); //find the band-pass
}

