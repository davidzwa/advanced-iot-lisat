/*
 * signalGenerator.cpp
 *
 *  Created on: Nov 11, 2020
 *      Author: david
 */

#include <limits.h>
#include "DSP/signalGenerator.h"

void sweepChirp(int16_t* buffer, uint16_t length, double f_start, double f_end, double interval);
void sineWave(int16_t* buffer, uint16_t length, uint16_t frequency);

void generateSignatureSine(int16_t* preambleRefBuffer, uint16_t period, uint16_t totalLength)
{
    sineWave(preambleRefBuffer, period, totalLength);
}

void generateSignatureChirp(int16_t* audioBuffer, uint16_t length)
{
    sweepChirp(audioBuffer, length, chirpFrequencyStart, chirpFrequencyEnd, chirpInterval);
}

// https://stackoverflow.com/questions/11199509/sine-wave-that-slowly-ramps-up-frequency-from-f1-to-f2-for-a-given-time
void sweepChirp(int16_t* buffer, uint16_t length, double f_start, double f_end, double interval)
{
    for (uint16_t i = 0; i < length; ++i)
    {
        double delta = i / (float)length;
        double t = interval * delta;
        double phase = 2 * PI * t * (f_start + (f_end - f_start) * delta / 2);
        while (phase > 2 * PI) {
            phase -= 2 * PI; // optional
        }
        buffer[i] = (int16_t) 1000 * sin(phase);
    }
}

void sineWave(q15_t* buffer, uint16_t periodLength, uint16_t generatedLength)
{
    for (uint16_t i = 0; i < generatedLength; ++i)
    {
        int32_t x = SHRT_MAX * (i % periodLength)/periodLength;
        buffer[i] = arm_sin_q15(x);
    }
}
