/*
 * signalCodeDetector.cpp
 *
 *  Created on: Nov 12, 2020
 *      Author: david
 */
#include <common.h>

#include <TDOA/signalCodeDetector.h>

int16_t processSingleBuffer(q15_t* micBuffer, uint16_t bufferLength,
                            q15_t *chirp, uint32_t chirp_length,
                            uint32_t num_chirps);

valin_tdoa_input processThreeLongBuffer(
        q15_t* micBuffer0, q15_t* micBuffer1, q15_t* micBuffer2, uint16_t bufferLength,
        int16_t* refSignal, uint16_t refSignalLength)
{
    valin_tdoa_input YODAs;

    int16_t arrival_times[NUM_ADC_CHANNELS];
    q15_t* buffers[3] = {micBuffer0, micBuffer1, micBuffer2};

    for(int16_t i = 0; i < NUM_ADC_CHANNELS; i++)
    {
        arrival_times[i] = processSingleBuffer(buffers[i], bufferLength, refSignal, refSignalLength, NUM_CHIRPS);
    }

    YODAs.tdoa1 = arrival_times[1] - arrival_times[0];
    YODAs.tdoa2 = arrival_times[2] - arrival_times[0];
    return YODAs;
}

int16_t processSingleBuffer(q15_t* micBuffer, uint16_t bufferLength, q15_t *chirp, uint32_t chirp_length, uint32_t num_chirps)
{
    uint32_t correlation_buffer_length = bufferLength*2-1;
    q15_t correlation_buffer[correlation_buffer_length];

    q15_t pulse_delays[num_chirps];
    q15_t pulse_values;
    uint32_t pulse_delay;
    uint16_t zeroing_range = 50;

    // correlate chirp with the buffer
    arm_correlate_q15(chirp, chirp_length, micBuffer, bufferLength, correlation_buffer);

    for(int16_t i = 0; i < num_chirps; i++)
    {
        // find highest peak in correlation buffer
        arm_max_q15(correlation_buffer, bufferLength+150, &pulse_values, &pulse_delay);

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
    for(int i=0; i<num_chirps; i++) {
        final_pulse_delays[i] = pulse_delays[i];
    }

    // Determine mean of NUM_CHIRPS
    q15_t mean_delay;
    arm_mean_q15(final_pulse_delays, (uint32_t)num_chirps, &mean_delay);
    return mean_delay;
}
