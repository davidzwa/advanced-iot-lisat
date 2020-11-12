/*
 * signalCodeDetector.cpp
 *
 *  Created on: Nov 12, 2020
 *      Author: david
 */
#include <common.h>

#include <TDOA/signalCodeDetector.h>

int16_t processSingleBuffer(q15_t* micBuffer, int16_t bufferLength, q15_t *chirp, uint32_t chip_length, uint32_t num_chirps);

valin_tdoa_input processThreeLongBuffer(q15_t* micBuffer1L, q15_t* micBuffer2M, q15_t* micBuffer3R, int16_t bufferLength)
{
    // Signal parameters
    // Chirp 5 kHz to 15 kHz in 2 ms, sampled at 44kHz
    q15_t chirp[] = {32767, 24588, 3608, -19680, -32371, -26453, -4453, 20684, 32722, 22828, -3186, -27245, -31190, -11002, 18248, 32751, 19465, -11102, -31926, -23133, 8000, 31513, 23169, -9487, -32228, -19594, 15328, 32707, 11251, -24091, -29536, 2814, 31668, 18204, -20311, -30531, 2973, 32274, 13418, -26007, -25041, 15750, 31123, -4927, -32767, -4400, 31722, 11401, -29627, -16030, 27679, 18556, -26609, -19252, 26732, 18204, -28016, -15282, 30065, 10196, -32063, -2709, 32711, -7018, -30314, 17938, 23169, -27709, -10449, 32687, -6549, -28936, 23319, 14524, -32519, 7431, 27096, -27245, -6132, 31913, -20144, -14525, 32766, -15376, -18072, 32687, -14572, -17491};
    int16_t chirp_length = 88;
    int16_t num_chirps = 6;

    valin_tdoa_input TODAs;

    int16_t arrival_times[3];
    q15_t* buffers[3] = {micBuffer1L, micBuffer2M, micBuffer3R};

    for(int16_t i = 0; i < 3; i++)
    {
        arrival_times[i] = processSingleBuffer(buffers[i], bufferLength, chirp, chirp_length, num_chirps);
    }

    TODAs.tdoa1 = arrival_times[2] - arrival_times[1];
    TODAs.tdoa2 = arrival_times[3] - arrival_times[1];

    return TODAs;
}

int16_t processSingleBuffer(q15_t* micBuffer, int16_t bufferLength, q15_t *chirp, uint32_t chip_length, uint32_t num_chirps)
{
    uint32_t correlation_buffer_length =  2* bufferLength -1;
    q15_t correlation_buffer[correlation_buffer_length];
    q15_t pulse_values;
    uint32_t pulse_delay;
    q15_t pulse_delays[num_chirps];
    uint16_t zeroing_range = 50;

    // correlate chirp with the buffer
    arm_correlate_q15(chirp, CHIRP_SAMPLE_COUNT, micBuffer, bufferLength, correlation_buffer);

    for(int16_t i = 0; i < num_chirps; i++)
    {
        // find highest peak in correlation buffer
        arm_max_q15(correlation_buffer, correlation_buffer_length, &pulse_values, &pulse_delay);

        // store time value of highest peak
        pulse_delays[i] = pulse_delay;

        // clear highest peak and surrounding samples
        for(int16_t j = pulse_delay - zeroing_range; j < pulse_delay + zeroing_range; j++)
        {
            if(j < 0) {
                j = 0;
            }
            else if (j >= correlation_buffer_length){
                break;
            }
            else {
                correlation_buffer[j] = 0;
            }
        }
    }

    // compute mean time
    int16_t final_pulse_delays[num_chirps];
    final_pulse_delays[0] = pulse_delays[0];
    final_pulse_delays[1] = pulse_delays[1];
    final_pulse_delays[2] = pulse_delays[2];
    final_pulse_delays[3] = pulse_delays[3];
    final_pulse_delays[4] = pulse_delays[4];
    final_pulse_delays[5] = pulse_delays[5];
    q15_t mean_delay;

    arm_mean_q15(final_pulse_delays, (uint32_t)num_chirps, &mean_delay);
    return mean_delay;
}
