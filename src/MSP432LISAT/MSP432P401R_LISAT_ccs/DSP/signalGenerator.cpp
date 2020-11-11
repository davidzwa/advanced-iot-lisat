/*
 * signalGenerator.cpp
 *
 *  Created on: Nov 11, 2020
 *      Author: david
 */

#include <DSP/signalGenerator.h>

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

void generateSignatureChirp(int16_t *audioBuffer, uint16_t length)
{
    sweepChirp(audioBuffer, length, chirpFrequencyStart, chirpFrequencyEnd, chirpInterval);
}
