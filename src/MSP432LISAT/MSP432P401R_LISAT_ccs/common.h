#ifndef COMMON__H
#define COMMON__H

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <semaphore.h>
/* Advanced dsp math */
#include <arm_math.h>
#include <arm_const_structs.h>

#include "ti_drivers_config.h"
#include <ti/devices/msp432p4xx/inc/msp.h>

/* Peripheral Header files */
#include <ti/display/Display.h>
#include <ti/display/DisplayUart.h>
#include <ti/drivers/GPIO.h>
#include <ti/drivers/ADCBuf.h>
#include <ti/drivers/Timer.h>

enum MIC
{
    MIC_LEFT = MIC1L_MODE_WOS,
    MIC_MID = MIC2M_MODE_WOS,
    MIC_RIGHT = MIC3R_MODE_WOS
};

typedef enum {
    IDLE,
    INTER_DRIVING,
    INTER_LISTENING,
    INTER_WAITING,
    INTER_TRANSMITTING,
    INTER_CROSSING,
    FIND_WAITING,
    FIND_TRANSMITTING,
    FIND_LISTENING,
    FIND_DRIVING,
    NUM_STATES
} RobotState;

typedef enum {
    CHANGE_MODE,
    CHANGE_SPEED,
    STOP,
    NUM_MESSAGE_TYPES
} MessageType;

void changeState(RobotState state);
void changeMotorSpeed(int speed);

#define NUM_ADC_CHANNELS        (3)
#define CARRIER_FREQUENCY       (3000) // Choose this to get integer number for PREAMBLE_LENGTH, which is validated. So be careful!
#define CARRIER_SHIFT           (5) // Avoid (partial corr.) dot product overflow, bit-shift by this amount
#define TARGET_FREQUENCY        (39000)
#define SAMPLE_FREQUENCY        (NUM_ADC_CHANNELS*TARGET_FREQUENCY)
#define PREAMBLE_SINE_PERIOD    (TARGET_FREQUENCY/CARRIER_FREQUENCY) // 22 (@44 kHz)
#if (PREAMBLE_SINE_PERIOD*CARRIER_FREQUENCY) != TARGET_FREQUENCY
#error Preamble length should be an integer number.
#endif
#define ADCBUFFERSIZE_SHORT (PREAMBLE_SINE_PERIOD * 3) // 22*3 = 66 samples

#define PREAMBLE_REF_LENGTH (ADCBUFFERSIZE_SHORT + PREAMBLE_SINE_PERIOD/2) // Contains multiple sines 'VDHorst optimization'
#define MATCH_THRESHOLD (3000) // Each signal correlator threshold 'round'
#define HISTORY_LENGTH (20)
#define DETECTION_THRESHOLD (15) // Enough matching 'rounds'

// Non-preamble signature signal generator properties
#define NUM_CHIRPS (2)
#define ADCBUFFERSIZE 1700 // MAX: 3000 (ONE-SHOT) 1700 (CONTINUOUS)     ((NUM_CHIRPS-1)*CHIRP_SILENCE + )
const double chirpInterval = 17.0;       // ms
const double chirpFrequencyStart = 1.0; // kHz
const double chirpFrequencyEnd = 2.0;  // kHz
#define CHIRP_SAMPLE_COUNT (88)         // Count

// Short buffer EMA BPF filter
#define EMA_A_LOW (0.5f)
#define EMA_A_HIGH (0.9f)

// Switch flag to indicate whether the MSP ignores the ESP's signals, and just prints the debugging statements to the PC
#define MSP_MIC_MEASUREMENT_PC_MODE (0)
#define MSP_MIC_RAW_MODE (0) // RAW mode disables filters, detectors etc
#define MIC_CONTINUOUS_SAMPLE (0) // If not the main_thread will have to kick it when it can to continue.
// Switch flag to indicate whether MSP handles speakers commands  (bumper interrupts only works in robot mode)
#define MSP_SPEAKER_INTERRUPTS (1)
// Switch flag to indicate whether IR Sensors with high speed timer interrupts are active
#define MSP_IR_SENSORS (0)

/* Speakers */
//currently in ms, depends on kernel clock config
#define SPEAKER_CLOCK_INITIAL_OFFSET    4000 // needs to be larger than 0 for clock to start
#define SPEAKER_CLOCK_PERIOD_SOUND      4000 // duration before rewind button is pressed
#define SPEAKER_CLOCK_PERIOD_BUTTON     200

/* IR sensors */
#define IRSENSORS_CLOCK_INITIAL_OFFSET  100 // needs to be larger than 0 for clock to start
#define IRSENSORS_CLOCK_PERIOD          100
#define LINE_DETECTION_THRESHOLD        6   // number of IR sensors that should observe a black surface to consider a line to be detected

/* Robot Bumpers */
#define BUMPER_DEBOUNCE_INTERVAL 1000 // minimum time between consecutive bumper interrupts (in ms)

#define LISTEN_WAIT_TIME 5000 // Time robot waits for other robot to cross intersection (in ms)

#endif // COMMON__H
