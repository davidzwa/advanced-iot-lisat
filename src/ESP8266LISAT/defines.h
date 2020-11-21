#include <Esp.h>

#ifndef DEFINES__H
#define DEFINES__H

// #define MIC3_MEASURE_SETUP // Not having this will override anything below except MQTT_CLIENT

// Runtime debug definitions (do we print over 'expensive' Serial)
// #define NO_BUFFER // Direct stream values
// #define STREAM_PLOTTER // Adjust output so Arduino's plotter likes it, otherwise go for Python interface
// #define PRINT_TRIGGER_INTERRUPTS
// #define DEBUG
// #define MEASURE_ADCTIMER_JITTER

#define SERIAL_SWAP_MSP432 // Swaps serial and disables serial debug
#define MQTT_CLIENT
#define MQTT_DISCONNECT_RETRY 5000
// Arduino has 1000000 or even more unstable 2000000
const int serial_baud_rate = 115200; // MSP432 set to this, VS Code has max 250000 -_-

#ifdef MIC3_MEASURE_SETUP
#ifdef DEBUG
// Slow mode sampling and small size buffer so Serial can keep up
const int sampling_period_us = 50000; // 50ms, 20 Hz;
#define ADC_SAMPLES_COUNT 100
#else
const int sampling_period_us = 250; // 250us, 4 kHz;
#define ADC_SAMPLES_COUNT 250
#endif

// Timer jitter tracker (microseconds)
#ifdef MEASURE_ADCTIMER_JITTER
long avg_jitter_us = 0;
#endif
#endif

#endif // !DEFINES__H
