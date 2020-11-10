#include <common.h>
#include <DSP/filter.h>

#ifndef TDOA_EXTERNALINTERRUPT_H_
#define TDOA_EXTERNALINTERRUPT_H_

enum MIC
{
    MIC_LEFT = MIC1L_MODE_WOS,
    MIC_MID = MIC2M_MODE_WOS,
    MIC_RIGHT = MIC3R_MODE_WOS
};

int16_t sampleBuffer1a[ADCBUFFERSIZE]; // This is for long and short mode
int16_t sampleBuffer1b[ADCBUFFERSIZE];
#if NUM_ADC_CHANNELS >= 2
int16_t sampleBuffer2a[ADCBUFFERSIZE];
int16_t sampleBuffer2b[ADCBUFFERSIZE];
#endif
#if NUM_ADC_CHANNELS >= 3
int16_t sampleBuffer3a[ADCBUFFERSIZE];
int16_t sampleBuffer3b[ADCBUFFERSIZE];
#endif
int16_t outputBuffer[ADCBUFFERSIZE];
int16_t outputBuffer_filtered[ADCBUFFERSIZE];
int16_t rms;
int16_t rms_filt;

/* ADCBuf semaphore */
sem_t adcbufSem;

extern bool startAdcSampling;
extern unsigned long lastTriggerMic1L;
extern unsigned long lastTriggerMic2M;
extern unsigned long lastTriggerMic3R;

// CASPER's TDOA PLAYGROUND

float outputDirVector2D_plane_cutting[2];
float outputDirVector2D_valin[2];

uint32_t getCurrentPreciseTime(); // Timer difference method
void resetWosMicMode();
void setWosMode(MIC micType);
void setNormalMicMode(MIC micType);

void initADCBuf();
void openADCBuf();
void adcBufCallback(ADCBuf_Handle handle, ADCBuf_Conversion *conversion,
                    void *completedADCBuffer, uint32_t completedChannel);
void initInterruptCallbacks();
void enableMicTriggerInterrupts();
void disableMicTriggerInterrupts();
void interruptMic1LTriggered(uint_least8_t index);
void interruptMic2MTriggered(uint_least8_t index);
void interruptMic3RTriggered(uint_least8_t index);

#endif /* TDOA_EXTERNALINTERRUPT_H_ */
