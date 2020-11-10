#ifndef COMMON__H
#define COMMON__H

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <semaphore.h>

#include <ti/devices/msp432p4xx/inc/msp.h>

/* Peripheral Header files */
#include <ti/display/Display.h>
#include <ti/display/DisplayUart.h>
#include <ti/drivers/GPIO.h>
#include <ti/drivers/ADCBuf.h>
#include <ti/drivers/Timer.h>

/* Advanced dsp math */
#include <arm_math.h>
#include <arm_const_structs.h>

#include "ti_drivers_config.h"

// Switch flag to indicate whether the MSP ignores the ESP's signals, and just prints the debugging statements to the PC
#define MSP_MIC_MEASUREMENT_PC_MODE (1)

#define NUM_ADC_CHANNELS    (1)
#define ADCBUFFERSIZE_SHORT (128)
#define ADCBUFFERSIZE       (512)
#define SAMPLE_FREQUENCY    (20000)

/* Robot Bumpers */
#define BUMPER_DEBOUNCE_INTERVAL 1000 // minimum time between consecutive bumper interrupts (in ms)

#endif // COMMON__H
