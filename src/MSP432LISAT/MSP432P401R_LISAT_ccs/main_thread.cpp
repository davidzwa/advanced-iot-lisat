/* DriverLib Includes */
#include <stdint.h>
#include <stdio.h>
#include <System/periodicKernelTask.h>
#include <unistd.h>

// Bios interaction
#include <ti/sysbios/BIOS.h>
#include <TDOA/signalPreambleDetector.h>

// Our common defines and entrypoint
#include "common.h"
#include "Robot/robot.h"
#include "Robot/bumpers.h"
#include "Robot/irSensors.h"
#include "System/freeRunningTimer.h"
#include "System/highSpeedTimer.h"
#include "DSP/signalGenerator.h"
#include "SpeakerInterface/speakerControl.h"
#include "SerialInterface/serialESPBridge.h"
#include "SerialInterface/serialDebugInterface.h"
#include "TDOA/microphoneLocalization.h"

int32_t buffersCompletedCounter = 0;
uint32_t maxIndex;
uint32_t minIndex;
int16_t minValue;
int16_t maxValue;

const int num_calibs = 10;
int32_t targetSpeed_MMPS[] = {30, 40, 50, 60, 70, 80, 90, 100, 110, 120};
uint32_t duty_LUT[num_calibs];
Robot* robot = new Robot();
uint16_t speed = 0;
timespec ts;
sem_t pressPauseSem;

RobotState robotState = IDLE;
uint32_t currentTime;

void changeMode(RobotState state) { // change between intersection/find each other mode
    if (robotState == IDLE) {
        robotState = state;
    }
}

void changeMotorSpeed(uint16_t newSpeed) {
    speed = newSpeed;
    robot->motorDriver->DriveForwards(speed);
}

void panicStop() { // panic break (bumpers)
    robot->motorDriver->DriveForwards(0);
    speed = 0;
    robotState = IDLE;
}

void turnLeft(float angle) {
    robot->TurnLeft(angle);
}

void turnRight(float angle) {
    robot->TurnRight(angle);
}

void generateSignatureSignals() {
    generateSignatureChirp(tjirp, CHIRP_SAMPLE_COUNT);
    generateSignatureSine(preprocessed_reference_preamble, PREAMBLE_SINE_PERIOD, PREAMBLE_REF_LENGTH);
}

bool awaitAudioByListening() {
    timespec startWaitTime;
    clock_gettime(CLOCK_MONOTONIC, &startWaitTime);
    // Open the driver again, set the short mode and call conversion kickoff
    setAdcBufConversionMode(true);
    convertADCBuf();

    bool returnSet = false;
    bool returnFlag = false;
    while(1) {
        timespec currentTime;
        clock_gettime(CLOCK_MONOTONIC, &currentTime);
#if MIC_CONTINUOUS_SAMPLE == 0
        convertADCBuf();
#endif
        // This should not wait too long (less than 50ms), but just to be sure
        currentTime.tv_sec+=1;
        sem_timedwait(&adcbufSem, &currentTime);

        // Register time for the next part
        if(wasPreambleDetected()) {
            resetPreambleDetectionHistory();
            returnFlag = true;
            returnSet = true;
        }
        else {
#if MSP_ESP_ROBOT_MODE == 1
            clock_gettime(CLOCK_MONOTONIC, &currentTime);
            if (currentTime.tv_sec > startWaitTime.tv_sec + MIC_SOUND_WAITING_SECONDS) {
                returnFlag = false;
                returnSet = true;
            }
#endif
//            setAdcBufConversionMode(true);
        }
#if MSP_ESP_ROBOT_MODE == 1
        if (returnSet) {
//            closeADCBuf();
            return returnFlag;
        }
#endif
    }
}

/*
 *  ======== mainThread ========
 */
void *mainThread(void *arg0)
{
    Display_Params displayParams;
    int32_t status;

    sem_init(&pressPauseSem, 0, 0);

    robot->StartUp();
    robot->motorDriver->DriveForwards(0);
#if MSP_ROBOT_PID_CONTROL == 1
    robot->EnableDriveControl();
#endif
// Some tests/debug things
//    robot->RunTachoCalibrations(targetSpeed_MMPS, duty_LUT, num_calibs);

#if MSP_MIC_MEASUREMENT_PC_MODE==1
    Display_init();
    /* Configure & open Display driver */
    Display_Params_init(&displayParams);
    displayParams.lineClearMode = DISPLAY_CLEAR_BOTH;
    display = Display_open(Display_Type_UART, &displayParams);
    if (display == NULL) {
        Display_printf(display, 0, 0, "Error creating display\n");
        while (1);
    }

    status = sem_init(&adcbufSem, 0, 0);
    if (status != 0) {
        Display_printf(display, 0, 0, "Error creating adcbufSem\n");
        while(1);
    }

    Display_printf(display, 0, 0, "Started MSP UART Display Driver\n");
    initADCBuf();
    openADCBuf();
    setAdcBufConversionMode(true);
    generateSignatureSignals();
#endif

#if BUMPER_INTERRUPTS == 1
    initBumpers();
#endif

#if MSP_SPEAKER_INTERRUPTS == 1
    initSpeakerTaskClock();
#endif

#if MSP_IR_SENSORS == 1
    initHighSpeedTimer(irTimerCallback);
    initLineDetectionSem();
    initIrTaskClock(robot);
#endif

    while(1) {
#if MSP_ESP_ROBOT_MODE==1
        switch(robotState) {
            case IDLE:
                /* Suspend thread and wake up after set period to circumvent semaphore issue */
                clock_gettime(CLOCK_REALTIME, &ts);
                ts.tv_sec += MAIN_THREAD_TIMED_WAIT;
                sem_timedwait(&mqttWakeupSem, &ts);
                break;
            case INTER_DRIVING:
                robot->motorDriver->DriveForwards(STANDARD_FORWARD_SPEED);
#if MSP_IR_SENSORS == 1
                startIrTaskClock();
                listenToLineDetection = true;
                sem_wait(&lineDetectionSem);
                //stopIrTaskClock();
#endif
                robot->Stop();
                robotState = INTER_LISTENING;
                break;
            case INTER_LISTENING:
                bool givePriority = awaitAudioByListening();
                if(givePriority) {
                    currentTime = Clock_getTicks();
                    robotState = INTER_WAITING;
                } else {
                    robotState = INTER_TRANSMITTING;
                }
                break;
            case INTER_WAITING:
                if (Clock_getTicks() - currentTime > LISTEN_WAIT_TIME) {
                    robotState = INTER_LISTENING;
                }
                break;
            case INTER_TRANSMITTING:
#if MSP_SPEAKER_INTERRUPTS == 1
                speakerPressPause();
                clock_gettime(CLOCK_REALTIME, &ts);
                ts.tv_sec += SPEAKER_SOUND_DURATION_SECONDS;
                sem_timedwait(&pressPauseSem, &ts);// basically non blocking wait for wait 4 seconds
                speakerPressPause();
#endif
                robotState = INTER_CROSSING;
                break;
            case INTER_CROSSING:
                robot->motorDriver->DriveForwards(STANDARD_FORWARD_SPEED);
#if MSP_IR_SENSORS == 1
                clock_gettime(CLOCK_REALTIME, &ts);
                /* LINE DETECTION DEBOUNCE, USE PRESS PAUSE SEM SINCE IT WON'T BE TRIGGERED ANYWAY AND NO MEMORY LEFT FOR ANOTHER SEM */
                ts.tv_sec += 2;
                sem_timedwait(&pressPauseSem, &ts);
                listenToLineDetection = true;
                //resetLineDetection();
                //startIrTaskClock();
                sem_wait(&lineDetectionSem);
                stopIrTaskClock();
#endif
                robot->Stop();
                robotState = IDLE;
                break;
            default:
                break;
        }
#else
        // We want to listen to audio constinuously
        awaitAudioByListening();
#endif
    }
}
