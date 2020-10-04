///*
// * signal_filter.c
// *
// *  Created on: Sep 27, 2020
// *      Author: david
// */
//
//#include "signal_filter.h"
//
//bool calculate_lpf(q15_t* inputQ15, int16_t length, q15_t* outputQ15) {
//    /* Initialize input and output buffer pointers */
////    inputQ15 = &testInput_q15_1kHz_15kHz[0];
////    outputQ15 = &testOutput[0];
//
//    /* Call FIR init function to initialize the instance structure. */
//    arm_fir_init_q15(&S, NUM_TAPS, &firCoeffsQ15[0], &firStateQ15[0], blockSize);
//
//    /* ----------------------------------------------------------------------
//    ** Call the FIR process function for every blockSize samples
//    ** ------------------------------------------------------------------- */
//
//    for(i=0; i < numBlocks; i++)
//    {
//      arm_fir_f32(&S, inputQ15 + (i * blockSize), inputQ15 + (i * blockSize), blockSize);
//    }
//}
