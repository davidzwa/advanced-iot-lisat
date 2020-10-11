#include <TDOA/externalInterrupt.h>
#include "tdoaAlgorithm.h"
#include "SerialInterface/serialInterface.h"

// ADCBuf driver structs
ADCBuf_Handle adcBuf;
ADCBuf_Params adcBufParams;
ADCBuf_Conversion continuousConversion[2];

// Timer: Auxiliary variables
unsigned long lastTriggerMic1L = 0;
unsigned long lastTriggerMic2M = 0;
unsigned long lastTriggerMic3R = 0;
bool startAdcSampling = false;

/* Set up an ADCBuf peripheral in ADCBuf_RECURRENCE_MODE_CONTINUOUS */
void initADCBuf() {
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
}

void openADCBuf() {
    // Start ADCBuf here for measurements
    ADCBuf_open(CONFIG_ADCBUF0CHANNEL_0, &adcBufParams);
    ADCBuf_convert(adcBuf, continuousConversion, NUM_ADC_CHANNELS);
}

void closeADCBuf() {
    // Start ADCBuf here for measurements
    ADCBuf_close(adcBuf);
}

void testADCBufOpened() {
    if (!adcBuf){
        /* AdcBuf did not open correctly. */
        while(1);
    }

    /* Start converting sequencer 0. */
    if (ADCBuf_convert(adcBuf, continuousConversion, 1) !=
        ADCBuf_STATUS_SUCCESS) {
        /* Did not start conversion process correctly. */
        while(1);
    }
}

// CASPER's PLAYGROUND
long getCurrentPreciseTime()
{
    // Timer peripheral needs to be setup
    // Do that first, using this example
    // https://dev.ti.com/tirex/explore/node?node=AKyVym.I2F89E.1HEd4gnA__z-lQYNj__LATEST
    return 100; // return time in us
}

void enableMicTriggerInterrupts()
{
    GPIO_setCallback(MIC1L_D_OUT_INTRPT, interruptMic1LTriggered);
    GPIO_setCallback(MIC2M_D_OUT_INTRPT, interruptMic2MTriggered);
    GPIO_setCallback(MIC3R_D_OUT_INTRPT, interruptMic3RTriggered);
    GPIO_enableInt(MIC1L_D_OUT_INTRPT);
    GPIO_enableInt(MIC2M_D_OUT_INTRPT);
    GPIO_enableInt(MIC3R_D_OUT_INTRPT);
}

void disableMicTriggerInterrupts()
{
    GPIO_disableInt(MIC1L_D_OUT_INTRPT);
    GPIO_disableInt(MIC2M_D_OUT_INTRPT);
    GPIO_disableInt(MIC3R_D_OUT_INTRPT);
}

// Set the mic to a new Wake-on-sound cycle
// Dont need delay, because we use ADCBuf fixed timing for this (but it needs delay normally).
void resetWosMicMode()
{
    setWosMode(MIC_LEFT);
    setWosMode(MIC_MID);
    setWosMode(MIC_RIGHT);
}

// Set the mic to continuous listening
void setNormalMicMode(MIC pinNumber)
{
    GPIO_write(pinNumber, 0);
}

void setWosMode(MIC pinNumber)
{
    GPIO_write(pinNumber, 1);
}

/*
 * This function is called whenever a buffer is full. The semaphore can be used to listen for this event in the main thread below.
 */
void adcBufCallback(ADCBuf_Handle handle, ADCBuf_Conversion *conversion,
    void *completedADCBuffer, uint32_t completedChannel) {

    ADCBuf_close(handle);

    // Transfer buffer to our own
    uint_fast16_t i;
    uint16_t *completedBuffer = (uint16_t *) completedADCBuffer;
    for (i = 0; i < CHUNK_LENGTH; i++) {
        outputBuffer[i] = completedBuffer[i];
    }

    int inputTdoaVector2D[2];
    inputTdoaVector2D[0] = lastTriggerMic2M - lastTriggerMic1L;
    inputTdoaVector2D[1] = lastTriggerMic3R - lastTriggerMic1L;
    float outputDirVector2D[2];
    TDOA_direction_estimation(inputTdoaVector2D, outputDirVector2D);

    transmittedData_t serialData = {
        lastTriggerMic1L,
        lastTriggerMic2M,
        lastTriggerMic3R,
        outputDirVector2D[0],
        outputDirVector2D[1],
        CHUNK_LENGTH,
        outputBuffer
    };

    transmitSerialData(&serialData);
    enableMicTriggerInterrupts();
    resetWosMicMode(); // Reset all mics: we are ready for a new round

    /* post adcbuf semaphore */
    sem_post(&adcbufSem);
}

void interruptMic1LTriggered(uint_least8_t index)
{
    lastTriggerMic1L = getCurrentPreciseTime();
    setNormalMicMode(MIC_LEFT); // Disable interrupt externally
    GPIO_disableInt(MIC1L_D_OUT_INTRPT);
}

void interruptMic2MTriggered(uint_least8_t index)
{
    lastTriggerMic2M = getCurrentPreciseTime();
    setNormalMicMode(MIC_MID); // Disable interrupt externally
    GPIO_disableInt(MIC2M_D_OUT_INTRPT);
}

void interruptMic3RTriggered(uint_least8_t index)
{
    lastTriggerMic3R = getCurrentPreciseTime();
    startAdcSampling = true;
    setNormalMicMode(MIC_RIGHT); // Disable interrupt externally
    GPIO_disableInt(MIC3R_D_OUT_INTRPT);

    openADCBuf();
}