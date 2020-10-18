/* DriverLib Includes */
#include <DSP/fft.h>
#include <DSP/iirFilter.h>
#include <Robot/Motor.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

// Our common defines and entrypoint
#include "common.h"
#include "TDOA/externalInterrupt.h"
#include "TDOA/diffTimer.h"
#include "SerialInterface/serialInterface.h"
#include "SerialInterface/UART_esp.h"

// Extra data file
#include "corrAudio.h"

/* DSP LPF Filter */
IirFilter* filter;

int32_t buffersCompletedCounter = 0;

/*
 *  ======== mainThread ========
 */
void *mainThread(void *arg0)
{
    Display_Params displayParams;
    int32_t status;

    /* Call driver init functions */
    ADCBuf_init();
    GPIO_init();
    Timer_init();
    UART_init();

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

    initADCBuf();
    initTimer();

//    initUARTESP();
//    openUARTESP();
//    writeUARTInfinite();


//    resetWosMicMode(); // Override each mode pin to be HIGH (just to be sure)
//    initInterruptCallbacks();
    //    enableMicTriggerInterrupts();
    // Enable IirFilter
//    filter = new IirFilter();
//    filter->InitFilterState();
    int numBufsSent = 0;
    Motor* motors = new Motor();
    motors->Initialize();
    motors->PowerUp();
    while(1) {
        //        sem_wait(&adcbufSem);
        motors->DriveForwards(4000); sleep(1);
//        motors->DriveBackwards(4000); sleep(1);
        motors->DriveForwards(0); sleep(1);

        motors->DriveLeft(4000, 2000); sleep(1);
//        motors->DriveRight(4000, 1000); sleep(1);

//        motors->DriveLeft(4000, -500); sleep(1);
//        motors->DriveRight(4000, -500); sleep(1);
//
//        motors->DriveRight(4000, -4000); sleep(1);
//        motors->DriveRight(4000, 4000); sleep(1);

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
