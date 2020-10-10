/*
 *  ======== adcbufMultiChannel.c ========
 */

/* DriverLib Includes */
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#include <semaphore.h>

/* Driver Header files */
#include <ti/drivers/ADCBuf.h>

/* Display Header files */
#include <ti/display/Display.h>
#include <ti/display/DisplayUart.h>

/* Board Header file */
#include "ti_drivers_config.h"

// Our common defines
#include "common.h"
#include "dsp/iirfilter.h"

#define NUM_ADC_CHANNELS (1)
#define ADCBUFFERSIZE    (CHUNK_LENGTH)

uint16_t sampleBuffer1a[ADCBUFFERSIZE];
uint16_t sampleBuffer1b[ADCBUFFERSIZE];
uint16_t sampleBuffer2a[ADCBUFFERSIZE];
uint16_t sampleBuffer2b[ADCBUFFERSIZE];
uint16_t sampleBuffer3a[ADCBUFFERSIZE];
uint16_t sampleBuffer3b[ADCBUFFERSIZE];
uint32_t buffersCompletedCounter = 0;
uint16_t outputBuffer[ADCBUFFERSIZE];
//uint16_t outputBuffer_filtered[ADCBUFFERSIZE];

/* Display Driver Handle */
Display_Handle displayHandle;

/* ADCBuf semaphore */
sem_t adcbufSem;

/* DSP LPF Filter */
//IirFilter* filter;

/*
 * This function is called whenever a buffer is full.
 * The content of the buffer is then converted into human-readable format and
 * sent to the PC via UART.
 *
 */
void adcBufCallback(ADCBuf_Handle handle, ADCBuf_Conversion *conversion,
    void *completedADCBuffer, uint32_t completedChannel) {
    uint_fast16_t i;
    uint16_t *completedBuffer = (uint16_t *) completedADCBuffer;
 
    for (i = 0; i < ADCBUFFERSIZE; i++) {
        outputBuffer[i] = completedBuffer[i];
    }
 
    /* post adcbuf semaphore */
    sem_post(&adcbufSem);
}

/*
 *  ======== mainThread ========
 */
void *mainThread(void *arg0)
{
    Display_Params displayParams;
    ADCBuf_Handle adcBuf;
    ADCBuf_Params adcBufParams;
    ADCBuf_Conversion continuousConversion[2];
    uint_fast16_t i;
    int32_t status;

    /* Call driver init functions */
    ADCBuf_init();
    Display_init();

    /* Configure & open Display driver */
    Display_Params_init(&displayParams);
    displayParams.lineClearMode = DISPLAY_CLEAR_BOTH;
    displayHandle = Display_open(Display_Type_UART, &displayParams);
    if (displayHandle == NULL) {
        Display_printf(displayHandle, 0, 0, "Error creating displayHandle\n");
        while (1);
    }
    
    status = sem_init(&adcbufSem, 0, 0);
    if (status != 0) {
        Display_printf(displayHandle, 0, 0, "Error creating adcbufSem\n");
        while(1);
    }

    Display_printf(displayHandle, 0, 0, "Starting the ADCBuf Multi-Channel example");

    /* Set up an ADCBuf peripheral in ADCBuf_RECURRENCE_MODE_CONTINUOUS */
    ADCBuf_Params_init(&adcBufParams);
    adcBufParams.callbackFxn = adcBufCallback;
    adcBufParams.recurrenceMode = ADCBuf_RECURRENCE_MODE_CONTINUOUS;
    adcBufParams.returnMode = ADCBuf_RETURN_MODE_CALLBACK;
    adcBufParams.samplingFrequency = SAMPLE_FREQUENCY;
    adcBuf = ADCBuf_open(CONFIG_ADCBUF0, &adcBufParams);

    /* Configure the conversion struct for two channels on same sequencer */
    continuousConversion[0].arg = NULL;
    continuousConversion[0].adcChannel = CONFIG_ADCBUF0CHANNEL_0;
    continuousConversion[0].sampleBuffer = sampleBuffer1a;
    continuousConversion[0].sampleBufferTwo = sampleBuffer1b;
    continuousConversion[0].samplesRequestedCount = ADCBUFFERSIZE;

#if NUM_ADC_CHANNELS >= 2
    continuousConversion[1].arg = NULL;
    continuousConversion[1].adcChannel = CONFIG_ADCBUF0CHANNEL_1;
    continuousConversion[1].sampleBuffer = sampleBuffer2a;
    continuousConversion[1].sampleBufferTwo = sampleBuffer2b;
    continuousConversion[1].samplesRequestedCount = ADCBUFFERSIZE;
#endif
#if NUM_ADC_CHANNELS == 3
    continuousConversion[1].arg = NULL;
    continuousConversion[1].adcChannel = CONFIG_ADCBUF0CHANNEL_2;
    continuousConversion[1].sampleBuffer = sampleBuffer3a;
    continuousConversion[1].sampleBufferTwo = sampleBuffer3b;
    continuousConversion[1].samplesRequestedCount = ADCBUFFERSIZE;
#endif
    if (!adcBuf){
        /* AdcBuf did not open correctly. */
        while(1);
    }

    // Enable IirFilter
//    filter = new IirFilter();
//    filter->InitFilterState();


    /* Start converting sequencer 0. */
    if (ADCBuf_convert(adcBuf, continuousConversion, NUM_ADC_CHANNELS) !=
        ADCBuf_STATUS_SUCCESS) {
        /* Did not start conversion process correctly. */
        while(1);
    }

    /*
     * Go to sleep in the foreground thread forever. The data will be collected
     * and transfered in the background thread
     */
    int numBufsSent = 0;
    while(1) {
        sem_wait(&adcbufSem);
        
        /*
         * Start with a header message and print current buffer values
         */
//        Display_printf(displayHandle, 0, 0, "\r\n", //Buffer %u finished:
//            (unsigned int)buffersCompletedCounter++);

        // filter->FilterBuffer(outputBuffer, outputBuffer_filtered);

        for (i = 0; i < ADCBUFFERSIZE; i++) {
            ;
            Display_printf(displayHandle, 0, 0, "v.%u\r\n", outputBuffer[i]);
        }

        Display_printf(displayHandle, 0, 0, "--Done\r\n");
        numBufsSent++;
    }

}
