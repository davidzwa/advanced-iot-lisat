/*
 * fft.cpp
 *
 *  Created on: Oct 10, 2020
 *      Author: david
 */

#include <Math/fft.h>

FFT::FFT()
{

}

uint32_t fftSize = CHUNK_LENGTH;
uint32_t ifftFlag = 0;
uint32_t doBitReverse = 1;
volatile arm_status status;
int16_t fftComplex[CHUNK_LENGTH*2];

void doFFT(int16_t* buffer, int16_t* bufferOutput){
    arm_rfft_instance_q15 instance;
    status = arm_rfft_init_q15(&instance, fftSize, ifftFlag, doBitReverse);

    arm_rfft_q15(&instance, buffer, fftComplex);

    for(int i = 0; i < fftSize*2; i += 2)
    {
        bufferOutput[i /
                    2] =
            (int32_t)(sqrtf((fftComplex[i] *
                    fftComplex[i]) +
                            (fftComplex[i + 1] * fftComplex[i + 1])));
    }
}

