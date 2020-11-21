#include <ti/sysbios/knl/Clock.h>

#include "common.h"
#include <TDOA/microphoneLocalization.h>
#include <TDOA/signalPreambleDetector.h>
#include <TDOA/signalCodeDetector.h>
#include "tdoaAlgorithm.h"

// Ease of debugging
int adcshort = ADCBUFFERSIZE_SHORT;

// ADCBuf driver structs
ADCBuf_Handle adcBuf;
ADCBuf_Params adcBufParams;
ADCBuf_Conversion conversionStruct[3];

Filter* filter = new Filter();

bool startAdcSampling = false;
int lastChannelCompleted = -1;
bool completedChannels[3] = {false, false, false};
bool shortBufferMode = false;
int16_t rms;
void adcBufCompletionCallback(ADCBuf_Handle handle, ADCBuf_Conversion *conversion, void *completedADCBuffer, uint32_t completedChannel);
bool allChannelsCompleted(int setCompletedChannel);

/* Set up an ADCBuf peripheral in ADCBuf_RECURRENCE_MODE_CONTINUOUS */
void initADCBuf() {
    ADCBuf_Params_init(&adcBufParams);
    adcBufParams.samplingFrequency = SAMPLE_FREQUENCY;
#if MIC_CONTINUOUS_SAMPLE == 1
    adcBufParams.recurrenceMode = ADCBuf_RECURRENCE_MODE_CONTINUOUS;
#else
    adcBufParams.recurrenceMode = ADCBuf_RECURRENCE_MODE_ONE_SHOT; // ADCBuf_RECURRENCE_MODE_CONTINUOUS;
#endif
    adcBufParams.returnMode = ADCBuf_RETURN_MODE_CALLBACK;
    adcBufParams.callbackFxn = adcBufCompletionCallback;
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

    conversionStruct[0].arg = NULL;
    conversionStruct[0].adcChannel = MIC_ADCBUFCHANNEL_0;
    conversionStruct[0].sampleBuffer = sampleBuffer1a;
#if MIC_CONTINUOUS_SAMPLE
    conversionStruct[0].sampleBufferTwo = sampleBuffer1b;
#endif
    conversionStruct[0].samplesRequestedCount = bufferLength;
#if NUM_ADC_CHANNELS >= 2
    conversionStruct[1].arg = NULL;
    conversionStruct[1].adcChannel = MIC_ADCBUFCHANNEL_1; // Mic 2
    conversionStruct[1].sampleBuffer = sampleBuffer2a;
#if MIC_CONTINUOUS_SAMPLE
    conversionStruct[1].sampleBufferTwo = sampleBuffer2b;
#endif
    conversionStruct[1].samplesRequestedCount = bufferLength;
#endif
#if NUM_ADC_CHANNELS >= 3
    conversionStruct[2].arg = NULL;
    conversionStruct[2].adcChannel = MIC_ADCBUFCHANNEL_2; // Mic 2
    conversionStruct[2].sampleBuffer = sampleBuffer3a;
#if MIC_CONTINUOUS_SAMPLE
    conversionStruct[2].sampleBufferTwo = sampleBuffer3b;
#endif
    conversionStruct[2].samplesRequestedCount = bufferLength;
#endif
    shortBufferMode = shortConversion;
}

void openADCBuf() {
    ADCBuf_convert(adcBuf, conversionStruct, NUM_ADC_CHANNELS);
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
#if MSP_MIC_RAW_MODE == 0
    uint16_t *completedBuffer = (uint16_t*) completedADCBuffer;
    filter->FilterEMABuffer(completedBuffer, outputBuffer_filtered, conversion->samplesRequestedCount);

    if (shortBufferMode) {
        // Track history - short buffer only
        bool result = signalPreambleDetector(outputBuffer_filtered, &detection_history_mics[completedChannel]);
    }
#endif

    if (allChannelsCompleted(completedChannel)) {
        if (!shortBufferMode) {
            // Preamble was detected, abuse ERROR LED for it.
            GPIO_toggle(LED_ERROR_2);

            // Perform long buffer analysis
            valin_tdoa_input tdoaData = processThreeLongBuffer(sampleBuffer1a, sampleBuffer2a, sampleBuffer3a, conversion->samplesRequestedCount);

            GPIO_toggle(LED_ERROR_2);
        }

        startAdcSampling = false;
        sem_post(&adcbufSem);
    }
}

bool allChannelsCompleted(int setCompletedChannel) {
    completedChannels[setCompletedChannel] = true;
    bool syncCompletion = completedChannels[0]
                                            && (completedChannels[1] || NUM_ADC_CHANNELS<2)
                                            && (completedChannels[2] || NUM_ADC_CHANNELS<3);

    // All channels completed, reset
    if (syncCompletion) {
        completedChannels[0]=false;
        completedChannels[1]=false;
        completedChannels[2]=false;
    }
    lastChannelCompleted = setCompletedChannel;
    return syncCompletion;
}

void resetPreambleDetectionHistory()
{
    detection_history_mics[0] = false;
    detection_history_mics[1] = false;
    detection_history_mics[2] = false;
}

bool wasPreambleDetected()
{
    return detection_history_mics[0]
                                  && (detection_history_mics[1] || NUM_ADC_CHANNELS<2)
                                  && (detection_history_mics[2] || NUM_ADC_CHANNELS<3);
}
