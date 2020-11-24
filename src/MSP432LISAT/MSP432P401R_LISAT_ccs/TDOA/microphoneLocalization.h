#include <common.h>
#include <DSP/filter.h>

#ifndef TDOA_EXTERNALINTERRUPT_H_
#define TDOA_EXTERNALINTERRUPT_H_

int16_t tjirp[CHIRP_SAMPLE_COUNT];

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
int16_t outputBuffer_filtered[ADCBUFFERSIZE];

uint32_t detection_history_mics[3];
void resetPreambleDetectionHistory();
bool wasPreambleDetected();

/* ADCBuf semaphore */
sem_t adcbufSem;
extern int lastChannelCompleted;
extern bool startAdcSampling;
extern int16_t rms;

void initADCBuf();
void openADCBuf();
void convertADCBuf();
void closeADCBuf();
void setAdcBufConversionMode(bool shortConversion);

#endif /* TDOA_EXTERNALINTERRUPT_H_ */
