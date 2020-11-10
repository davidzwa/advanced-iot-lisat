/* DriverLib Includes */
#include <stdint.h>
#include <stdio.h>
#include <SerialInterface/serialDebugInterface.h>
#include <unistd.h>

// Our common defines and entrypoint
#include "common.h"
#include "Robot/robot.h"
#include "SerialInterface/serialESPBridge.h"
#include "System/freeRunningTimer.h"
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

/*
 *  ======== mainThread ========
 */
void *mainThread(void *arg0)
{
    Display_Params displayParams;
    int32_t status;
    int numBufsSent = 0;

#if MSP_MIC_MEASUREMENT_PC_MODE!=1
    int speed = 1000;

    initUARTESP();
    openUARTESP();
    initTimerTacho();

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
    initTimerTacho();
    resetWosMicMode(); // Override each mode pin to be HIGH (just to be sure)
//    initInterruptCallbacks();
//    enableMicTriggerInterrupts();
#endif

    while(1) {
#if MSP_MIC_MEASUREMENT_PC_MODE!=1
        usleep(50000);
        robot->UpdateRobotPosition();
        //        speed += 10;
        //        if (speed > 4500) {
        //            speed = 1000;
        //        }
#else
        openADCBuf();
        sem_wait(&adcbufSem);

        arm_min_q15(outputBuffer_filtered, ADCBUFFERSIZE_SHORT, &minValue, &minIndex);
        Display_printf(display, 0, 0, "Mi.%d", minValue);
        arm_max_q15(outputBuffer_filtered, ADCBUFFERSIZE_SHORT, &maxValue, &maxIndex);
        Display_printf(display, 0, 0, "Ma.%d", maxValue);

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

//        Send RMS value of ADCBuf for either Valin or CTP or algorithm
        arm_rms_q15(outputBuffer_filtered, ADCBUFFERSIZE_SHORT, &rms);
        Display_printf(display, 0, 0, "R.%d", rms);
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
