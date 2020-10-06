// Import if you need debug defines to define pins
// #include "defines.h"

#ifndef PINDEFS__H
#define PINDEFS__H

// Pin definition header
// https://randomnerdtutorials.com/esp8266-pinout-reference-gpios/

const int ledPin = LED_BUILTIN;

// Mic analog input
const int analogInPin = A0;
// Mic wake-on-sound trigger inputs
const int mic3TriggerPin = D7; // Mic 'right'
const int mic2TriggerPin = D6; // Mic 'mid' (needs to move into triangle position soon...)
const int micTriggerPin = D5; // Mic 'left'

// Mic mode output bus
// TODO Need to split up instead of driving all mics, might be cause for multiple interrupts?
const int wosModePin = 16; // D0, but define points to 0... so set to GPIO16 directly

#endif // !PINDEFS__H
