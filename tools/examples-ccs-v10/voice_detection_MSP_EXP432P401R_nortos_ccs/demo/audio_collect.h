/* --COPYRIGHT--,BSD
 * Copyright (c) 2017, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * --/COPYRIGHT--*/

/** \file audio_collect.h Contains Audio API declaration for setting up a
 *  ping-pong buffer for audio data, starting and stopping audio data
 *  collection, and the interrupt handler for the DMA interrupt that
 *  declares a buffer of data is available for
 *  processing.\n (C) Copyright 2015, Texas Instruments, Inc.
 *
 * The user must allocate two int_least16_t buffers of size bufferSize to
 * hold the ping-pong data. The user must allocate the memory for
 * the DMA Control Table on the MSP432. All peripherals use SMCLK, which
 * must be a multiple of the audio sample rate, preferably and even multiple.
 *
 * The code implements a DMA channel 1 interrupt isr function "dma_1_isr"
 * which must be set in the interrupt vector code.
 *
 * Resources used on MSP432 are:
 * ADC14
 * DMA channel 7
 * DMA Interrupt 1
 * Timer A0
 * SMCLK
 * GPIO 3.7 preamp power
 * GPIO 4.4 A9 ADC14 input
 */

#ifndef AUDIO_COLLECT_H_
#define AUDIO_COLLECT_H_

/*----------------------------------------------------------------------------
 * Includes
 * -------------------------------------------------------------------------*/
#include <stdint.h>

/*----------------------------------------------------------------------------
 * Defines and Typedefs
 * -------------------------------------------------------------------------*/
typedef struct Audio_Struct *Audio_Handle;

/*----------------------------------------------------------------------------
 * Functions
 * --------------------------------------------------------------------------*/
/* Set up the device to collect audio samples in ping-pong buffers */
Audio_Handle Audio_setupCollect(uint32_t *dmaControlTable,
    int_least16_t *audioBuffer1, int_least16_t *audioBuffer2,
    uint_least32_t bufferSize, uint_least32_t sampleRate);

/* Start collecting audio samples in ping-pong buffers */
void Audio_startCollect(Audio_Handle audio);

/* Get pointer to the active buffer with valid data ready for processing */
int_least16_t *Audio_getActiveBuffer(Audio_Handle audio);

/* Check if a frame of data is ready for processing */
bool Audio_getActive(Audio_Handle audio);

/* Indicate done with processing active buffer holding valid data */
void Audio_resetActive(Audio_Handle audio);

/* Get overflow error status */
bool Audio_getOverflow(Audio_Handle audio);

/* Reset overflow error status */
void Audio_resetOverflow(Audio_Handle audio);

/* Stop collecting audio samples in buffers */
void Audio_stopCollect(Audio_Handle audio);

/* Shut down the audio collection peripherals */
void Audio_shutdownCollect(Audio_Handle audio);

void DMA_INT1_IRQHandler(void);

#endif /* AUDIO_COLLECT_H_ */
