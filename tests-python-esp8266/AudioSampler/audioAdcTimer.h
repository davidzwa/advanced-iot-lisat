#include "defines.h"

#ifndef AUDIO_ADC_TIMER__H
#define AUDIO_ADC_TIMER__H

// Source: https://www.switchdoc.com/2015/10/iot-esp8266-timer-tutorial-arduino-ide/
// Optimized timer: https://github.com/esp8266/Arduino/issues/2426
// 2nd source: https://www.esp8266.com/viewtopic.php?f=32&t=16194

void initOsTimer(uint16_t periodUs);
void startOsTimer();
void disarmOsTimer();
void timerCallback(void *pArg);

#endif // !AUDIO_ADC_TIMER__H
