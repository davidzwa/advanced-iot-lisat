#include <Esp.h>

#ifndef DEFINES__H
#define DEFINES__H

// Runtime debug definitions (do we print over 'expensive' Serial)
// #define NO_BUFFER // Direct stream values
// #define STREAM_PLOTTER // Adjust output so Arduino's plotter likes it, otherwise go for Python interface
// #define PRINT_TRIGGER_INTERRUPTS
// #define DEBUG
// #define MEASURE_ADCTIMER_JITTER
// #define MQTT_CLIENT

// Arduino has 1000000 or even more unstable 2000000
const int serial_baud_rate = 250000; // VS Code has max 250000 -_-

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

#endif // !DEFINES__H