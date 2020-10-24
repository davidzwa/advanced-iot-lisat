#include "defines.h"

#ifndef PINDEFS__H
#define PINDEFS__H

// Pin definition header
// https://randomnerdtutorials.com/esp8266-pinout-reference-gpios/

const int ledPin = LED_BUILTIN;

#ifdef MIC3_MEASURE_SETUP
// Mic analog input
const int analogInPin = A0;
// Mic wake-on-sound trigger inputs
const int mic1LTriggerPin = D5; // Mic 'left'
const int mic2MTriggerPin = D6; // Mic 'mid' (needs to move into triangle position soon...)
const int mic3RTriggerPin = D7; // Mic 'right' also on AnalogInput

// Mic mode output bus
// TODO Need to split up instead of driving all mics, might be cause for multiple interrupts?
const int wosModePin1L = 5; // D1
const int wosModePin2M = 16; // D0
const int wosModePin3R = 4; // D2
#endif

#endif // !PINDEFS__H
