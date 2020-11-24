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

/* Semaphore to signal main thread that speaker sound has finished playing */
extern sem_t speakerSoundFinishedSem;
extern sem_t mqttWakeupSem;
extern sem_t lineDetectionSem;

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

void changeMode(RobotState state);
void changeMotorSpeed(uint16_t speed);
void panicStop();
void turnLeft(float angle);
void turnRight(float angle);

bool listenToLineDetection;


#define NUM_ADC_CHANNELS        (3)
#define CARRIER_FREQUENCY       (3000)
#define TARGET_FREQUENCY        (33000)
#define SAMPLE_FREQUENCY        (NUM_ADC_CHANNELS*TARGET_FREQUENCY)
#define PREAMBLE_SINE_PERIOD    (TARGET_FREQUENCY/CARRIER_FREQUENCY) // 22 (@44 kHz)
#if (PREAMBLE_SINE_PERIOD*CARRIER_FREQUENCY) != TARGET_FREQUENCY
#error Preamble length should be an integer number.
#endif
#define ADCBUFFERSIZE_SHORT     (PREAMBLE_SINE_PERIOD * 3) // 22*3 = 66 samples

// Non-preamble signature signal generator properties
// MAX: 3000 (ONE-SHOT) 1700 (CONTINUOUS)
#define ADCBUFFERSIZE           1600
#define CARRIER_SHIFT           (5) // Avoid (partial corr.) dot product overflow, bit-shift by this amount
#define CHIRP_SHIFT             (4)
#define NUM_CHIRPS              (1)
#define CHIRP_LENGTH            (0.030) // ms
#define CHIRP_FREQ_START_KHZ    (500)  // kHz
#define CHIRP_FREQ_END_KHZ      (3500)  // kHz
#define CHIRP_SAMPLE_COUNT      (uint32_t)(CHIRP_LENGTH*TARGET_FREQUENCY)
#define PREAMBLE_REF_LENGTH     (ADCBUFFERSIZE_SHORT + PREAMBLE_SINE_PERIOD/2) // Contains multiple sines 'VDHorst optimization'
#define MATCH_THRESHOLD         (3000) // Each signal correlator threshold 'round'
#define HISTORY_LENGTH          (20) // Total number of 'rounds'
#define DETECTION_THRESHOLD     (15) // Enough matching 'rounds'

// EMA BPF filter
#define EMA_A_LOW               (0.5f)
#define EMA_A_HIGH              (0.9f)

// Switch flag to indicate whether the MSP ignores the ESP's signals, and just prints the debugging statements to the PC
#define MSP_ESP_ROBOT_MODE          (1)
#define MSP_MIC_MEASUREMENT_PC_MODE (1)
#define MSP_MIC_RAW_MODE            (0) // RAW mode disables filters, detectors etc
#define MIC_CONTINUOUS_SAMPLE       (1) // If not the main_thread will have to kick it when it can to continue.
#define MIC_SOUND_WAITING_SECONDS   (3)    // used for timedwait in main, in seconds

// Switch flag to indicate whether bumper interrupts are enabled
#define BUMPER_INTERRUPTS (1)
// Switch flag to indicate whether MSP handles speakers commands
#define MSP_SPEAKER_INTERRUPTS (1)
// Switch flag to indicate whether IR Sensors with high speed timer interrupts are active
#define MSP_IR_SENSORS (1)
// Flag indicating a task which will counter-act any driving offsets
#define MSP_ROBOT_PID_CONTROL (1)

/* Speakers */
//currently in ms, depends on kernel clock config
#define SPEAKER_CLOCK_INITIAL_OFFSET    4000 // needs to be larger than 0 for clock to start
#define SPEAKER_CLOCK_PERIOD_SOUND      4000 // duration before rewind button is pressed
#define SPEAKER_CLOCK_PERIOD_BUTTON     200
#define SPEAKER_SOUND_DURATION_SECONDS  8    // used for timedwait in main, in seconds

/* IR sensors */
#define IRSENSORS_CLOCK_INITIAL_OFFSET  100     // needs to be larger than 0 for clock to start
#define IRSENSORS_CLOCK_PERIOD          100
#define LINE_DETECTION_THRESHOLD        6       // number of IR sensors that should observe a black surface to consider a line to be detected
#define LINE_DETECTION_DEBOUNCE         1000    // time after which line detection is reactivated after detecting line (in ms)

/* Robot Bumpers */
#define BUMPER_DEBOUNCE_INTERVAL 500 // minimum time between consecutive bumper interrupts (in ms)

/* State machine */
#define MAIN_THREAD_TIMED_WAIT 1 // Time after which main thread wakes up when waiting for mode (intersection/find each other) switch command (in SECONDS)
#define LISTEN_WAIT_TIME 5000 // Time robot waits for other robot to cross intersection (in ms)

/* Robot control */
#define CONTROL_LOOP_INITIAL_OFFSET 1000
#define CONTROL_LOOP_PERIOD 100 // 10 Hz (currently kernel has 1000 ticks per second)

#define STANDARD_FORWARD_SPEED 500

#endif // COMMON__H
