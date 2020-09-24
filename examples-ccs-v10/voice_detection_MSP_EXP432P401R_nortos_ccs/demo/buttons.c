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

/** \file buttons.c Contains functions for managing and getting results for
 * the two buttons on the Launchpad the Launchpad.\n (C) Copyright 2015,
 * Texas Instruments, Inc.
 */

/*----------------------------------------------------------------------------
 * Includes
 * --------------------------------------------------------------------------*/
#include <stdlib.h>
#include <stdbool.h>


#include <ti/devices/msp432p4xx/inc/msp.h>
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

// #include <ti/drivers/dpl/HwiP.h>

#include "buttons_private.h"


/*----------------------------------------------------------------------------
 * Globals
 * -------------------------------------------------------------------------*/

static Buttons_Object ButtonsGlobal;
// HwiP_Handle TA3_N_Hwi;
// HwiP_Handle PORT1_Hwi;

/*----------------------------------------------------------------------------
 * Functions
 * --------------------------------------------------------------------------*/

Buttons_Handle Buttons_setup()
{
    Timer_A_ContinuousModeConfig contConfig =
    {
            TIMER_A_CLOCKSOURCE_ACLK,
            TIMER_A_CLOCKSOURCE_DIVIDER_1,
            TIMER_A_TAIE_INTERRUPT_DISABLE,
            TIMER_A_DO_CLEAR
    };

    /* Configure buttons 1 and 2 as P1.1 & P1.4 */
    MAP_GPIO_disableInterrupt(GPIO_PORT_P1, GPIO_PIN1 | GPIO_PIN4);
    MAP_GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN1 | GPIO_PIN4);
    MAP_GPIO_clearInterruptFlag(GPIO_PORT_P1, GPIO_PIN1 | GPIO_PIN4);
    MAP_GPIO_interruptEdgeSelect(GPIO_PORT_P1, GPIO_PIN1 | GPIO_PIN4, GPIO_HIGH_TO_LOW_TRANSITION);

    /* Set button press GPIO interrupt to higher priority than timer A3 */
    MAP_Interrupt_setPriority(INT_PORT1, 2<<5);

    /* Set up timer A3 for use in debouncing button presses */
    MAP_Interrupt_disableInterrupt(INT_TA3_0);
    MAP_Interrupt_disableInterrupt(INT_TA3_N);
    MAP_Timer_A_disableInterrupt(TIMER_A3_BASE);
    MAP_Timer_A_stopTimer(TIMER_A3_BASE);
    MAP_Timer_A_disableCaptureCompareInterrupt(TIMER_A3_BASE,
        TIMER_A_CAPTURECOMPARE_REGISTER_1);
    MAP_Timer_A_disableCaptureCompareInterrupt(TIMER_A3_BASE,
        TIMER_A_CAPTURECOMPARE_REGISTER_2);
    MAP_Timer_A_clearCaptureCompareInterrupt(TIMER_A3_BASE,
        TIMER_A_CAPTURECOMPARE_REGISTER_1);
    MAP_Timer_A_clearCaptureCompareInterrupt(TIMER_A3_BASE,
        TIMER_A_CAPTURECOMPARE_REGISTER_2);


    /* Set timer to count continuously when started */
    MAP_Timer_A_configureContinuousMode(TIMER_A3_BASE, &contConfig);

    /* Set timer A3 interrupt lower than button press GPIO interrupt. This
     * will ensure in the timer interrupt that all GPIO button press processing
     * has been done when the interrupt is disabled. */
    MAP_Interrupt_setPriority(INT_TA3_N, 3<<5);

    /* Setup buttons structure */
    ButtonsGlobal.button1Debounce = false;
    ButtonsGlobal.button1Pressed = false;
    ButtonsGlobal.button2Debounce = false;
    ButtonsGlobal.button2Pressed = false;

    /* Enable button press and timer A2 interrupts */
    MAP_Interrupt_enableInterrupt(INT_PORT1);
    MAP_Interrupt_enableInterrupt(INT_TA3_N);
    MAP_GPIO_enableInterrupt(GPIO_PORT_P1, GPIO_PIN1 | GPIO_PIN4);

    /* Link button press and timer A2 interrupts */
    // HwiP_Params hwiParamsPORT1;
    // HwiP_Params_init(&hwiParamsPORT1);
    // PORT1_Hwi = HwiP_create(51, PORT1_IRQHandler, &hwiParamsPORT1);

    // HwiP_Params hwiParamsTA3_N;
    // HwiP_Params_init(&hwiParamsTA3_N);
    // TA3_N_Hwi = HwiP_create(31, TA3_N_IRQHandler, &hwiParamsTA3_N);

    return &ButtonsGlobal;
}

/*--------------------------------------------------------------------------*/
/* Find out if button1 event set */
bool Buttons_getButton1(Buttons_Handle buttons)
{
    return buttons->button1Pressed;
}

/*--------------------------------------------------------------------------*/
/* Find out if button2 event set */
bool Buttons_getButton2(Buttons_Handle buttons)
{
    return buttons->button2Pressed;
}

/*--------------------------------------------------------------------------*/
/* Reset button1 event */
void Buttons_clearButton1(Buttons_Handle buttons)
{
    buttons->button1Pressed = false;
}

/*--------------------------------------------------------------------------*/
/* Reset button2 event */
void Buttons_clearButton2(Buttons_Handle buttons)
{
    buttons->button2Pressed = false;
}

/*--------------------------------------------------------------------------*/
/* Shutdown use of the buttons API */
void Buttons_shutdown(Buttons_Handle buttons)
{
    /* Disable button press and timer A3 interrupts */

    MAP_Timer_A_stopTimer(TIMER_A3_BASE);

    MAP_GPIO_disableInterrupt(GPIO_PORT_P1, GPIO_PIN1 | GPIO_PIN4);
    MAP_GPIO_clearInterruptFlag(GPIO_PORT_P1, GPIO_PIN1 | GPIO_PIN4);

    MAP_Timer_A_disableCaptureCompareInterrupt(TIMER_A3_BASE,
        TIMER_A_CAPTURECOMPARE_REGISTER_1);
    MAP_Timer_A_disableCaptureCompareInterrupt(TIMER_A3_BASE,
        TIMER_A_CAPTURECOMPARE_REGISTER_2);
    MAP_Timer_A_clearCaptureCompareInterrupt(TIMER_A3_BASE,
        TIMER_A_CAPTURECOMPARE_REGISTER_1);
    MAP_Timer_A_clearCaptureCompareInterrupt(TIMER_A3_BASE,
            TIMER_A_CAPTURECOMPARE_REGISTER_2);


    buttons->button1Debounce = false;
    buttons->button1Pressed = false;
    buttons->button2Debounce = false;
    buttons->button2Pressed = false;
}

/*--------------------------------------------------------------------------*/
/* GPIO interrupt ISR for buttons */
void PORT1_IRQHandler(void)
{
    int_fast8_t p1iv;
    uint16_t counter;

    /* Obtain the interrupt vector to determine button that triggered
     * interrupt. This will clear the highest priority P1IFG */
    p1iv = P1IV;

    /* button1 negative edge detected, no pending event, and debounce not ongoing */
    if( (p1iv == PXIV_PIN1) && !ButtonsGlobal.button1Pressed &&
        !ButtonsGlobal.button1Debounce)
    {
        /* Start timer A3 running, if not already */
        MAP_Timer_A_startCounter(TIMER_A3_BASE, TIMER_A_CONTINUOUS_MODE);

        /* Set up timer A CCR1 interrupt after debounce time */
        counter = Timer_A_getCounterValue(TIMER_A3_BASE);
        counter += BUTTON_DELAY;
        MAP_Timer_A_setCompareValue(TIMER_A3_BASE,
            TIMER_A_CAPTURECOMPARE_REGISTER_1, counter);
        MAP_Timer_A_enableCaptureCompareInterrupt (TIMER_A3_BASE,
            TIMER_A_CAPTURECOMPARE_REGISTER_1);

        ButtonsGlobal.button1Debounce = true;
    }

    /* button2 negative edge detected, no pending event, and debounce not ongoing */
    else if( (p1iv == PXIV_PIN4) && !ButtonsGlobal.button2Pressed &&
        !ButtonsGlobal.button2Debounce)
    {
        /* Start timer A3 running, if not already */
        MAP_Timer_A_startCounter(TIMER_A3_BASE, TIMER_A_CONTINUOUS_MODE);

        /* Set up timer A CCR2 interrupt after debounce time */
        counter = Timer_A_getCounterValue(TIMER_A3_BASE);
        counter += BUTTON_DELAY;
        MAP_Timer_A_setCompareValue(TIMER_A3_BASE,
            TIMER_A_CAPTURECOMPARE_REGISTER_2, counter);
        MAP_Timer_A_enableCaptureCompareInterrupt (TIMER_A3_BASE,
            TIMER_A_CAPTURECOMPARE_REGISTER_2);

        ButtonsGlobal.button2Debounce = true;
    }
}

/*--------------------------------------------------------------------------*/
/* Timer A3 CCR1&2 interrupt ISR */
void TA3_N_IRQHandler(void)
{
    int_fast16_t ta3iv;

    /* Obtain the interrupt vector to determine which CCR interrupted. This
     * will clear the highest priority TAXCCRn CCIFG */
    ta3iv = TA3IV;

    /* Debounce timeout for button1 */
    if(ta3iv == TAXIV_CCR1)
    {
        /* Disable the CCR interrupt timeout for button1 */
        MAP_Timer_A_disableCaptureCompareInterrupt(TIMER_A3_BASE,
            TIMER_A_CAPTURECOMPARE_REGISTER_1);
        ButtonsGlobal.button1Debounce = false;

        /* Set button1 event if button still pressed */
        if(MAP_GPIO_getInputPinValue(GPIO_PORT_P1, GPIO_PIN1) == GPIO_INPUT_PIN_LOW)
        {
            ButtonsGlobal.button1Pressed = true;
           Interrupt_disableSleepOnIsrExit();
            // sem_post(&semMain);
        }

        /* If button2 debounce count not ongoing, stop timer */
        Interrupt_disableInterrupt(INT_PORT1);
        if(!ButtonsGlobal.button2Debounce)
        {
            MAP_Timer_A_stopTimer(TIMER_A3_BASE);
        }
        MAP_Interrupt_enableInterrupt(INT_PORT1);
    }

    /* Debounce timeout for button2 */
    else if(ta3iv == TAXIV_CCR2)
    {
        /* Disable the CCR interrupt timeout for button2 */
        MAP_Timer_A_disableCaptureCompareInterrupt(TIMER_A3_BASE,
            TIMER_A_CAPTURECOMPARE_REGISTER_2);
        ButtonsGlobal.button2Debounce = false;

        /* Set button2 event if button still pressed */
        if(MAP_GPIO_getInputPinValue(GPIO_PORT_P1, GPIO_PIN4) == GPIO_INPUT_PIN_LOW)
        {
            ButtonsGlobal.button2Pressed = true;
           Interrupt_disableSleepOnIsrExit();
            // sem_post(&semMain);
        }

        /* If button1 debounce count not ongoing, stop timer */
        MAP_Interrupt_disableInterrupt(INT_PORT1);
        if(!ButtonsGlobal.button1Debounce)
        {
            MAP_Timer_A_stopTimer(TIMER_A3_BASE);
        }
        MAP_Interrupt_enableInterrupt(INT_PORT1);
    }
}

