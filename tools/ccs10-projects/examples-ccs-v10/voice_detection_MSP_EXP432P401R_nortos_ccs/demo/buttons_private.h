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

/** \file ledsbtns.h Contains the structure for mangaging setting leds and
 *  getting results for the two buttons on the MSP432 Launchpad.\n (C)
 *  Copyright 2015, Texas Instruments, Inc.
 */

#ifndef BUTTONS_PRIVATE_H_
#define BUTTONS_PRIVATE_H_

/*---------------------------------------------------------------------------
 * Includes
 * -------------------------------------------------------------------------*/
#include <stdbool.h>

#include "buttons.h"

/*---------------------------------------------------------------------------
 * Defines and Typedefs
 * -------------------------------------------------------------------------*/
/* GPIO interrupt vector values for each button */
#define PXIV_PIN1 (0x4)
#define PXIV_PIN4 (0xA)

/* Debounce delay counts relative to REF0 32.678KHz clock; ~15.3msec  */
#define BUTTON_DELAY (500)

/* CCR interrupt vector values for each CCR */
#define TAXIV_CCR1 (0x2)
#define TAXIV_CCR2 (0x4)

/* The Buttons structure, containing the button events and debounce state */
typedef struct Buttons_Struct
{
    /* Button press events */
    bool button1Pressed;
    bool button2Pressed;

    /* buttonPress debounce count active */
    bool button1Debounce;
    bool button2Debounce;

} Buttons_Object;


#endif /* BUTTONS_PRIVATE_H_ */
