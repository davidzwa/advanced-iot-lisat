#include <SerialInterface/serialDebugInterface.h>
#include "TDOA/microphoneLocalization.h"
#include "tdoaAlgorithm.h"

// ADCBuf driver structs
ADCBuf_Handle adcBuf;
ADCBuf_Params adcBufParams;
ADCBuf_Conversion conversionStruct[3];

Filter* filter = new Filter();

bool startAdcSampling = false;
int lastChannelCompleted = -1;
bool shortBufferMode = false;

void adcBufCompletionCallback(ADCBuf_Handle handle, ADCBuf_Conversion *conversion, void *completedADCBuffer, uint32_t completedChannel);

/* Set up an ADCBuf peripheral in ADCBuf_RECURRENCE_MODE_CONTINUOUS */
void initADCBuf() {
    ADCBuf_Params_init(&adcBufParams);
    adcBufParams.callbackFxn = adcBufCompletionCallback;
    adcBufParams.recurrenceMode = ADCBuf_RECURRENCE_MODE_ONE_SHOT; // ADCBuf_RECURRENCE_MODE_CONTINUOUS;
    adcBufParams.returnMode = ADCBuf_RETURN_MODE_CALLBACK;
    adcBufParams.samplingFrequency = SAMPLE_FREQUENCY;
    adcBuf = ADCBuf_open(MIC_ADCBUF, &adcBufParams);

    /* Configure the conversion struct for 1-3 channels on same sequencer */
    setAdcBufConversionMode(true);
}

void setAdcBufConversionMode(bool shortConversion) {
    uint16_t bufferLength;
    if (shortConversion) {
        bufferLength = ADCBUFFERSIZE_SHORT;
    }else {
        bufferLength = ADCBUFFERSIZE;
    }
    shortBufferMode = shortConversion;

    conversionStruct[0].arg = NULL;
    conversionStruct[0].adcChannel = MIC_ADCBUFCHANNEL_0;
    conversionStruct[0].sampleBuffer = sampleBuffer1a;
    conversionStruct[0].sampleBufferTwo = sampleBuffer1b;
    conversionStruct[0].samplesRequestedCount = bufferLength;
#if NUM_ADC_CHANNELS >= 2
    conversionStruct[1].arg = NULL;
    conversionStruct[1].adcChannel = MIC_ADCBUFCHANNEL_1; // Mic 2
    conversionStruct[1].sampleBuffer = sampleBuffer2a;
    conversionStruct[1].sampleBufferTwo = sampleBuffer2b;
    conversionStruct[1].samplesRequestedCount = bufferLength;
#endif
#if NUM_ADC_CHANNELS >= 3
    conversionStruct[2].arg = NULL;
    conversionStruct[2].adcChannel = MIC_ADCBUFCHANNEL_2; // Mic 2
    conversionStruct[2].sampleBuffer = sampleBuffer3a;
    conversionStruct[2].sampleBufferTwo = sampleBuffer3b;
    conversionStruct[2].samplesRequestedCount = bufferLength;
#endif
}

void openADCBuf() {
    ADCBuf_convert(adcBuf, conversionStruct, 3);
    if (!adcBuf){
        /* AdcBuf did not open correctly. */
        GPIO_write(LED_ERROR_2, 1);
        while(1);
    }
    else {
        startAdcSampling = true;
    }
}

// Not used as we have a one-shot adcbuf
void closeADCBuf() {
    // Start ADCBuf here for measurements
    ADCBuf_close(adcBuf);
}

/*
 * This function is called whenever a buffer is full. The semaphore post is awaited in the main thread.
 */
void adcBufCompletionCallback(ADCBuf_Handle handle, ADCBuf_Conversion *conversion,
    void *completedADCBuffer, uint32_t completedChannel) {
    uint_fast16_t i;
    uint16_t *completedBuffer = (uint16_t*) completedADCBuffer;
    for (i = 0; i < conversion->samplesRequestedCount; i++) {
        outputBuffer[i] = completedBuffer[i];
    }
    filter->FilterEMABuffer(outputBuffer, outputBuffer_filtered, conversion->samplesRequestedCount);
    lastChannelCompleted = completedChannel;

    if (lastChannelCompleted == 0) {
        sem_post(&adcbufSem);
    }
    else {
        GPIO_toggle(LED_BLUE_2_GPIO);
    }
    startAdcSampling = false;
}
