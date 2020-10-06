#include "filter.h"

// HPF EMA
// https://www.norwegiancreations.com/2016/03/arduino-tutorial-simple-high-pass-band-pass-and-band-stop-filtering/
float EMA_a = 0.2; //initialization of EMA alpha
int EMA_S = 0;     //initialization of EMA S
int filterValueEMA(int analogValue)
{
    EMA_S = (EMA_a * analogValue) + ((1 - EMA_a) * EMA_S); // run the EMA
    return analogValue;                                    // - EMA_S; // calculate the high-pass signal
}

float EMA_a_low = 0.6; //initialization of EMA alpha
float EMA_a_high = 0.9;
int EMA_S_low = 0; //initialization of EMA S
int EMA_S_high = 0;
int bandpassEMA(int analogValue)
{
    EMA_S_low = (EMA_a_low * analogValue) + ((1 - EMA_a_low) * EMA_S_low); //run the EMA
    EMA_S_high = (EMA_a_high * analogValue) + ((1 - EMA_a_high) * EMA_S_high);
    return EMA_S_high - EMA_S_low; //find the band-pass
}
