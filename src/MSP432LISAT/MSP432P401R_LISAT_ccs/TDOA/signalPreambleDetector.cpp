/*
 * SignalDetection.cpp
 *
 *  Created on: Nov 10, 2020
 *      Author: david
 */

#include <TDOA/signalPreambleDetector.h>
q15_t correlation_output;
q15_t match_score = 0;
q15_t absolute_correlation_output;
q15_t preambleMatch(q15_t* mic_buffer);
uint16_t hammingWeight(uint16_t N);

bool stupidDetectionBlackBox(int16_t* outputBuffer_filtered, int16_t length, int16_t rms) {
    return (rms > 1100);
}

bool signalPreambleDetector(q15_t* mic_buffer, uint32_t* detection_history)
{
    q15_t match_score;
    uint16_t match_success;
    uint16_t succesfull_detections;

    match_score = preambleMatch(mic_buffer);
    *detection_history = *detection_history >> 1; // shift history

    match_success = match_score >= MATCH_THRESHOLD;
    *detection_history = *detection_history | (match_success << HISTORY_LENGTH);

    succesfull_detections = hammingWeight(*detection_history);
    return (succesfull_detections >= DETECTION_THRESHOLD);
}

// Function trying to match the buffer with a prepared reference preamble (a smart preparation of known length PREAMBLE_REF_LENGTH)
q15_t preambleMatch(q15_t* mic_buffer)
{
    q15_t multiplication_array[ADCBUFFERSIZE_SHORT];
    absolute_correlation_output = 0;
    correlation_output = 0;
    match_score = 0;
    for(int16_t i = 0; i < (PREAMBLE_SINE_PERIOD/2); i++)
    {
        // (A, B, output, length)
        arm_mult_q15(mic_buffer, preprocessed_reference_preamble + i, multiplication_array, ADCBUFFERSIZE_SHORT);
        for(int16_t j = 0; j < ADCBUFFERSIZE_SHORT; j++)
        {
            arm_add_q15(&correlation_output, multiplication_array + j, &correlation_output, 1);
        }
        arm_abs_q15(&correlation_output, &absolute_correlation_output, 1);

        // Optimally calculate what we need to know
        if (absolute_correlation_output > match_score)
        {
            match_score = absolute_correlation_output;
        }
    }
    return match_score;
}

uint16_t hammingWeight(uint16_t N)
{
    uint16_t result = 0;
    while(N)
    {
        result++;
        N &=N-1;
    }
    return result;
}
