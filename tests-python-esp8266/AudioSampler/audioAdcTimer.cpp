#include "audioAdcTimer.h"
#include "pindefs.h"
#include "filter.h"

#ifdef MIC3_MEASURE_SETUP
extern "C"
{
#define USE_US_TIMER
#include "user_interface.h"
#include "osapi.h"
#define os_timer_arm_us(a, b, c) ets_timer_arm_new(a, b, c, 0)
#define os_timer_disarm(a) ets_timer_disarm(a)
}

// Should reduce this set of variables, as most ar old
// long tickOccured = 0;
// bool averaging = false;
// int numAveragedSamples = 0;
// float averagedAmplitude = 0.0f;
// long integratedAmplitude = 0;
// int maxAmplitude = 0;
// int minAmplitude = 100024;
//     numAveragedSamples = 0;
//     averagedAmplitude = 0.0f;
//     integratedAmplitude = 0;
//     maxAmplitude = 0;
//     minAmplitude = 10024;

int16_t bufPosition = 0;
int timerPeriodUs = 1000;
os_timer_t samplingTimerUs;
int analogBuffer[ADC_SAMPLES_COUNT];

void (*timerDoneCallback)(void);

void initOsTimer(uint16_t periodUs)
{
    timerPeriodUs = periodUs;
    system_timer_reinit();
    os_timer_setfn(&samplingTimerUs, timerCallback, analogBuffer);
}

void startOsTimer(void(*finishingTask)(void))
{
    timerDoneCallback = finishingTask;
    os_timer_arm_us(&samplingTimerUs, timerPeriodUs, true);
}

void stopOsTimer()
{
    os_timer_disarm(&samplingTimerUs);
}

void timerCallback(void *pArg)
{
#ifdef MEASURE_ADCTIMER_JITTER
    // Calculate jitter average (IIR filter)
    // Serial.println(abs(micros() - lastMicros - sampling_period_us)); // Steady 200 - 500 us jitter
    // avg_jitter_us = (avg_jitter_us * (bufPosition) + abs(micros() - lastMicros - sampling_period_us)) / (bufPosition + 1); // Resort to jitter calc
#endif
    if (bufPosition < ADC_SAMPLES_COUNT)
    {
        int16_t value = analogRead(analogInPin);
        analogBuffer[bufPosition] = bandpassEMA(value);
        bufPosition++;
    }
    else {
        // Finished, report back
        stopOsTimer();
        bufPosition = 0;
        timerDoneCallback();
        // throw std::overflow_error("Buffer overflow");
    }
}

#endif
