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

/** \file touch.h Contains functions for managing and getting results for
 * the QVGA touch screen  on the Launchpad .\n (C) Copyright 2016,
 *  Texas Instruments, Inc.
 */

#ifndef TOUCH_H_
#define TOUCH_H_

/*----------------------------------------------------------------------------
 * Includes
 * -------------------------------------------------------------------------*/
#include <stdint.h>

/*----------------------------------------------------------------------------
 * Defines and Typedefs
 * -------------------------------------------------------------------------*/
typedef struct Touch_Struct *Touch_Handle;

typedef struct TouchInfo_Struct
{
    int_least16_t locationX;
    int_least16_t locationY;
    bool validLocation;
}Touch_Info;

/*----------------------------------------------------------------------------
 * Functions
 * --------------------------------------------------------------------------*/
/* Setup for using touch for QVGA LCD on the Launchpad */
Touch_Handle Touch_setup(void);


/*--------------------------------------------------------------------------*/
/* Shutdown use of the touch API */
void Touch_shutdown(Touch_Handle touch);


/*--------------------------------------------------------------------------*/
/* Enable Touch API to detect touch. The user can choose to interrupt when
 * a touch occurs or go into a polling loop until a touch occurs. */
void Touch_enableDetect(Touch_Handle touch, bool enableInterrupt);

/*--------------------------------------------------------------------------*/
/* Obtain current state of the touch detection comparator. This should only
 * be called when touch detection is enabled. The return value is true if
 * the state can be read. The state of the comparator is returned in the
 * location specified by the state argument */
bool Touch_getState(Touch_Handle touch, bool *state);

/*--------------------------------------------------------------------------*/
/* Disable the Touch API to detect and report a touch. The user can choose to
 * completely disable the comparator module, or leave it active so that the
 * user can read the current state of the touch detection comparator. */
void Touch_disableDetect(Touch_Handle touch, bool disableModule);

/*--------------------------------------------------------------------------*/
/* Read if an interrupt state event has been set */
bool Touch_getDetected(Touch_Handle touch);

/*--------------------------------------------------------------------------*/
/* Reset touch event */
void Touch_resetDetected(Touch_Handle touch);

/*--------------------------------------------------------------------------*/
/* Get location of touch, either in raw coordinates or normalized 0 to 1 in Q14.
 * Returns a flag indicating if the touch location qualifies as valid. */
bool Touch_getLocation(Touch_Handle touch, Touch_Info *touchInfo, bool normalize);

/*--------------------------------------------------------------------------*/
/* Normalize raw location values contained in a Touch_Info object.
 * The values are normalized to 0 to 1 in Q14. */
void Touch_normalizeLocation(Touch_Handle touch, Touch_Info *touchInfo);

/*--------------------------------------------------------------------------*/
/* Recalibrate the QVGA touch screen if the values indicate that a calibrate
 * sequence has been provided. The touchInfo argument is an array of four
 * touchInfo elements. In order to recalibrate, the four touches must be in
 * the order upper left, upper right, lower right, lower left. */
void Touch_calibrate(Touch_Handle touch, Touch_Info *touchInfo);

void COMP_E1_IRQHandler(void);
void TA2_0_IRQHandler(void);

#endif /* TOUCH_H_ */
