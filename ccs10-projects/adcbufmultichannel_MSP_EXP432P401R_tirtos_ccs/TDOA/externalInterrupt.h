#include "common.h"

#ifndef TDOA_EXTERNALINTERRUPT_H_
#define TDOA_EXTERNALINTERRUPT_H_

enum MIC
{
    MIC_LEFT = MIC1L_MODE_WOS,
    MIC_MID = MIC2M_MODE_WOS,
    MIC_RIGHT = MIC3R_MODE_WOS
};

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
//int16_t correlation[CHUNK_LENGTH];
int16_t outputBuffer[ADCBUFFERSIZE];
int16_t outputBuffer_filtered[ADCBUFFERSIZE];
int16_t rms;

/* ADCBuf semaphore */
sem_t adcbufSem;

extern bool startAdcSampling;
extern unsigned long lastTriggerMic1;
extern unsigned long lastTriggerMic2;
extern unsigned long lastTriggerMic3;

// CASPER's TDOA PLAYGROUND
long getCurrentPreciseTime(); // Timer difference method
void resetWosMicMode();
void setWosMode(MIC micType);
void setNormalMicMode(MIC micType);

void initADCBuf();
void testADCBufOpened();
void adcBufCallback(ADCBuf_Handle handle, ADCBuf_Conversion *conversion,
                    void *completedADCBuffer, uint32_t completedChannel);
void enableMicTriggerInterrupts();
void disableMicTriggerInterrupts();
void interruptMic1LTriggered(uint_least8_t index);
void interruptMic2MTriggered(uint_least8_t index);
void interruptMic3RTriggered(uint_least8_t index);

#endif /* TDOA_EXTERNALINTERRUPT_H_ */
