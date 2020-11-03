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
#include "TDOA/microphoneLocalization.h""

//#include "DSP/fft.h"
//#include "DSP/iirFilter.h"

/* DSP LPF Filter */
//IirFilter* filter;

int32_t buffersCompletedCounter = 0;

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

    /* Call driver init functions */
    initUARTESP();
    openUARTESP();
    initADCBuf();
    initTimerTacho();

//    Display_init();
//    /* Configure & open Display driver */
//    Display_Params_init(&displayParams);
//    displayParams.lineClearMode = DISPLAY_CLEAR_BOTH;
//    display = Display_open(Display_Type_UART, &displayParams);
//    if (display == NULL) {
//        Display_printf(display, 0, 0, "Error creating display\n");
//        while (1);
//    }
//
//    status = sem_init(&adcbufSem, 0, 0);
//    if (status != 0) {
//        Display_printf(display, 0, 0, "Error creating adcbufSem\n");
//        while(1);
//    }
  
//    resetWosMicMode(); // Override each mode pin to be HIGH (just to be sure)
//    initInterruptCallbacks();
//    enableMicTriggerInterrupts();
  
    int numBufsSent = 0;
    robot->StartUp();
//    robot->RunTachoCalibrations(targetSpeed_MMPS, duty_LUT, num_calibs);

//    writeUARTInfinite(); // BLOCKING for testing

    int speed = 1000;
    robot->motorDriver->DriveForwards(speed);
    while(1) {
        //        sem_wait(&adcbufSem);
        usleep(50000);
        robot->UpdateRobotPosition();

//        speed += 10;
//        if (speed > 4500) {
//            speed = 1000;
//        }

        /*
         * Start with a header message and print current buffer values
         */
//        Display_printf(display, 0, 0, "\r\n", //Buffer %u finished:
//            (unsigned int)buffersCompletedCounter++);

         //filter->FilterBuffer(outputBuffer, outputBuffer_filtered);
        //filter->FilterEMABuffer(outputBuffer, outputBuffer_filtered);

//        doFFT(outputBuffer_filtered, fftOutput);
//        arm_correlate_q15(outputBuffer_filtered, CHUNK_LENGTH, audioVector, AUDIO_LENGTH, correlation);

//        for (i = 0; i < ADCBUFFERSIZE; i++) {
//            Display_printf(display, 0, 0, "v.%d", outputBuffer_filtered[i]);

//        arm_rms_q15(outputBuffer, ADCBUFFERSIZE, &rms);
//
//        Display_printf(display, 0, 0, "Dv1.%f", outputDirVector2D_valin[0]);
//        Display_printf(display, 0, 0, "Dv2.%f", outputDirVector2D_valin[1]);
//        Display_printf(display, 0, 0, "Dp1.%f", outputDirVector2D_plane_cutting[0]);
//        Display_printf(display, 0, 0, "Dp2.%f", outputDirVector2D_plane_cutting[1]);
//        Display_printf(display, 0, 0, "R.%d", rms);
//        if (rms > 0) {
//
//        }
//        int16_t maxValue;
//        uint32_t maxIndex;
//        arm_max_q15(outputBuffer, ADCBUFFERSIZE, &maxValue, &maxIndex);

//        outputBuffer_filtered[]
//        Display_printf(display, 0, 0, "%d", maxValue);
//        Display_printf(display, 0, 0, "Sem++");
//        numBufsSent++;
    }
}
