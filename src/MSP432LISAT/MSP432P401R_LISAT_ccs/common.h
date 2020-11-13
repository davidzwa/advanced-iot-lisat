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

enum MIC
{
    MIC_LEFT = MIC1L_MODE_WOS,
    MIC_MID = MIC2M_MODE_WOS,
    MIC_RIGHT = MIC3R_MODE_WOS
};

// Short buffer EMA BPF filter
#define EMA_A_LOW (0.8f)
#define EMA_A_HIGH (0.9f)

// Signature signal generator properties
const double chirpInterval = 2.0;       // ms
const double chirpFrequencyStart = 5.0; // kHz
const double chirpFrequencyEnd = 15.0;  // kHz
#define CHIRP_SAMPLE_COUNT (88)         // Count

// Switch flag to indicate whether the MSP ignores the ESP's signals, and just prints the debugging statements to the PC
#define MSP_MIC_MEASUREMENT_PC_MODE (1)
// Switch flag to indicate whether MSP handles speakers commands  (bumper interrupts only works in robot mode)
#define MSP_SPEAKER_INTERRUPTS (1)
#define MIC_CONTINUOUS_SAMPLE (1) // If not the main_thread will have to kick it when it can to continue.
#define MSP_SPEAKER_INTERRUPTS (0)
// Switch flag to indicate whether IR Sensors with high speed timer interrupts are active
#define MSP_IR_SENSORS (1)

#define NUM_ADC_CHANNELS    (3)
#define ADCBUFFERSIZE_SHORT (64)
#define ADCBUFFERSIZE       (512)
#define SAMPLE_FREQUENCY    (44000)

/* Speakers */
//currently in ms, depends on kernel clock config
#define SPEAKER_CLOCK_INITIAL_OFFSET    4000 // needs to be larger than 0 for clock to start
#define SPEAKER_CLOCK_PERIOD_SOUND      4000 // duration before rewind button is pressed
#define SPEAKER_CLOCK_PERIOD_BUTTON     200

/* IR sensors */
#define IRSENSORS_CLOCK_INITIAL_OFFSET  100    // needs to be larger than 0 for clock to start
#define IRSENSORS_CLOCK_PERIOD          100

/* Robot Bumpers */
#define BUMPER_DEBOUNCE_INTERVAL 1000 // minimum time between consecutive bumper interrupts (in ms)

#endif // COMMON__H
