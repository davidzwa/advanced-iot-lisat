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

/** \file leds.c Contains functions for managing LED display of the two
 * LEDs on the Launchpad.\n (C) Copyright 2015, Texas Instruments, Inc.
 */

/*----------------------------------------------------------------------------
 * Includes
 * --------------------------------------------------------------------------*/
#include <stdlib.h>
#include <stdbool.h>


#include <ti/devices/msp432p4xx/inc/msp.h>
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

#include "buttons_private.h"
#include "leds.h"

/*----------------------------------------------------------------------------
 * Globals
 * -------------------------------------------------------------------------*/
/* Port mapper configuration register to map P2.0-2 to outputs of timer A1,
 * CCR1-3 to control LED2 colors */
const uint8_t leds_mapping[] =
{
    PM_TA1CCR1A, PM_TA1CCR2A, PM_TA1CCR3A, PM_NONE, PM_NONE, PM_NONE, PM_NONE,
    PM_NONE
};

/*----------------------------------------------------------------------------
 * Functions
 * --------------------------------------------------------------------------*/
/* Set up LED ports and timer for color LED */
void Leds_setup()
{
    Timer_A_UpModeConfig upConfig =
    {
            TIMER_A_CLOCKSOURCE_ACLK,
            TIMER_A_CLOCKSOURCE_DIVIDER_1,
            254,
            TIMER_A_TAIE_INTERRUPT_DISABLE,
            TIMER_A_CCIE_CCR0_INTERRUPT_DISABLE,
            TIMER_A_DO_CLEAR
    };

    Timer_A_CompareModeConfig compareConfig =
    {
        TIMER_A_CAPTURECOMPARE_REGISTER_1,
        TIMER_A_CAPTURECOMPARE_INTERRUPT_DISABLE,
        TIMER_A_OUTPUTMODE_SET_RESET,
        255
    };


    /* Configure to control LED1 */
    MAP_GPIO_disableInterrupt(GPIO_PORT_P1, GPIO_PIN0);
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);

    /* Set up P2.0-2 to output the timer A1 CCR1-3 OUT PWM signals
     * to control LED2 color */
    MAP_GPIO_disableInterrupt(GPIO_PORT_P2, GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2);
    MAP_PMAP_configurePorts((const uint8_t *) leds_mapping, PMAP_P2MAP, 1,
        PMAP_ENABLE_RECONFIGURATION);
    MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2,
        GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2, GPIO_PRIMARY_MODULE_FUNCTION);

    /* Set up timer A1 to output pulse widths for RGB on outputs CCR1-3 */
    MAP_Interrupt_disableInterrupt(INT_TA1_0);
    MAP_Interrupt_disableInterrupt(INT_TA1_N);
    MAP_Timer_A_disableInterrupt(TIMER_A1_BASE);
    MAP_Timer_A_stopTimer(TIMER_A1_BASE);
    MAP_Timer_A_disableCaptureCompareInterrupt(TIMER_A1_BASE, 1);
    MAP_Timer_A_disableCaptureCompareInterrupt(TIMER_A1_BASE, 2);
    MAP_Timer_A_disableCaptureCompareInterrupt(TIMER_A1_BASE, 3);

    /* Set timer to count up */
    MAP_Timer_A_configureUpMode(TIMER_A1_BASE, &upConfig);

    /* Set timer to turn on LED color when count reached */
    MAP_Timer_A_initCompare(TIMER_A1_BASE, &compareConfig);
    compareConfig.compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_2;
    MAP_Timer_A_initCompare(TIMER_A1_BASE, &compareConfig);
    compareConfig.compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_3;
    MAP_Timer_A_initCompare(TIMER_A1_BASE, &compareConfig);

    /* Start timer for color LED PWM and button debounce */
    MAP_Timer_A_startCounter(TIMER_A1_BASE, TIMER_A_UP_MODE);
}

/*--------------------------------------------------------------------------*/
/* Turn on LED1 */
void Leds_on1()
{
    MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);
}

/*--------------------------------------------------------------------------*/
/* Turn off LED1 */
void Leds_off1()
{
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
}

/*--------------------------------------------------------------------------*/
/* Set color of LED2 */
void Leds_setColor2(uint_least8_t red, uint_least8_t green, uint_least8_t blue)
{
    Timer_A_CompareModeConfig compareConfig =
    {
        TIMER_A_CAPTURECOMPARE_REGISTER_1,
        TIMER_A_CAPTURECOMPARE_INTERRUPT_DISABLE,
        TIMER_A_OUTPUTMODE_SET_RESET,
        255
    };

    /* Set color of LED2 */
    red = 255-red;
    compareConfig.compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_1;
    compareConfig.compareValue = red;
    if(red == 0)
    {
        /* Set timer to turn on LED color 100% duty cyle */
        compareConfig.compareOutputMode = TIMER_A_OUTPUTMODE_SET;
    }
    else
    {
        /* Set timer to adjust PWM duty cycle */
         compareConfig.compareOutputMode = TIMER_A_OUTPUTMODE_SET_RESET;
    }
    Timer_A_initCompare(TIMER_A1_BASE, &compareConfig);


    green = 255 - green;
    compareConfig.compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_2;
    compareConfig.compareValue = green;
    if(green == 0)
    {
        /* Set timer to turn on LED color 100% duty cyle */
        compareConfig.compareOutputMode = TIMER_A_OUTPUTMODE_SET;
    }
    else
    {
        /* Set timer to adjust PWM duty cycle */
          compareConfig.compareOutputMode = TIMER_A_OUTPUTMODE_SET_RESET;
         MAP_Timer_A_initCompare(TIMER_A1_BASE, &compareConfig);
    }
    MAP_Timer_A_initCompare(TIMER_A1_BASE, &compareConfig);

    blue = 255 - blue;
    compareConfig.compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_3;
    compareConfig.compareValue = blue;
    if(blue == 0)
    {
        /* Set timer to turn on LED color 100% duty cyle */
        compareConfig.compareOutputMode = TIMER_A_OUTPUTMODE_SET;
    }
    else
    {
        /* Set timer to adjust PWM duty cycle */
         compareConfig.compareOutputMode = TIMER_A_OUTPUTMODE_SET_RESET;
    }
    MAP_Timer_A_initCompare(TIMER_A1_BASE, &compareConfig);
}

void Leds_shutdown()
{
    MAP_Timer_A_stopTimer(TIMER_A1_BASE);
    MAP_Timer_A_clearTimer(TIMER_A1_BASE);

    Leds_off1();
}



