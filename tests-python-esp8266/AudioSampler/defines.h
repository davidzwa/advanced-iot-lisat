#ifndef DEFINES__H
#define DEFINES__H

// WiFi/AP config ESP
#include <ESP8266WiFi.h>            // Include the Wi-Fi library
const char *ssid = "Brus";          // The SSID (name) of the Wi-Fi network you want to connect to
const char *password = "Paaswoord"; // The password of the Wi-Fi network

// Runtime debug definitions (do we print over 'expensive' Serial)
// #define NO_BUFFER
// #define PRINT_TRIGGER_INTERRUPTS
// #define DEBUG
// #define MEASURE_ADCTIMER_JITTER

// Arduino has 1000000 or even more unstable 2000000
const int serial_baud_rate = 250000; // VS Code has max 250000 -_-
#define timeSecondsMs 50

// Timer jitter tracker (microseconds)
#ifdef MEASURE_ADCTIMER_JITTER
long avg_jitter_us = 0;
#endif

#ifdef DEBUG
// Slow mode sampling and small size buffer so Serial can keep up
const int sampling_period_us = 50000; // 50ms, 20 Hz;
#define ADC_SAMPLES_COUNT 100
#else
const int sampling_period_us = 500; // 200us, 5 kHz;
#define ADC_SAMPLES_COUNT 1000
#endif

#endif // !DEFINES__H
