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

/** \file audio_collect.c Contains functions for setting up a ping-pong
 * buffer for audio data, starting and stopping audio data collection, and
 * the interrupt handler for the DMA interrupt that declares a buffer of
 * data is available for processing.\n (C) Copyright 2015, Texas Instruments,
 * Inc.
 */

/*----------------------------------------------------------------------------
 * Includes
 * --------------------------------------------------------------------------*/
#include <stdlib.h>

#include <ti/devices/msp432p4xx/inc/msp.h>
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

#include "audio_collect_private.h"

/*----------------------------------------------------------------------------
 * Globals
 * -------------------------------------------------------------------------*/

Audio_Object AudioGlobal;

/*----------------------------------------------------------------------------
 * Functions
 * --------------------------------------------------------------------------*/
/* Set up the device to collect audio samples in ping-pong buffers */
Audio_Handle Audio_setupCollect( uint32_t *dmaControlTable,
    int_least16_t *audioBuffer1, int_least16_t *audioBuffer2,
    uint_least32_t bufferSize, uint_least32_t sampleRate)
{

    Audio_Handle audio = &AudioGlobal;

    /* Initial audio object settings */
    audio->dmaControlTable = dmaControlTable;
    audio->audioBuffer1 = audioBuffer1;
    audio->audioBuffer2 = audioBuffer2;
    audio->bufferSize = bufferSize;
    audio->sampleRate = sampleRate;

    audio->overflow = false;
    audio->bufferActive = false;


    /* Power up the audio booster-pack preamp. Output high = power on */
    MAP_GPIO_setAsOutputPin(AUDIO_PREAMP_PORT, AUDIO_PREAMP_PIN);
    MAP_GPIO_setOutputHighOnPin(AUDIO_PREAMP_PORT, AUDIO_PREAMP_PIN);

    #if defined(AUDIODEBUG)
    /* DEBUG ONLY P7.7 indicates audio timing */
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P7, GPIO_PIN7);
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P7, GPIO_PIN7);
    #endif

    return audio;
}

/*--------------------------------------------------------------------------*/
/* Start collecting audio samples in ping-pong buffers */
void Audio_startCollect(Audio_Handle audio)
{
    uint_fast32_t smclk;

    Timer_A_UpModeConfig upConfig =
    {
        TIMER_A_CLOCKSOURCE_SMCLK,
        TIMER_A_CLOCKSOURCE_DIVIDER_1,
        2999,
        TIMER_A_TAIE_INTERRUPT_DISABLE,
        TIMER_A_CCIE_CCR0_INTERRUPT_DISABLE,
        TIMER_A_DO_CLEAR
    };

    Timer_A_CompareModeConfig compareConfig =
    {
        TIMER_A_CAPTURECOMPARE_REGISTER_1,
        TIMER_A_CAPTURECOMPARE_INTERRUPT_DISABLE,
        TIMER_A_OUTPUTMODE_TOGGLE_RESET,
        1500
    };

    /* Start with primary (first) audio buffer transfer */
    audio->fillingBuffer1 = true;
    audio->overflow = false;
    audio->bufferActive = false;

    /* Set up Timer A0 to generate 8kHz sample hold signal for ADC14. */
    MAP_Interrupt_disableInterrupt(INT_TA0_0);
    MAP_Interrupt_disableInterrupt(INT_TA0_N);

    /* Determine timer settings */
    smclk = MAP_CS_getSMCLK();
    upConfig.timerPeriod = (smclk / audio->sampleRate) - 1;
    compareConfig.compareValue = upConfig.timerPeriod / 2;

    /* Ensure timer stopped and cleared */
    MAP_Timer_A_stopTimer(TIMER_A0_BASE);
    MAP_Timer_A_clearTimer(TIMER_A0_BASE);

    /* Set timer to count up at sample Rate */
    MAP_Timer_A_configureUpMode(TIMER_A0_BASE, &upConfig);

    /* Set timer output to sample ADC input signal for half cycle */
    MAP_Timer_A_initCompare(TIMER_A0_BASE, &compareConfig);

    /* Set ADC14 to use audio output port as input, TA0C1 as trigger, DMA as output,
     * and to run in single sample repeat mode. */
    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(AUDIO_OUT_PORT, AUDIO_OUT_PIN,
        GPIO_TERTIARY_MODULE_FUNCTION);

    // Ensure ADC14 stopped
    MAP_ADC14_disableConversion();

    // Clear any outstanding busy bit
    ADC14->CTL0 &= ~(ADC14_CTL0_CONSEQ_MASK);

    // Set-up ADC14
    MAP_ADC14_enableModule();
    MAP_ADC14_initModule(ADC_CLOCKSOURCE_SMCLK, ADC_PREDIVIDER_1,
        ADC_DIVIDER_1, ADC_NOROUTE);
    MAP_ADC14_setResolution(ADC_14BIT);
    MAP_ADC14_setResultFormat(ADC_SIGNED_BINARY);
    MAP_ADC14_disableSampleTimer();
    MAP_ADC14_setSampleHoldTrigger(ADC_TRIGGER_SOURCE1, true);

    /* Does ADC_INT0 need to be enabled for DMA to receive it? */
    MAP_Interrupt_disableInterrupt(INT_ADC14);
    MAP_ADC14_enableInterrupt(ADC_INT0);

    MAP_ADC14_configureSingleSampleMode(ADC_MEM0, true);
    MAP_ADC14_configureConversionMemory(ADC_MEM0, ADC_VREFPOS_AVCC_VREFNEG_VSS,
        AUDIO_ADC_INPUT, ADC_NONDIFFERENTIAL_INPUTS);
    MAP_ADC14_disableComparatorWindow(ADC_MEM0);

    /* Set up DMA to connect to ADC14 */
    MAP_Interrupt_disableInterrupt(INT_DMA_INT1);
    MAP_DMA_enableModule();
    MAP_DMA_setControlBase(audio->dmaControlTable);
    MAP_DMA_assignChannel(DMA_CH7_ADC14);
    MAP_DMA_assignInterrupt(DMA_INT1, DMA_CHANNEL_7);

    /* set DMA to priority 1, highest app priority (system priorities highest) */
    MAP_Interrupt_setPriority(INT_DMA_INT1, 1<<5);

    MAP_DMA_enableChannelAttribute(DMA_CHANNEL_7, UDMA_ATTR_HIGH_PRIORITY);
    MAP_DMA_disableChannelAttribute(DMA_CHANNEL_7,
        UDMA_ATTR_USEBURST | UDMA_ATTR_ALTSELECT | UDMA_ATTR_REQMASK);

    /* Primary & alternate channels set to move audio from ADC14 to memory */
    MAP_DMA_setChannelControl(DMA_CHANNEL_7 | UDMA_PRI_SELECT,
        UDMA_SIZE_16 | UDMA_SRC_INC_NONE | UDMA_DST_INC_16 | UDMA_ARB_1);

    MAP_DMA_setChannelControl(DMA_CHANNEL_7 | UDMA_ALT_SELECT,
        UDMA_SIZE_16 | UDMA_SRC_INC_NONE | UDMA_DST_INC_16 | UDMA_ARB_1);

    /* Primary DMA transfer settings for ADC14MEM0 to audioBuffer1 */
    MAP_DMA_setChannelTransfer(DMA_CHANNEL_7 | UDMA_PRI_SELECT,
        UDMA_MODE_PINGPONG,
        (void *) &ADC14->MEM[0], (void *)audio->audioBuffer1, audio->bufferSize);

    /* Alternate DMA transfer settings for ADC14MEM0 to audioBuffer2 */
    MAP_DMA_setChannelTransfer(DMA_CHANNEL_7 | UDMA_ALT_SELECT,
        UDMA_MODE_PINGPONG,
        (void *) &ADC14->MEM[0], (void *)audio->audioBuffer2, audio->bufferSize);

    /* Enable processor to receive DMA interrupt, and enable DMA channel */
    MAP_Interrupt_enableInterrupt(INT_DMA_INT1);
    MAP_DMA_enableChannel(DMA_CHANNEL_7);

    /* Enable the ADC14 to convert samples */
    MAP_Timer_A_clearTimer(TIMER_A0_BASE);
    MAP_ADC14_enableConversion();

    /* Start TA0 timer to begin audio data collection */
    MAP_Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_UP_MODE);
}

/*--------------------------------------------------------------------------*/
/* Stop collecting audio samples in buffers */
void Audio_stopCollect(Audio_Handle audio)
{

    // Disable ADC conversion
    MAP_ADC14_disableConversion();

    MAP_Timer_A_stopTimer(TIMER_A0_BASE);
    MAP_Timer_A_clearTimer(TIMER_A0_BASE);

    // Clear any outstanding busy bit condition in ADC
    ADC14->CTL0 &= ~ADC14_CTL0_CONSEQ_MASK;
    ADC14->CTL0 |= ADC14_CTL0_CONSEQ_2;

    /* Disable the ADC */
    MAP_ADC14_disableModule();

    /* Disable DMA channel and processor interrupt */
    MAP_DMA_disableChannel(DMA_CHANNEL_7);
    MAP_Interrupt_disableInterrupt(INT_DMA_INT1);
    MAP_DMA_disableModule();


#if defined(AUDIODEBUG)
    /* !!! DEBUG ONLY P7.7 indicates timing with Sharp display */
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P7, GPIO_PIN7);
#endif
}

/*--------------------------------------------------------------------------*/
/* Shut down the audio collection peripherals*/
void Audio_shutdownCollect(Audio_Handle audio)
{
    /* Turn off preamp power */
    MAP_GPIO_setOutputLowOnPin(AUDIO_PREAMP_PORT, AUDIO_PREAMP_PIN);
}

/*--------------------------------------------------------------------------*/
/* Get pointer to active buffer with valid data ready for processing */
int_least16_t *Audio_getActiveBuffer(Audio_Handle audio)
{
    if(audio->fillingBuffer1)
    {
        return audio->audioBuffer2;
    }
    else
    {
        return audio->audioBuffer1;
    }
}

/*--------------------------------------------------------------------------*/
/* Indicate if an active buffer exists to be processed */
bool Audio_getActive(Audio_Handle audio)
{
    return(audio->bufferActive);
}

/*--------------------------------------------------------------------------*/
/* Indicate done processing active buffer holding valid data */
void Audio_resetActive(Audio_Handle audio)
{
    audio->bufferActive = false;


#if defined(AUDIODEBUG)
    /* !!! DEBUG ONLY P7.7 indicates timing with Sharp display */
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P7, GPIO_PIN7);
#endif
}

/*--------------------------------------------------------------------------*/
/* Get overflow status */
bool Audio_getOverflow(Audio_Handle audio)
{
    return audio->overflow;
}

/*--------------------------------------------------------------------------*/
/* Reset overflow status */
void Audio_resetOverflow(Audio_Handle audio)
{
    audio->overflow = false;
}

/*--------------------------------------------------------------------------*/
/* Interrupt for DMA */
void DMA_INT1_IRQHandler(void)
{
    /* Buffer being processed should not be active on interrupt */
    if(AudioGlobal.bufferActive)
    {
        AudioGlobal.overflow = true;
    }

    if(MAP_DMA_getChannelAttribute(DMA_CHANNEL_7) & UDMA_ATTR_ALTSELECT)
    {
        /* Reconfigure primary DMA transfer settings for next transfer */
        MAP_DMA_setChannelTransfer(DMA_CHANNEL_7 | UDMA_PRI_SELECT,
            UDMA_MODE_PINGPONG, (void *) &ADC14->MEM[0],
            (void *)AudioGlobal.audioBuffer1, AudioGlobal.bufferSize);

        AudioGlobal.fillingBuffer1 = false;
    }
    else
    {
        /* Reconfigure alternate DMA transfer settings for next transfer */
        MAP_DMA_setChannelTransfer(DMA_CHANNEL_7 | UDMA_ALT_SELECT,
            UDMA_MODE_PINGPONG, (void *) &ADC14->MEM[0],
            (void *)AudioGlobal.audioBuffer2, AudioGlobal.bufferSize);

        AudioGlobal.fillingBuffer1 = true;
    }


    /* Indicate a buffer of data is ready to process */
    AudioGlobal.bufferActive = true;


#if defined(AUDIODEBUG)
    /* !!! DEBUG ONLY P7.7 indicates timing with Sharp display */
    MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P7, GPIO_PIN7);
#endif


    /* Always allow processing of frame data upon ISR exit */
   Interrupt_disableSleepOnIsrExit();
}

