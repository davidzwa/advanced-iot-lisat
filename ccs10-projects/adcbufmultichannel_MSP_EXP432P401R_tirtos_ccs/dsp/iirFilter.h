#include "common.h"
#include <dsp/filterDesign.h>
#ifndef DSP_IIRFILTER_H_
#define DSP_IIRFILTER_H_

class IirFilter
{
public:
//    q15_t firCoeffs32LP[NUM_TAPS] = {272, 449, -266, -540, -55, -227, -1029, 745, 3927, 1699, -5616, -6594, 2766, 9228, 2766, -6594, -5616, 1699, 3927, 745, -1029, -227, -55, -540, -266, 449};
//    q15_t firCoefss32LP[NUM_TAPS] = {
//                                     1856,-1899, -710, -967, 1449, 462, 3040, -1183,
//                                     -144,-7217, -5492, 21835, -5492, -7217, -144,
//                                     -1183, 3040, 462, 1449, -967, -710, -1899, 1856
//    };

    arm_fir_instance_q15 S;
    q15_t firStateF32[CHUNK_LENGTH + FILTER_TAP_NUM -1];

    float EMA_a_low = 0.6; //initialization of EMA alpha
    float EMA_a_high = 0.9;
    int EMA_S_low = 0; //initialization of EMA S
    int EMA_S_high = 0;
    int16_t FilterEMA(int16_t value);
    void ResetEMAState();
    void FilterEMABuffer(int16_t* buffer, int16_t* outputBuffer);

//    arm_fir_instance_f32 S;
//    float32_t firStateF32[CHUNK_LENGTH + NUM_TAPS - 1];
//    float32_t firCoeffs[NUM_TAPS] = {
//            -0.0018225230f, -0.0015879294f, +0.0000000000f, +0.0036977508f, +0.0080754303f,
//            +0.0085302217f, -0.0000000000f, -0.0173976984f, -0.0341458607f, -0.0333591565f,
//            +0.0000000000f, +0.0676308395f, +0.1522061835f, +0.2229246956f, +0.2504960933f,
//            +0.2229246956f, +0.1522061835f, +0.0676308395f, +0.0000000000f, -0.0333591565f,
//            -0.0341458607f, -0.0173976984f, -0.0000000000f, +0.0085302217f, +0.0080754303f,
//            +0.0036977508f, +0.0000000000f, -0.0015879294f, -0.0018225230f
//    };

    IirFilter();
    void InitFilterState();
    void FilterBuffer(int16_t* buffer, int16_t* outputBuffer);
};

#endif /* DSP_IIRFILTER_H_ */
