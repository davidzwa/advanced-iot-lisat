/* DriverLib Includes */
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

// Bios interaction
#include <ti/sysbios/BIOS.h>
#include <TDOA/signalPreambleDetector.h>

// Our common defines and entrypoint
#include "common.h"
#include "Robot/robot.h"
#include "Robot/bumpers.h"
#include "Robot/irSensors.h"
#include "System/kernelSingleTaskClock.h"
#include "System/freeRunningTimer.h"
#include "System/highSpeedTimer.h"
#include "DSP/signalGenerator.h"
#include "SpeakerInterface/speakerControl.h"
#include "SerialInterface/serialESPBridge.h"
#include "SerialInterface/serialDebugInterface.h"
#include "TDOA/microphoneLocalization.h"

// Chirp buffah
int16_t tsjirpBuffah[CHIRP_SAMPLE_COUNT];
#define CORRELATION_LENGTH (2*ADCBUFFERSIZE_SHORT-1)
int16_t correlationChirp[CORRELATION_LENGTH];

int32_t buffersCompletedCounter = 0;
uint32_t maxIndex;
uint32_t minIndex;
int16_t minValue;
int16_t maxValue;

const int num_calibs = 10;
int32_t targetSpeed_MMPS[] = {30, 40, 50, 60, 70, 80, 90, 100, 110, 120};
uint32_t duty_LUT[num_calibs];
Robot* robot = new Robot();
int speed = 500;

RobotState robotState = IDLE;
uint32_t currentTime;

void changeMode(RobotState state) { // change between intersection/find each other mode
    if (robotState == IDLE) {
        robotState = state;
    }
}

void changeMotorSpeed(int newSpeed) {
    speed = newSpeed;
    robot->motorDriver->DriveForwards(speed);
}

void panicStop() { // panic break (bumpers)
    robot->motorDriver->DriveForwards(0);
    speed = 0;
    robotState = IDLE;
}

void generateSignatureSignals() {
    generateSignatureChirp(tsjirpBuffah, CHIRP_SAMPLE_COUNT);
    generateSignatureSine(preprocessed_reference_preamble, PREAMBLE_SINE_PERIOD, PREAMBLE_REF_LENGTH);
}

/*
 *  ======== mainThread ========
 */
void *mainThread(void *arg0)
{
    Display_Params displayParams;
    int32_t status;
    int numBufsSent = 0;

    robot->StartUp();
    robot->motorDriver->DriveForwards(0);

#if MSP_MIC_MEASUREMENT_PC_MODE!=1

   // Some tests/debug things
    //    robot->RunTachoCalibrations(targetSpeed_MMPS, duty_LUT, num_calibs);
#else
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
    generateSignatureSignals();
    openADCBuf();
#endif

#if BUMPER_INTERRUPTS == 1
    initBumpers();
#endif

#if MSP_SPEAKER_INTERRUPTS == 1
    initSpeakerTaskClock();
    startSpeakerTaskClock();
#endif

#if MSP_IR_SENSORS == 1
    initHighSpeedTimer(irTimerCallback);
    initLineDetectionSem();
    initIrTaskClock();
    startIrTaskClock();
#endif
    while(1) {
#if MSP_MIC_MEASUREMENT_PC_MODE!=1
        switch(robotState) {
            case IDLE:
                sem_wait(&mqttWakeupSem); // at this point state can only change when command is received
                break;
            case INTER_DRIVING:
                robot->motorDriver->DriveForwards(speed);
                sem_wait(&lineDetectionSem);
                robot->Stop();
                robotState = INTER_LISTENING;
                break;
            case INTER_LISTENING:
                //todo: LISTEN FOR SET AMOUNT OF TIME
                bool givePriority = false; // todo: remove, just for testing purposes
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
                speakerPlaySound();
                sem_wait(&speakerSoundFinishedSem); // wait for sound to finish playing
                robotState = INTER_CROSSING;
                break;
            case INTER_CROSSING:
                robot->motorDriver->DriveForwards(speed);
                resetLineDetection(); //todo: might have to wait a little before resetting line detection
                sem_wait(&lineDetectionSem);
                robot->Stop();
                robotState = IDLE;
                break;
        }

#else
#if MIC_CONTINUOUS_SAMPLE != 1
        openADCBuf();
#endif
        sem_wait(&adcbufSem);
#if MSP_MIC_RAW_MODE == 1

#else
        arm_min_q15(outputBuffer_filtered, ADCBUFFERSIZE_SHORT, &minValue, &minIndex);
        Display_printf(display, 0, 0, "Mi.%d", minValue);
        arm_max_q15(outputBuffer_filtered, ADCBUFFERSIZE_SHORT, &maxValue, &maxIndex);
        Display_printf(display, 0, 0, "Ma.%d", maxValue);

//        Send RMS value of ADCBuf for either Valin or CTP or algorithm
//        arm_rms_q15(outputBuffer_filtered, ADCBUFFERSIZE_SHORT, &rms);
//        Display_printf(display, 0, 0, "R.%d", rms);

//        if (stupidDetectionBlackBox(outputBuffer_filtered, ADCBUFFERSIZE_SHORT, rms)) {
        if(wasPreambleDetected()) {
            setAdcBufConversionMode(false);
            resetPreambleDetectionHistory();
            GPIO_write(LED_GREEN_2_GPIO, 1);
        }
        else {
            setAdcBufConversionMode(true);
//            openADCBuf();
            GPIO_write(LED_GREEN_2_GPIO, 0);
        }
        //         Decide to send ADC buffer over the line
        //        for (int i = 0; i < ADCBUFFERSIZE; i++) {
        //            Display_printf(display, 0, 0, "v.%d", outputBuffer[i]);
        //        }

                Display_printf(display, 0, 0, "S.%d", ADCBUFFERSIZE_SHORT);
                Display_printf(display, 0, 0, "F.%d", SAMPLE_FREQUENCY);

        //        Send mic time differences
        //        Display_printf(display, 0, 0, "M1.%ld", lastTriggerMic1L);
        //        Display_printf(display, 0, 0, "M2.%ld", lastTriggerMic2M);
        //        Display_printf(display, 0, 0, "M3.%ld", lastTriggerMic3R);
        //        Send DOA values for either Valin or CTP or algorithm
        //        Display_printf(display, 0, 0, "Dv1.%f", outputDirVector2D_valin[0]);
        //        Display_printf(display, 0, 0, "Dv2.%f", outputDirVector2D_valin[1]);
        //        Display_printf(display, 0, 0, "Dp1.%f", outputDirVector2D_plane_cutting[0]);
        //        Display_printf(display, 0, 0, "Dp2.%f", outputDirVector2D_plane_cutting[1]);
#endif
        Display_printf(display, 0, 0, "--Done");
        numBufsSent++;
#endif
    }
}
