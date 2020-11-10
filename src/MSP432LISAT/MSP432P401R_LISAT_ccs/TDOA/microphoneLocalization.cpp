#include <SerialInterface/serialDebugInterface.h>
#include "TDOA/microphoneLocalization.h"
#include "System/freeRunningTimer.h"
#include "tdoaAlgorithm.h"

// ADCBuf driver structs
ADCBuf_Handle adcBuf;
ADCBuf_Params adcBufParams;
ADCBuf_Conversion continuousConversion[2];

// Timer: Auxiliary variables
unsigned long lastTriggerMic1L = 0;
bool mic1LTriggered = false;
unsigned long lastTriggerMic2M = 0;
bool mic2MTriggered = false;
unsigned long lastTriggerMic3R = 0;
bool mic3RTriggered = false;
bool startAdcSampling = false;
bool timerStarted = false;

/* DSP LPF Filter */
Filter* filter = new Filter();

// Long: ADCBUFFERSIZE, short: ADCBUFFERSIZE_SHORT
bool longConversion = false;
void setAdcBufConversion(ADCBuf_Conversion* conversionStruct, bool shortConversion);
void startTimerIfStarted();
void stopTimerIfStarted();

/* Set up an ADCBuf peripheral in ADCBuf_RECURRENCE_MODE_CONTINUOUS */
void initADCBuf() {
    ADCBuf_Params_init(&adcBufParams);
    adcBufParams.callbackFxn = adcBufCallback;
    adcBufParams.recurrenceMode = ADCBuf_RECURRENCE_MODE_ONE_SHOT; // ADCBuf_RECURRENCE_MODE_CONTINUOUS;
    adcBufParams.returnMode = ADCBuf_RETURN_MODE_CALLBACK;
    adcBufParams.samplingFrequency = SAMPLE_FREQUENCY;
    adcBuf = ADCBuf_open(MIC_ADCBUF, &adcBufParams);

    /* Configure the conversion struct for two channels on same sequencer */
    setAdcBufConversion(&continuousConversion[0], true);
}

void setAdcBufConversion(ADCBuf_Conversion* conversionStruct, bool shortConversion) {
    if (shortConversion) {
        longConversion = false;
        conversionStruct[0].arg = NULL;
        conversionStruct[0].adcChannel = MIC_ADCBUFCHANNEL_0;
        conversionStruct[0].sampleBuffer = sampleBuffer1a;
        conversionStruct[0].sampleBufferTwo = sampleBuffer1b;
        conversionStruct[0].samplesRequestedCount = ADCBUFFERSIZE_SHORT;
#if NUM_ADC_CHANNELS >= 2
        conversionStruct[1].arg = NULL;
        conversionStruct[1].adcChannel = MIC_ADCBUFCHANNEL_1; // Mic 2
        conversionStruct[1].sampleBuffer = sampleBuffer2a;
        conversionStruct[1].sampleBufferTwo = sampleBuffer2b;
        conversionStruct[1].samplesRequestedCount = ADCBUFFERSIZE_SHORT;
#elif NUM_ADC_CHANNELS >= 3
        conversionStruct[1].arg = NULL;
        conversionStruct[1].adcChannel = MIC_ADCBUFCHANNEL_2; // Mic 2
        conversionStruct[1].sampleBuffer = sampleBuffer3a;
        conversionStruct[1].sampleBufferTwo = sampleBuffer3b;
        conversionStruct[1].samplesRequestedCount = ADCBUFFERSIZE_SHORT;
#endif
    } else {
        longConversion = true;
        conversionStruct[0].arg = NULL;
        conversionStruct[0].adcChannel = MIC_ADCBUFCHANNEL_0; // Mic 1
        conversionStruct[0].sampleBuffer = sampleBuffer1a;
        conversionStruct[0].sampleBufferTwo = sampleBuffer1b;
        conversionStruct[0].samplesRequestedCount = ADCBUFFERSIZE;

//        conversionStruct[1].arg = NULL;
//        conversionStruct[1].adcChannel = MIC_ADCBUFCHANNEL_1; // Mic 2
//        conversionStruct[1].sampleBuffer = sampleBuffer1a;
//        conversionStruct[1].sampleBufferTwo = sampleBuffer1b;
//        conversionStruct[1].samplesRequestedCount = ADCBUFFERSIZE;
//
//        conversionStruct[2].arg = NULL;
//        conversionStruct[2].adcChannel = MIC_ADCBUFCHANNEL_2; // Mic 3
//        conversionStruct[2].sampleBuffer = sampleBuffer1a;
//        conversionStruct[2].sampleBufferTwo = sampleBuffer1b;
//        conversionStruct[2].samplesRequestedCount = ADCBUFFERSIZE;
    }
}

void openADCBuf() {
    ADCBuf_convert(adcBuf, continuousConversion, 3);
    if (!adcBuf){
        /* AdcBuf did not open correctly. */
        GPIO_write(LED_ERROR_2, 1);
        while(1);
    }
}

void closeADCBuf() {
    // Start ADCBuf here for measurements
    ADCBuf_close(adcBuf);
}

/*
 * This function is called whenever a buffer is full. The semaphore post is awaited in the main thread.
 */
void adcBufCallback(ADCBuf_Handle handle, ADCBuf_Conversion *conversion,
    void *completedADCBuffer, uint32_t completedChannel) {

    uint_fast16_t i;
    uint16_t *completedBuffer = (uint16_t*) completedADCBuffer;
    for (i = 0; i < conversion->samplesRequestedCount; i++) {
        outputBuffer[i] = completedBuffer[i];
    }
    filter->FilterEMABuffer(outputBuffer, outputBuffer_filtered, conversion->samplesRequestedCount);
    if(mic1LTriggered && mic2MTriggered && mic3RTriggered)
    {
        int inputTDOAVector2D[2];
        inputTDOAVector2D[0] = lastTriggerMic1L - lastTriggerMic2M;
        inputTDOAVector2D[1] = lastTriggerMic1L - lastTriggerMic3R;
        TDOA_direction_estimation(inputTDOAVector2D, outputDirVector2D_valin);

        unsigned long inputTOAVector[3] = {lastTriggerMic1L, lastTriggerMic2M, lastTriggerMic3R};
        plane_cutting_direction_estimation(inputTOAVector, outputDirVector2D_plane_cutting);
    }
    else {
        GPIO_toggle(LED_ERROR_2);
    }

    stopTimerIfStarted();
//    enableMicTriggerInterrupts();
//    resetWosMicMode(); // Reset all mics: we are ready for a new round
//    GPIO_write(LED_TRIGGER_1, 0);

//    if(mic1LTriggered && mic2MTriggered && mic3RTriggered)
//    {
//             /* post adcbuf semaphore */
//    }
    sem_post(&adcbufSem);
    mic1LTriggered = false;
    mic2MTriggered = false;
    mic3RTriggered = false;
}

void startTimerIfStopped() {
    if (timerStarted == false) {
        startTimerTacho();
        timerStarted = true;
    }
}

void stopTimerIfStarted() {
    if (timerStarted == true) {
        stopTimerTacho();
        timerStarted = false;
    }
}

// CASPER's PLAYGROUND
uint32_t getCurrentPreciseTime()
{
    // Timer example
    // https://dev.ti.com/tirex/explore/node?node=AKyVym.I2F89E.1HEd4gnA__z-lQYNj__LATEST
    return getTimerUsTacho(); // return time in us
}

void initInterruptCallbacks() {
    GPIO_setCallback(MIC1L_D_OUT_INTRPT, interruptMic1LTriggered);
    GPIO_setCallback(MIC2M_D_OUT_INTRPT, interruptMic2MTriggered);
    GPIO_setCallback(MIC3R_D_OUT_INTRPT, interruptMic3RTriggered);
}

void enableMicTriggerInterrupts()
{
    GPIO_clearInt(MIC1L_D_OUT_INTRPT);
    GPIO_clearInt(MIC2M_D_OUT_INTRPT);
    GPIO_clearInt(MIC3R_D_OUT_INTRPT);
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

void interruptMic1LTriggered(uint_least8_t index)
{
    if (mic1LTriggered == true) {
        GPIO_toggle(LED_ERROR_2);
        return;
    }
    startTimerIfStopped();
    lastTriggerMic1L = getCurrentPreciseTime();
    mic1LTriggered = true;
    setNormalMicMode(MIC_LEFT); // Disable interrupt externally
    GPIO_disableInt(MIC1L_D_OUT_INTRPT);
}

void interruptMic2MTriggered(uint_least8_t index)
{
    if (mic2MTriggered == true) {
        GPIO_toggle(LED_ERROR_2);
        return;
    }
    startTimerIfStopped();
    lastTriggerMic2M = getCurrentPreciseTime();
    mic2MTriggered = true;
    setNormalMicMode(MIC_MID); // Disable interrupt externally
    GPIO_disableInt(MIC2M_D_OUT_INTRPT);
}

void interruptMic3RTriggered(uint_least8_t index)
{
    if (mic3RTriggered == true) {
        GPIO_toggle(LED_ERROR_2);
        return;
    }
    startTimerIfStopped();
    lastTriggerMic3R = getCurrentPreciseTime();
    mic3RTriggered = true;
    setNormalMicMode(MIC_RIGHT); // Disable interrupt externally
    GPIO_disableInt(MIC3R_D_OUT_INTRPT);

    GPIO_write(LED_TRIGGER_1, 1);
    openADCBuf();
}
