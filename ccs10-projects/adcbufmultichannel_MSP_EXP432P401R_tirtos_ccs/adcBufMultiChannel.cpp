/*
 *  ======== adcbufMultiChannel.c ========
 */

/* DriverLib Includes */
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#include <semaphore.h>

/* Peripheral Header files */
#include <ti/drivers/ADCBuf.h>
#include <ti/drivers/Capture.h>
#include <ti/drivers/GPIO.h>
#include <ti/display/Display.h>
#include <ti/display/DisplayUart.h>

/* Board Header file */
#include "ti_drivers_config.h"

// Our common defines
#include "common.h"
#include "dsp/iirfilter.h"
#include "dsp/fft.h"
#include "corrAudio.h"

#define NUM_ADC_CHANNELS (1)
#define ADCBUFFERSIZE    (CHUNK_LENGTH)

int16_t sampleBuffer1a[ADCBUFFERSIZE];
int16_t sampleBuffer1b[ADCBUFFERSIZE];
//int16_t fftOutput[ADCBUFFERSIZE];
#if NUM_ADC_CHANNELS >= 2
int16_t sampleBuffer2a[ADCBUFFERSIZE];
int16_t sampleBuffer2b[ADCBUFFERSIZE];
#endif
#if NUM_ADC_CHANNELS >= 3
int16_t sampleBuffer3a[ADCBUFFERSIZE];
int16_t sampleBuffer3b[ADCBUFFERSIZE];
#endif
int32_t buffersCompletedCounter = 0;
//int16_t correlation[CHUNK_LENGTH];
int16_t outputBuffer[ADCBUFFERSIZE];
int16_t outputBuffer_filtered[ADCBUFFERSIZE];
int16_t rms;

/* Display Driver Handle */
Display_Handle display;
volatile uint32_t curInterval;

/* ADCBuf semaphore */
sem_t adcbufSem;

/* DSP LPF Filter */
IirFilter* filter;

/* Callback used for blinking LED on timer completion */
//void captureCallback(Capture_Handle handle, uint32_t interval);

/* Callback function that displays the interval */
void captureCallback(Capture_Handle handle, uint32_t interval)
{
    curInterval = interval;
    GPIO_write(CONFIG_GPIO_0, 0);
    GPIO_write(CONFIG_GPIO_0, 1);
//    SemaphoreP_post(captureSem);
}

void interruptCallback(uint_least8_t index) {
    /* Clear the GPIO interrupt and toggle an LED */
    GPIO_toggle(CONFIG_GPIO_LED_1);
    GPIO_write(CONFIG_GPIO_0, 0);
}

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
//    Capture_Params captureParams;
//    Capture_Handle capture;
    ADCBuf_Handle adcBuf;
    ADCBuf_Params adcBufParams;
    ADCBuf_Conversion continuousConversion[2];
    uint_fast16_t i;
    int32_t status;

    /* Call driver init functions */
    ADCBuf_init();
    GPIO_init();
    Display_init();
//    Capture_init();

    /* Turn on user LED */
//    GPIO_write(CONFIG_, CONFIG_GPIO_LED_ON);
    GPIO_write(CONFIG_GPIO_0, 1);

    /* Configure & open Display driver */
    Display_Params_init(&displayParams);
    displayParams.lineClearMode = DISPLAY_CLEAR_BOTH;
    display = Display_open(Display_Type_UART, &displayParams);
    if (display == NULL) {
        Display_printf(display, 0, 0, "Error creating display\n");
        while (1);
    }
    
    status = sem_init(&adcbufSem, 0, 0);
    if (status != 0) {
        Display_printf(display, 0, 0, "Error creating adcbufSem\n");
        while(1);
    }

    GPIO_setCallback(INTERRUPT_D_OUT2, interruptCallback);
    GPIO_enableInt(INTERRUPT_D_OUT2);

    /* Setting up the Capture driver to detect two rising edges and report
     * the result in microseconds
     */
//    Capture_Params_init(&captureParams);
//    captureParams.mode = Capture_RISING_EDGE;
//    captureParams.periodUnit = Capture_PERIOD_US;
//    captureParams.callbackFxn = captureCallback;
//    capture = Capture_open(CONFIG_CAPTURE_0, &captureParams);
//    if (capture == NULL)
//    {
//        Display_printf(display, 0, 0, "Failed to initialized Capture!\n");
//        while(1);
//    }
    Display_printf(display, 0, 0, "Capture Driver initialized.\n");

    Display_printf(display, 0, 0, "Dual ADCBuf driver initialized.");
//    Capture_start(capture);

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
    filter = new IirFilter();
    filter->InitFilterState();

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
//        Display_printf(display, 0, 0, "\r\n", //Buffer %u finished:
//            (unsigned int)buffersCompletedCounter++);

//         filter->FilterBuffer(outputBuffer, outputBuffer_filtered);
        filter->FilterEMABuffer(outputBuffer, outputBuffer_filtered);

//        doFFT(outputBuffer_filtered, fftOutput);
//        arm_correlate_q15(outputBuffer_filtered, CHUNK_LENGTH, audioVector, AUDIO_LENGTH, correlation);

//        for (i = 0; i < ADCBUFFERSIZE; i++) {
//            Display_printf(display, 0, 0, "v.%d", outputBuffer_filtered[i]);
//        }
        arm_rms_q15(outputBuffer_filtered, ADCBUFFERSIZE, &rms);
        Display_printf(display, 0, 0, "R.%d", rms);
//        if (rms > 0) {
//
//        }
//        int16_t maxValue;
//        uint32_t maxIndex;
//        arm_max_q15(outputBuffer, ADCBUFFERSIZE, &maxValue, &maxIndex);

//        outputBuffer_filtered[]
//        Display_printf(display, 0, 0, "%d", maxValue);
        Display_printf(display, 0, 0, "--Done");
        numBufsSent++;
    }

}
