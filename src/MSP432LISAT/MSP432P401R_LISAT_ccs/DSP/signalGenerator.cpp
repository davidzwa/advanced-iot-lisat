/*
 * signalGenerator.cpp
 *
 *  Created on: Nov 11, 2020
 *      Author: david
 */

#include <limits.h>
#include "DSP/signalGenerator.h"

void sweepChirp(int16_t* buffer, uint16_t length, double f_start, double f_end, double f_samp, double duration_ms);
void sineWave(int16_t* buffer, uint16_t length, uint16_t frequency);

void generateSignatureSine(int16_t* preambleRefBuffer, uint16_t period, uint16_t totalLength)
{
    sineWave(preambleRefBuffer, period, totalLength);
}

void generateSignatureChirp(int16_t* audioBuffer, uint16_t length)
{
    sweepChirp(audioBuffer, length, (double)CHIRP_FREQ_START_KHZ, (double)CHIRP_FREQ_END_KHZ, (double)TARGET_FREQUENCY, (double)CHIRP_LENGTH);
}

// https://stackoverflow.com/questions/11199509/sine-wave-that-slowly-ramps-up-frequency-from-f1-to-f2-for-a-given-time
void sweepChirp(int16_t* buffer, uint16_t length, double f1, double f2, double f_samp, double duration)
{
    for (uint16_t i = 0; i < length; ++i)
    {
        double t = i/f_samp;
        double angle = 2*PI*(f1*t + (f2-f1)*(t*t)/(2*duration));
        buffer[i] = (int16_t)(SHRT_MAX * sin(angle)) >> CHIRP_SHIFT;
    }
}

void sineWave(q15_t* buffer, uint16_t periodLength, uint16_t generatedLength)
{
    for (uint16_t i = 0; i < generatedLength; ++i)
    {
        int32_t x = SHRT_MAX * (i % periodLength)/periodLength;
        buffer[i] = arm_sin_q15(x) >> CARRIER_SHIFT;
    }
}
