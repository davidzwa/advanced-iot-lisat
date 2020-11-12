/* DriverLib Includes */
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

// Bios interaction
#include <ti/sysbios/BIOS.h>

// Our common defines and entrypoint
#include "common.h"
#include "Robot/robot.h"
#include "SerialInterface/serialESPBridge.h"
#include <SerialInterface/serialDebugInterface.h>
#include "System/freeRunningTimer.h"
#include "System/kernelSingleTaskClock.h"
#include "TDOA/microphoneLocalization.h"
#include "TDOA/signalDetection.h"
#include <DSP/signalGenerator.h>
#include "SpeakerInterface/speakerControl.h"

// Chirp buffah
int16_t tsjirpBuffah[CHIRP_SAMPLE_COUNT];
#define CORRELATION_LENGTH (2*ADCBUFFERSIZE_SHORT-1)
int16_t correlationChirp[CORRELATION_LENGTH];

int32_t buffersCompletedCounter = 0;
uint32_t maxIndex;
uint32_t minIndex;
int16_t minValue;
int16_t maxValue;

// Bumper tasks
KernelSingleTaskClock* singleBumperTask = new KernelSingleTaskClock();

const int num_calibs = 10;
int32_t targetSpeed_MMPS[] = {30, 40, 50, 60, 70, 80, 90, 100, 110, 120};
uint32_t duty_LUT[num_calibs];
Robot* robot = new Robot();
int speed = 2000;

void generateAndTransmitSignatureSignal() {
    generateSignatureChirp(tsjirpBuffah, CHIRP_SAMPLE_COUNT);
    Display_printf(display, 0, 0, "BEGIN");
    for (int i = 0; i < CHIRP_SAMPLE_COUNT; i++) {
        Display_printf(display, 0, 0, "%d", tsjirpBuffah[i]);
    }
    Display_printf(display, 0, 0, "END");
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
    robot->motorDriver->DriveForwards(speed);

#if MSP_MIC_MEASUREMENT_PC_MODE!=1
    initUARTESP();
    openUARTESP();

    robot->StartUp();
    robot->motorDriver->DriveForwards(speed);

   // Some tests/debug things
    //    robot->RunTachoCalibrations(targetSpeed_MMPS, duty_LUT, num_calibs);
    //    writeUARTInfinite(); // BLOCKING for testing
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
    generateAndTransmitSignatureSignal();
    openADCBuf();
#endif

#if MSP_SPEAKER_INTERRUPTS != 1
    singleBumperTask->setupClockHandler(200);
    /* Pass clock handle to speaker control for reference (start/stop) */
    attachSpeakerTaskClockHandle(singleBumperTask->getClockHandle());
#endif

    while(1) {
#if MSP_MIC_MEASUREMENT_PC_MODE!=1
        robot->UpdateRobotPosition();
        sem_wait(&adcbufSem);
        //        speed += 10;
        //        if (speed > 4500) {
        //            speed = 1000;
        //        }
#else
        sem_wait(&adcbufSem);

        // 2 * max(srcALen, srcBLen) - 1
//        arm_correlate_q15(outputBuffer_filtered, ADCBUFFERSIZE_SHORT, tsjirpBuffah, CHIRP_SAMPLE_COUNT, correlationChirp);
//        Display_printf(display, 0, 0, "BEGIN-CORR");
//        for (int i = 0; i < CORRELATION_LENGTH; i++) {
//            Display_printf(display, 0, 0, "%d", correlationChirp[i]);
//        }
//        Display_printf(display, 0, 0, "END-CORR");

        arm_min_q15(outputBuffer_filtered, ADCBUFFERSIZE_SHORT, &minValue, &minIndex);
        Display_printf(display, 0, 0, "Mi.%d", minValue);
        arm_max_q15(outputBuffer_filtered, ADCBUFFERSIZE_SHORT, &maxValue, &maxIndex);
        Display_printf(display, 0, 0, "Ma.%d", maxValue);

//        Send RMS value of ADCBuf for either Valin or CTP or algorithm
//        arm_rms_q15(outputBuffer_filtered, ADCBUFFERSIZE_SHORT, &rms);
        Display_printf(display, 0, 0, "R.%d", rms);

        if (StupidDetectionBlackBox(outputBuffer_filtered, ADCBUFFERSIZE_SHORT, rms)) {
            setAdcBufConversionMode(false);
            openADCBuf();
            GPIO_write(LED_GREEN_2_GPIO, 1);
        }
        else {
            setAdcBufConversionMode(true);
            openADCBuf();
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

        Display_printf(display, 0, 0, "--Done");
        numBufsSent++;
#endif
    }

//      Old and useful math operations
//    void DoMathOps()
//    {
//     Choose whether to filter or not, filterBuffer can be quite heavy, EMA is very light
//        filter->FilterBuffer(outputBuffer, outputBuffer_filtered);
//
//        int16_t maxValue;
//        uint32_t maxIndex;
//        arm_max_q15(outputBuffer, ADCBUFFERSIZE, &maxValue, &maxIndex);
//        arm_min_q15(outputBuffer, ADCBUFFERSIZE, &minValue, &minIndex)
//        Display_printf(display, 0, 0, "%d", maxValue);
//        doFFT(outputBuffer_filtered, fftOutput);
//        arm_correlate_q15(outputBuffer_filtered, CHUNK_LENGTH, audioVector, AUDIO_LENGTH, correlation);
//    }
}
