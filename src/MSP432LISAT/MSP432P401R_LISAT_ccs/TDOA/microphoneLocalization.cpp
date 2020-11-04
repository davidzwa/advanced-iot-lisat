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


/* Set up an ADCBuf peripheral in ADCBuf_RECURRENCE_MODE_CONTINUOUS */
void initADCBuf() {
    ADCBuf_Params_init(&adcBufParams);
    adcBufParams.callbackFxn = adcBufCallback;
    adcBufParams.recurrenceMode = ADCBuf_RECURRENCE_MODE_ONE_SHOT; // ADCBuf_RECURRENCE_MODE_CONTINUOUS;
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
    continuousConversion[2].arg = NULL;
    continuousConversion[2].adcChannel = CONFIG_ADCBUF0CHANNEL_2;
    continuousConversion[2].sampleBuffer = sampleBuffer3a;
    continuousConversion[2].sampleBufferTwo = sampleBuffer3b;
    continuousConversion[2].samplesRequestedCount = ADCBUFFERSIZE;
#endif
}

void openADCBuf() {
    // Start ADCBuf here for measurements
//    ADCBuf_open(CONFIG_ADCBUF0CHANNEL_0, &adcBufParams);
    ADCBuf_convert(adcBuf, continuousConversion, NUM_ADC_CHANNELS);

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
 * This function is called whenever a buffer is full. The semaphore post is awaited in the main thread.
 */
void adcBufCallback(ADCBuf_Handle handle, ADCBuf_Conversion *conversion,
    void *completedADCBuffer, uint32_t completedChannel) {

    // Transfer buffer to our own
    uint_fast16_t i;
    uint16_t *completedBuffer = (uint16_t *) completedADCBuffer;
    for (i = 0; i < CHUNK_LENGTH; i++) {
        outputBuffer[i] = completedBuffer[i];
    }

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
        GPIO_toggle(LED_ERROR_2, 1);
    }

// Not smart to do in interrupt/callback
//    transmittedData_t serialData = {
//        lastTriggerMic1L,
//        lastTriggerMic2M,
//        lastTriggerMic3R,
//        outputDirVector2D_valin[0],
//        outputDirVector2D_valin[1],
//        CHUNK_LENGTH,
//        outputBuffer
//    };
    //transmitSerialData(&serialData);

    stopTimerIfStarted();
    enableMicTriggerInterrupts();
    resetWosMicMode(); // Reset all mics: we are ready for a new round
    GPIO_write(LED_TRIGGER_1, 0);

    if(mic1LTriggered && mic2MTriggered && mic3RTriggered)
    {
        mic1LTriggered = false;
        mic2MTriggered = false;
        mic3RTriggered = false;
        sem_post(&adcbufSem);     /* post adcbuf semaphore */
    }
}

void interruptMic1LTriggered(uint_least8_t index)
{
    mic1LTriggered = true;
    startTimerIfStopped();
    lastTriggerMic1L = getCurrentPreciseTime();
    setNormalMicMode(MIC_LEFT); // Disable interrupt externally
    GPIO_disableInt(MIC1L_D_OUT_INTRPT);
}

void interruptMic2MTriggered(uint_least8_t index)
{
    mic2MTriggered = true;
    startTimerIfStopped();
    lastTriggerMic2M = getCurrentPreciseTime();
    setNormalMicMode(MIC_MID); // Disable interrupt externally
    GPIO_disableInt(MIC2M_D_OUT_INTRPT);
}

void interruptMic3RTriggered(uint_least8_t index)
{
    mic3RTriggered = true;
    startTimerIfStopped();
    lastTriggerMic3R = getCurrentPreciseTime();
    setNormalMicMode(MIC_RIGHT); // Disable interrupt externally
    GPIO_disableInt(MIC3R_D_OUT_INTRPT);

    GPIO_write(LED_TRIGGER_1, 1);
    openADCBuf();
}
