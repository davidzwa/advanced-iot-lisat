#include "common.h"

#ifndef DSP_IIRFILTER_H_
#define DSP_IIRFILTER_H_

class Filter
{
public:
    Filter();

    float EMA_a_low = EMA_A_LOW;
    float EMA_a_high = EMA_A_HIGH;
    // Initial state of filter
    float EMA_S_low = 0.0f;
    float EMA_S_high = 0.0f;
    int16_t FilterEMA(int16_t value);
    void ResetEMAState(int16_t initialValue);
    void FilterEMABuffer(uint16_t* buffer, int16_t* outputBuffer, uint16_t length);
};

#endif /* DSP_IIRFILTER_H_ */
