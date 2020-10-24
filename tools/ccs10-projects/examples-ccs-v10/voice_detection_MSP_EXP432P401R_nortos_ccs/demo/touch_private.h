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

/** \file touch_private.h Contains parameters and functions for providing touch
 * capabilities on the QVGA display.
 */

#ifndef TOUCH_PRIVATE_H_
#define TOUCH_PRIVATE_H_

/*----------------------------------------------------------------------------
 * Includes
 * -------------------------------------------------------------------------*/
#include <stdlib.h>
#include <stdbool.h>


#include <ti/devices/msp432p4xx/inc/msp.h>
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>


#include "touch.h"

/*----------------------------------------------------------------------------
 * Defines and Typedefs
 * -------------------------------------------------------------------------*/

/* X+ definitions. ---> Pin 24 on LaunchPad ---> P4.0, A13 on Msp432*/
#define TOUCH_X_PLUS_PORT       GPIO_PORT_P4
#define TOUCH_X_PLUS_PIN        GPIO_PIN0
#define TOUCH_X_PLUS_INPUT      ADC_INPUT_A13
#define TOUCH_X_PLUS_MEMORY     ADC_MEM1
#define TOUCH_X_PLUS_IFG        ADC_INT1

/* Y+ definitions. ----> Pin 23 on LaunchPad ---> P6.1, A14 on MSP432*/
#define TOUCH_Y_PLUS_PORT       GPIO_PORT_P6
#define TOUCH_Y_PLUS_PIN        GPIO_PIN1
#define TOUCH_Y_PLUS_INPUT      ADC_INPUT_A14
#define TOUCH_Y_PLUS_MEMORY     ADC_MEM0
#define TOUCH_Y_PLUS_IFG        ADC_INT0

/* X- definitions. --> Pin 31 on LaunchPad --> P3.7 on MSP432*/
#define TOUCH_X_MINUS_PORT      GPIO_PORT_P3
#define TOUCH_X_MINUS_PIN       GPIO_PIN7

/* Y- definitions. --> Pin 11 on LaunchPad  --> P3.6 on MSP432*/
/* This pin should be tied to Pin 17 (P5.7) on Launchpad to enable comparator touch sensing. */
#define TOUCH_Y_MINUS_PORT      GPIO_PORT_P3
#define TOUCH_Y_MINUS_PIN       GPIO_PIN6

/* Parameters for using comparator for detection of touch */
#define TOUCH_CE COMP_E1_BASE
#define TOUCH_CE_INT INT_COMP_E1
#define TOUCH_CE_PORT  GPIO_PORT_P5
#define TOUCH_CE_PIN   GPIO_PIN7

/* CE1 ladder settings for voltage threshold hysteresis for detecting a touch using CE1 */
/* Approximately 80% Vcc */
#define TOUCH_HYST_HIGH_THRESH  (26)
/* Approximately 50% Vcc */
#define TOUCH_HYST_LOW_THRESH   (16)

/* Debounce time for checking touch is 1/2000 sec */
#define TOUCH_DEBOUNCE (2000)

/* Loop count for comparator stabilization after reading location */
#define TOUCH_STAB_COUNT (20)

#define TOUCH_OVERSAMPLE  (16)
#define TOUCH_OVERSAMPLE_SHIFT (4)

/* Default initial touch extreme points */
#define TOUCH_X_MINIMUM (2500)
#define TOUCH_X_MAXIMUM (14200)
#define TOUCH_Y_MINIMUM (2600)
#define TOUCH_Y_MAXIMUM (14000)

#define TOUCH_EQUAL_THRESH  (700)
#define TOUCH_DIFFER_THRESH (11000)

typedef struct Touch_Struct
{
    /* Flag indicating touch detection is enabled */
    bool touchEnabled;

    /* Detect event */
    bool touchDetected;

    /* Touch detect debounce count active */
    bool touchDebounce;

    /* touch calibration point values */
    int_least16_t xMin;
    int_least16_t xMax;
    int_least16_t yMin;
    int_least16_t yMax;

    /* raw locations returned from ADC */
    int_least16_t xLocation;
    int_least16_t yLocation;

    bool validLocation;

} Touch_Object;



/*----------------------------------------------------------------------------
 * Functions
 * --------------------------------------------------------------------------*/

//extern void touch_initInterface(void);
//extern void touch_updateCurrentTouch(touch_context *data);
//extern void touch_calibrate(void);


/*--------------------------------------------------------------------------*/
/* Read the location of the touch in raw coordinates */
static void Touch_readLocation(Touch_Handle touch);

/*--------------------------------------------------------------------------*/
/* Normalize locations of the touch to the range of 0 to 1 in Q14. */
static int_least16_t Touch_normalize(int_least16_t loc, int_least16_t minLoc, int_least16_t maxLoc);

#endif /* TOUCH_H_ */
