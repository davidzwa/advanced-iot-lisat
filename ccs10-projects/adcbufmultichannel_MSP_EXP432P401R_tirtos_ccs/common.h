#ifndef COMMON__H
#define COMMON__H

#include <stdio.h>
#include <semaphore.h>

#include <ti/devices/msp432p4xx/inc/msp.h>

/* Peripheral Header files */
#include <ti/display/Display.h>
#include <ti/display/DisplayUart.h>
#include <ti/drivers/GPIO.h>
#include <ti/drivers/ADCBuf.h>

/* Advanced dsp math */
#include <arm_math.h>
#include <arm_const_structs.h>

#include "ti_drivers_config.h"

#define NUM_ADC_CHANNELS (1)
#define CHUNK_LENGTH     512
#define ADCBUFFERSIZE    (CHUNK_LENGTH)
#define SAMPLE_FREQUENCY (10000)

#endif // COMMON__H
