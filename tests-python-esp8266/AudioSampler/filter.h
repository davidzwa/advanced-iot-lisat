#ifndef FILTER__H
#define FILTER__H

#include <Arduino.h>

int bandpassEMA(int analogValue);
int filterValueEMA(int analogValue);

#endif // !FILTER__H
