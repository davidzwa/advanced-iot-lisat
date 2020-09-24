/* --COPYRIGHT--,BSD
 * Copyright (c) 2017, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * --/COPYRIGHT--*/

/** \file touch.c Contains functions for detecting and finding the location of
 * a touch on the LCD display.\n (C) Copyright 2015, Texas Instruments, Inc.
 */

/*----------------------------------------------------------------------------
 * Includes
 * --------------------------------------------------------------------------*/
/* Standard Includes */
#include <stdlib.h>
#include <stdbool.h>

/* MSP432 Family Generic Include File */
#include <ti/devices/msp432p4xx/inc/msp.h>

/* DriverLib Header Files */
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

/* LCD Driver module */
#if defined(KENTEC_TOUCH)

/* APIs used in this application */
#include "touch_private.h"

/*----------------------------------------------------------------------------
 * Defines and Typedefs
 * -------------------------------------------------------------------------*/
#define ABSVAL(x)  ( ((x) < 0)  ?  -(x) : (x) )

/*----------------------------------------------------------------------------
 * Globals
 * -------------------------------------------------------------------------*/

/* Detection comparator configuration */

const COMP_E_Config ceConfig =
{
        COMP_E_INPUT6,
        COMP_E_VREF,
        COMP_E_FILTEROUTPUT_DLYLVL4,
        COMP_E_NORMALOUTPUTPOLARITY,
        COMP_E_NORMAL_MODE
};

Timer_A_UpModeConfig upConfig =
 {
         TIMER_A_CLOCKSOURCE_ACLK,
         TIMER_A_CLOCKSOURCE_DIVIDER_1,
         546,
         TIMER_A_TAIE_INTERRUPT_DISABLE,
         TIMER_A_CCIE_CCR0_INTERRUPT_ENABLE,
         TIMER_A_DO_CLEAR
 };

static Touch_Object TouchGlobal;

/*----------------------------------------------------------------------------
 * Functions
 * --------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/* Set up the touch API capability */
Touch_Handle Touch_setup()
{
    bool initOK;

    Touch_Object *touch = &TouchGlobal;

    /* Disable any use of TOUCH_CE */
    MAP_Interrupt_disableInterrupt(TOUCH_CE_INT);
    MAP_COMP_E_disableInterrupt( TOUCH_CE, COMP_E_OUTPUT_INTERRUPT ||
            COMP_E_INVERTED_POLARITY_INTERRUPT || COMP_E_READY_INTERRUPT);

    MAP_COMP_E_disableModule(TOUCH_CE);

    /* Set up the comparator to do detection */
    initOK = MAP_COMP_E_initModule(TOUCH_CE, &ceConfig);
    if( !initOK)
    {
        return NULL;
    }

    /* Set up the hysteresis voltage levels */
    MAP_COMP_E_setReferenceVoltage(TOUCH_CE,
            COMP_E_REFERENCE_AMPLIFIER_DISABLED,
            TOUCH_HYST_HIGH_THRESH,
            TOUCH_HYST_LOW_THRESH);

    /* Interrupt when the voltage goes low => a touch has occurred */
    MAP_COMP_E_setInterruptEdgeDirection(TOUCH_CE, COMP_E_RISINGEDGE);

    MAP_COMP_E_clearInterruptFlag( TOUCH_CE, COMP_E_OUTPUT_INTERRUPT ||
            COMP_E_INVERTED_POLARITY_INTERRUPT || COMP_E_READY_INTERRUPT);

    /* Set comparator input GPIO port CE1.6 */
    MAP_GPIO_disableInterrupt(TOUCH_CE_PORT, TOUCH_CE_PIN);
    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(TOUCH_CE_PORT,TOUCH_CE_PIN, GPIO_TERTIARY_MODULE_FUNCTION);
    MAP_GPIO_clearInterruptFlag(TOUCH_CE_PORT, TOUCH_CE_PIN);

    /* Disable interrupts on all touch pins */
    MAP_GPIO_disableInterrupt(TOUCH_X_PLUS_PORT, TOUCH_X_PLUS_PIN);
    MAP_GPIO_disableInterrupt(TOUCH_X_MINUS_PORT, TOUCH_X_MINUS_PIN);
    MAP_GPIO_disableInterrupt(TOUCH_Y_PLUS_PORT, TOUCH_Y_PLUS_PIN);
    MAP_GPIO_disableInterrupt(TOUCH_Y_MINUS_PORT, TOUCH_Y_MINUS_PIN);

    MAP_GPIO_clearInterruptFlag(TOUCH_X_PLUS_PORT, TOUCH_X_PLUS_PIN);
    MAP_GPIO_clearInterruptFlag(TOUCH_X_MINUS_PORT, TOUCH_X_MINUS_PIN);
    MAP_GPIO_clearInterruptFlag(TOUCH_Y_PLUS_PORT, TOUCH_Y_PLUS_PIN);
    MAP_GPIO_clearInterruptFlag(TOUCH_Y_MINUS_PORT, TOUCH_Y_MINUS_PIN);

    /* Set up GPIO 7.2 to output the comparator value for debug */
    MAP_GPIO_disableInterrupt(GPIO_PORT_P7, GPIO_PIN2);
    MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P7, GPIO_PIN2, GPIO_PRIMARY_MODULE_FUNCTION);
    MAP_GPIO_clearInterruptFlag(GPIO_PORT_P7, GPIO_PIN2);

    /* Set up GPIO 7.5 to output pulse while measuring touch location for debug */
    MAP_GPIO_disableInterrupt(GPIO_PORT_P7, GPIO_PIN5);
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P7, GPIO_PIN5);
    MAP_GPIO_clearInterruptFlag(GPIO_PORT_P7, GPIO_PIN5);
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P7, GPIO_PIN5);

    /* Set up timer. When a touch interrupt occurs, it starts the timer.
     * After a delay the timer times out and its interrupt checks to
     * see if the touch is still occurring. If it is, then a touch
     * is declared. */

    /* Set up timer for touch delay */
     upConfig.timerPeriod = CS_getACLK()/TOUCH_DEBOUNCE;

     MAP_Interrupt_disableInterrupt(INT_TA2_0);
     MAP_Interrupt_disableInterrupt(INT_TA2_N);
     MAP_Timer_A_disableCaptureCompareInterrupt(TIMER_A2_BASE,
         TIMER_A_CAPTURECOMPARE_REGISTER_0);
     MAP_Timer_A_stopTimer(TIMER_A2_BASE);
     MAP_Timer_A_clearCaptureCompareInterrupt(TIMER_A2_BASE,
         TIMER_A_CAPTURECOMPARE_REGISTER_0);

     /* Setup the touch object */
     touch->touchDetected = false;
     touch->touchDebounce = false;
     touch->touchEnabled = false;
     touch->xMin = TOUCH_X_MINIMUM;
     touch->xMax = TOUCH_X_MAXIMUM;
     touch->yMin = TOUCH_Y_MINIMUM;
     touch->yMax = TOUCH_Y_MAXIMUM;
     touch->xLocation = 0;
     touch->yLocation = 0;

     return touch;
}

/*--------------------------------------------------------------------------*/
/* Shut down the touch API resources */
void Touch_shutdown(Touch_Handle touch)
{
    if(touch == NULL)
    {
        return;
    }

    if(touch->touchEnabled)
    {
        Touch_disableDetect(touch, true);
    }


    /* All touch signals set to high impedance */
    MAP_GPIO_setAsInputPin(TOUCH_X_PLUS_PORT, TOUCH_X_PLUS_PIN);
    MAP_GPIO_setAsInputPin(TOUCH_X_MINUS_PORT, TOUCH_X_MINUS_PIN);

    MAP_GPIO_setAsInputPin(TOUCH_Y_PLUS_PORT, TOUCH_Y_PLUS_PIN);
    MAP_GPIO_setAsInputPin(TOUCH_Y_MINUS_PORT, TOUCH_Y_MINUS_PIN);
}

/*--------------------------------------------------------------------------*/
/* Enable comparator to detect a touch. User can set to obtain information
 * about the touch event via interrupt or polling until a touch occurs. */
void Touch_enableDetect(Touch_Handle touch, bool enableInterrupt )
{

    /* Set up voltages on LCD resistive touch pins */

    /* Set Y- as input with pull-up resistor */
    MAP_GPIO_setAsInputPinWithPullUpResistor(TOUCH_Y_MINUS_PORT, TOUCH_Y_MINUS_PIN);

    /* X+ and X- set to ground */
//    MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(TOUCH_X_PLUS_PORT, TOUCH_X_PLUS_PIN, GPIO_SECONDARY_MODULE_FUNCTION);
//    MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(TOUCH_X_MINUS_PORT, TOUCH_X_MINUS_PIN, GPIO_SECONDARY_MODULE_FUNCTION);
    MAP_GPIO_setAsOutputPin(TOUCH_X_PLUS_PORT, TOUCH_X_PLUS_PIN);
    MAP_GPIO_setOutputLowOnPin(TOUCH_X_PLUS_PORT, TOUCH_X_PLUS_PIN);
    MAP_GPIO_setAsOutputPin(TOUCH_X_MINUS_PORT, TOUCH_X_MINUS_PIN);
    MAP_GPIO_setOutputLowOnPin(TOUCH_X_MINUS_PORT, TOUCH_X_MINUS_PIN);

    /* Set Y+ as high impedance floating input */
    MAP_GPIO_setAsInputPin(TOUCH_Y_PLUS_PORT, TOUCH_Y_PLUS_PIN);

    /* Turn on the comparator module to detect when Y- goes below threshold */
    MAP_COMP_E_enableModule(TOUCH_CE);

    MAP_COMP_E_clearInterruptFlag( TOUCH_CE, COMP_E_OUTPUT_INTERRUPT ||
            COMP_E_INVERTED_POLARITY_INTERRUPT || COMP_E_READY_INTERRUPT);

    MAP_Interrupt_enableInterrupt(TOUCH_CE_INT);

    /* Enable an interrupt on low-going comparator output */
    if( enableInterrupt)
    {
        touch->touchDetected = false;
        touch->touchDebounce = false;
        MAP_COMP_E_enableInterrupt(TOUCH_CE, COMP_E_OUTPUT_INTERRUPT);
    }

    touch->touchEnabled = true;
}

/*--------------------------------------------------------------------------*/
/* Disable comparator to interrupt on touch detection. The comparator module
 * may be left running, in order to obtain the comparator state. */
void Touch_disableDetect(Touch_Handle touch, bool disableModule)
{
    /* Disable any interrupts */
    MAP_Interrupt_disableInterrupt(INT_TA2_0);
    MAP_Interrupt_disableInterrupt(TOUCH_CE_INT);


    /* Inactivate the debounce timer */
    MAP_Timer_A_disableCaptureCompareInterrupt(TIMER_A2_BASE,
        TIMER_A_CAPTURECOMPARE_REGISTER_0);
    MAP_Timer_A_stopTimer(TIMER_A2_BASE);
    MAP_Timer_A_clearCaptureCompareInterrupt(TIMER_A2_BASE,
        TIMER_A_CAPTURECOMPARE_REGISTER_0);


    /* Disable any use of TOUCH_CE */
    MAP_COMP_E_disableInterrupt( TOUCH_CE, COMP_E_OUTPUT_INTERRUPT ||
            COMP_E_INVERTED_POLARITY_INTERRUPT || COMP_E_READY_INTERRUPT);

    if( disableModule )
    {
        MAP_COMP_E_disableModule(TOUCH_CE);
        touch->touchEnabled = false;
    }

    MAP_COMP_E_clearInterruptFlag( TOUCH_CE, COMP_E_OUTPUT_INTERRUPT);
    touch->touchDetected = false;
    touch->touchDebounce = false;


}

/*--------------------------------------------------------------------------*/
/* Obtain result to determine if touch interrupt detection occurred */
bool Touch_getDetected(Touch_Handle touch)
{
    return touch->touchDetected;
}

/*--------------------------------------------------------------------------*/
/* Reset the flag indicating a touch interrupt has occurred */
void Touch_resetDetected(Touch_Handle touch)
{
    touch->touchDetected = false;
}

/*--------------------------------------------------------------------------*/
/* Obtain current state of the touch detection comparator */
bool Touch_getState(Touch_Handle touch, bool *state)
{
    if(!touch->touchEnabled)
    {
        return false;
    }

    *state = (MAP_COMP_E_outputValue(TOUCH_CE) == COMP_E_HIGH);
    return true;
}

/*--------------------------------------------------------------------------*/
/* Get location of a touch. This should only be done after a touch is detected,
 * which disables the comparator processing.
 * While getting the locations, the set-up for the comparator can not continue
 * to determine if a touch is ongoing, so other tests must be done. The ADC must
 * not be otherwise active during getting the location. The location can be
 * provided as the raw location data, or normalized to 0 to 1 in Q14. */
bool Touch_getLocation(Touch_Handle touch, Touch_Info *touchInfo, bool normalize)
{

    /* For debug set 7.5 high */
    MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P7, GPIO_PIN5);

    /* Initialize the ADC14 module for manual sampling of X and Y signals. */
    MAP_ADC14_initModule(ADC_CLOCKSOURCE_ADCOSC, ADC_PREDIVIDER_1,
            ADC_DIVIDER_1, 0);
    MAP_ADC14_setResolution(ADC_14BIT);
    MAP_ADC14_setResultFormat(ADC_UNSIGNED_BINARY);

    MAP_ADC14_enableSampleTimer(ADC_MANUAL_ITERATION);
    MAP_ADC14_setSampleHoldTime(ADC_PULSE_WIDTH_96, ADC_PULSE_WIDTH_96);
    MAP_ADC14_setSampleHoldTrigger(ADC_TRIGGER_ADCSC, false);

    MAP_ADC14_enableModule();

    /* Configure Y+ input to memory buffer 0. */
    MAP_ADC14_configureConversionMemory(TOUCH_Y_PLUS_MEMORY,
            ADC_VREFPOS_AVCC_VREFNEG_VSS,
            TOUCH_Y_PLUS_INPUT,
            false);

    /* Configure X+ input to memory buffer 1. */
    MAP_ADC14_configureConversionMemory(TOUCH_X_PLUS_MEMORY,
            ADC_VREFPOS_AVCC_VREFNEG_VSS,
            TOUCH_X_PLUS_INPUT,
            false);

    /* Read the X and Y location from the ADC */
    Touch_readLocation(touch);

    /* For debug, set output low on 7.5 */
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P7, GPIO_PIN5);

    /* Locations are valid if touch still occurring */
    touch->validLocation = (MAP_COMP_E_outputValue(TOUCH_CE) == COMP_E_LOW);

    /* Disable the ADC14 */
    MAP_ADC14_disableConversion();

    if(normalize)
    {
        /* Supply the user with the locations reported 0 to 1 in Q14 */
        touchInfo->locationX = Touch_normalize(touch->xLocation, touch->xMin, touch->xMax);
        touchInfo->locationY = Touch_normalize(touch->yLocation, touch->yMin, touch->yMax);
    }
    else
    {
        touchInfo->locationX = touch->xLocation;
        touchInfo->locationY = touch->yLocation;
    }

    touchInfo->validLocation = touch->validLocation;
    return touchInfo->validLocation;
}

/*--------------------------------------------------------------------------*/
/* Recalibrate the QVGA touch screen if the values indicate that a calibrate
 * sequence has been provided. The touchInfo argument is an array of four
 * touchInfo elements. In order to recalibrate, the four touches must be in
 * the order upper left, upper right, lower right, lower left. */
void Touch_calibrate(Touch_Handle touch, Touch_Info *touchInfo)
{
    int_fast16_t i;

    /* All touches must be valid */
    for(i = 0; i < 3; i++)
    {
        if(!touchInfo[i].validLocation)
        {
            return;
        }
    }

    /* All touches must agree as to touch order, rectangular placement, and values expected at corners */
    if( ( ABSVAL( (touchInfo[0].locationX - touchInfo[3].locationX) ) < TOUCH_EQUAL_THRESH ) &&
        ( ABSVAL( (touchInfo[1].locationX - touchInfo[2].locationX) ) < TOUCH_EQUAL_THRESH )    &&
        ( ABSVAL( (touchInfo[0].locationY - touchInfo[1].locationY) ) < TOUCH_EQUAL_THRESH ) &&
        ( ABSVAL( (touchInfo[2].locationX - touchInfo[3].locationX) ) < TOUCH_EQUAL_THRESH ) &&
        ( (touchInfo[1].locationX - touchInfo[0].locationX) > TOUCH_DIFFER_THRESH ) &&
        ( (touchInfo[2].locationX - touchInfo[3].locationX) > TOUCH_DIFFER_THRESH ) &&
        ( (touchInfo[3].locationY - touchInfo[0].locationY) > TOUCH_DIFFER_THRESH ) &&
        ( (touchInfo[2].locationY - touchInfo[1].locationY) > TOUCH_DIFFER_THRESH ) )
    {
        touch->xMin = ( (touchInfo[0].locationX + touchInfo[3].locationX) >> 1 );
        touch->xMax = ( (touchInfo[1].locationX + touchInfo[2].locationX) >> 1 );

        touch->yMin = ( (touchInfo[0].locationY + touchInfo[1].locationY) >> 1 );
        touch->yMax = ( (touchInfo[2].locationY + touchInfo[3].locationY) >> 1 );
    }

}

/*--------------------------------------------------------------------------*/
/* Normalize raw location values contained in a Touch_Info object.
 * The values are normalized to 0 to 1 in Q14. */
void Touch_normalizeLocation(Touch_Handle touch, Touch_Info *touchInfo)
{
    /* Normalize the raw locations */
    touchInfo->locationX = Touch_normalize(touchInfo->locationX, touch->xMin, touch->xMax);
    touchInfo->locationY = Touch_normalize(touchInfo->locationY, touch->yMin, touch->yMax);
}

/*--------------------------------------------------------------------------*/
/* Read the raw location of the touch */
void Touch_readLocation(Touch_Handle touch)
{
    volatile uint_fast64_t status;
    uint_fast16_t i;
    uint_least16_t loc[TOUCH_OVERSAMPLE];

    /* Set up to read Y location. */

    /* Set X- as input (floating). */
    MAP_GPIO_setAsInputPin(TOUCH_X_MINUS_PORT, TOUCH_X_MINUS_PIN);

    /* Sample the X+ ADC channel. */
    MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(TOUCH_X_PLUS_PORT,
            TOUCH_X_PLUS_PIN,
            GPIO_TERTIARY_MODULE_FUNCTION);

    /* Set Y+ and Y- as output and set Y+ low and Y- high. */
    MAP_GPIO_setAsOutputPin(TOUCH_Y_PLUS_PORT, TOUCH_Y_PLUS_PIN);
    MAP_GPIO_setAsOutputPin(TOUCH_Y_MINUS_PORT, TOUCH_Y_MINUS_PIN);
    MAP_GPIO_setOutputLowOnPin(TOUCH_Y_PLUS_PORT, TOUCH_Y_PLUS_PIN);
    MAP_GPIO_setOutputHighOnPin(TOUCH_Y_MINUS_PORT, TOUCH_Y_MINUS_PIN);

    MAP_ADC14_clearInterruptFlag(TOUCH_X_PLUS_IFG | TOUCH_Y_PLUS_IFG);
    MAP_ADC14_disableConversion();
    MAP_ADC14_configureSingleSampleMode(TOUCH_X_PLUS_MEMORY, false);
    MAP_ADC14_enableConversion();

    for(i = 0; i < TOUCH_OVERSAMPLE; i++)
    {
        MAP_ADC14_toggleConversionTrigger();
        status = MAP_ADC14_getInterruptStatus();
        while( !(status & TOUCH_X_PLUS_IFG))
        {
            status = MAP_ADC14_getInterruptStatus();
        }
        loc[i] = MAP_ADC14_getResult(TOUCH_X_PLUS_MEMORY);
        MAP_ADC14_clearInterruptFlag(TOUCH_X_PLUS_IFG);
    }

//    accum >>= TOUCH_OVERSAMPLE_SHIFT;
    touch->yLocation = loc[TOUCH_OVERSAMPLE - 1];

    /* Set up to read X location. */

    /* Set Y- as input (floating). */
    MAP_GPIO_setAsInputPin(TOUCH_Y_MINUS_PORT, TOUCH_Y_MINUS_PIN);

    /* Sample the Y+ ADC channel. */
    MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(TOUCH_Y_PLUS_PORT,
                                                    TOUCH_Y_PLUS_PIN,
                                                    GPIO_TERTIARY_MODULE_FUNCTION);

    /* Set X+ and X- as output with X+ high and X- low. */
    MAP_GPIO_setAsOutputPin(TOUCH_X_PLUS_PORT, TOUCH_X_PLUS_PIN);
    MAP_GPIO_setAsOutputPin(TOUCH_X_MINUS_PORT, TOUCH_X_MINUS_PIN);
    MAP_GPIO_setOutputHighOnPin(TOUCH_X_PLUS_PORT, TOUCH_X_PLUS_PIN);
    MAP_GPIO_setOutputLowOnPin(TOUCH_X_MINUS_PORT, TOUCH_X_MINUS_PIN);

    MAP_ADC14_clearInterruptFlag(TOUCH_X_PLUS_IFG | TOUCH_Y_PLUS_IFG);
    MAP_ADC14_disableConversion();
    MAP_ADC14_configureSingleSampleMode(TOUCH_Y_PLUS_MEMORY, false);
    MAP_ADC14_enableConversion();

    for(i = 0; i < TOUCH_OVERSAMPLE; i++)
    {
        MAP_ADC14_toggleConversionTrigger();
        status = MAP_ADC14_getInterruptStatus();
        while(!(status & TOUCH_Y_PLUS_IFG))
        {
            status = MAP_ADC14_getInterruptStatus();
        }
        loc[i] = MAP_ADC14_getResult(TOUCH_Y_PLUS_MEMORY);
        MAP_ADC14_clearInterruptFlag(TOUCH_Y_PLUS_IFG);
//        accum += loc[i];
    }

//    accum >>= TOUCH_OVERSAMPLE_SHIFT;
    touch->xLocation = loc[TOUCH_OVERSAMPLE - 1];


    /* Return the touch signals to configuration for detecting touch */

    /* Change X+ from high to Vss */
//     GPIO_setAsPeripheralModuleFunctionOutputPin(TOUCH_X_PLUS_PORT, TOUCH_X_PLUS_PIN, GPIO_SECONDARY_MODULE_FUNCTION);
    MAP_GPIO_setAsOutputPin(TOUCH_X_PLUS_PORT, TOUCH_X_PLUS_PIN);
    MAP_GPIO_setOutputLowOnPin(TOUCH_X_PLUS_PORT, TOUCH_X_PLUS_PIN);

     /* Set Y- as input with pull-up resistor */
      MAP_GPIO_setAsInputPinWithPullUpResistor(TOUCH_Y_MINUS_PORT, TOUCH_Y_MINUS_PIN);

     /* Change X- from low to Vss */

//     GPIO_setAsPeripheralModuleFunctionOutputPin(TOUCH_X_MINUS_PORT, TOUCH_X_MINUS_PIN, GPIO_SECONDARY_MODULE_FUNCTION);
    MAP_GPIO_setAsOutputPin(TOUCH_X_MINUS_PORT, TOUCH_X_MINUS_PIN);
    MAP_GPIO_setOutputLowOnPin(TOUCH_X_MINUS_PORT, TOUCH_X_MINUS_PIN);

     /* Set Y+ from analog ADC input to high impedance floating input */
     MAP_GPIO_setAsInputPin(TOUCH_Y_PLUS_PORT, TOUCH_Y_PLUS_PIN);

     /* Wait for time to stabilize the comparator touch detection */
     for(i = 0; i < TOUCH_STAB_COUNT; i++)
     {
     }
}


/*--------------------------------------------------------------------------*/
/* Normalize locations of the touch to the range of 0 to 1 in Q14. */
int_least16_t Touch_normalize(int_least16_t loc, int_least16_t minLoc, int_least16_t maxLoc)
{
    int_least32_t tempLoc;

    loc = (loc < minLoc)  ?  minLoc : loc;
    loc = (loc > maxLoc)  ?  maxLoc : loc;

    tempLoc = loc - minLoc;
    tempLoc <<= 14;
    tempLoc /= (maxLoc - minLoc);
    return (int_least16_t)(tempLoc);
}

/*--------------------------------------------------------------------------*/
/* TOUCH_CE interrupt handler gets called when touch first occurs */
void COMP_E1_IRQHandler(void)
{
    /* Disable any further comparator interrupts */
    /* Enable an interrupt on low-going comparator output */
    MAP_COMP_E_disableInterrupt(TOUCH_CE, COMP_E_OUTPUT_INTERRUPT);
    MAP_COMP_E_clearInterruptFlag(TOUCH_CE, COMP_E_OUTPUT_INTERRUPT);

    /* Start delay timer with priority */
    MAP_Interrupt_setPriority(INT_TA2_0, 7<<5);
    MAP_Timer_A_configureUpMode(TIMER_A2_BASE, &upConfig);

    MAP_Timer_A_startCounter(TIMER_A2_BASE, TIMER_A_UP_MODE);
    MAP_Interrupt_enableInterrupt(INT_TA2_0);
    TouchGlobal.touchDebounce = true;
}


/*--------------------------------------------------------------------------*/
/* Interrupt handler for touch debounce */
void TA2_0_IRQHandler(void)
{

    /* Stop the delay interrupt timer */
    MAP_Timer_A_clearCaptureCompareInterrupt(TIMER_A2_BASE,
        TIMER_A_CAPTURECOMPARE_REGISTER_0);
    MAP_Timer_A_stopTimer(TIMER_A2_BASE);

    /* Declare touch if comparator still low */
    if( MAP_COMP_E_outputValue(TOUCH_CE) == COMP_E_LOW)
    {
        /* Disable comparator detect interrupts, but keep the comparator
         * active, in preparation for getting the touch location. The
         * comparator module can be shutdown later by the user. */
         Touch_disableDetect(&TouchGlobal, false);

         TouchGlobal.touchDetected = true;

        /* Allow processing of the new Touch event in main thread */
         Interrupt_disableSleepOnIsrExit();
    }
    else
    {
        /* The touch has ended prematurely. Continue to search for valid touch. */
        TouchGlobal.touchDetected = false;

        /* Keep looking for a valid touch */
        MAP_COMP_E_clearInterruptFlag(TOUCH_CE, COMP_E_OUTPUT_INTERRUPT);
        MAP_COMP_E_enableInterrupt(TOUCH_CE, COMP_E_OUTPUT_INTERRUPT);
    }

    TouchGlobal.touchDebounce  = false;
}
#endif
