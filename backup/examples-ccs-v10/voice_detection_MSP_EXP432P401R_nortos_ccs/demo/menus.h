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

/** \file menus.h Defines the menu structure and defines the basic menu
 * manipulation functions. \n (C) Copyright 2015, Texas Instruments, Inc.
 */


#ifndef MENUS_H_
#define MENUS_H_

/*----------------------------------------------------------------------------
 * Includes
 * -------------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>

/*----------------------------------------------------------------------------
 * Defines and Typedefs
 * -------------------------------------------------------------------------*/

/* Events that the menus will process. The App must provide these events */
typedef enum Event_Enum
{
    EVENT_BUTTON1,
    EVENT_BUTTON2,
    EVENT_AUDIOFRAME,
    EVENT_TOUCH
} Events;

/* Type of the function used for processing events */
typedef struct MenuStruct * (* const ProcessFunction)(Events);

/* Declaration of the menu structure */
typedef struct MenuStruct
{
    char const * const header;
    char const * const *items;
    ProcessFunction processEvent;
    uint_least8_t const numItems;
    uint_least8_t itemSelected;
    int_least8_t * itemState;
#if defined(KENTEC)
    uint_least8_t buttonHspacePct;
    uint_least8_t buttonVspacePct;
    uint_least8_t buttonWidthPct;
    uint_least8_t buttonHeightPct;
#endif

} Menu_Object;

/* Pointer to a MenuStruct */
typedef Menu_Object *Menu_Handle;

/*----------------------------------------------------------------------------
 * Functions
 * -------------------------------------------------------------------------*/
/* Move to next item of menu */
void Menus_nextItem(Menu_Handle menu);

#endif /* MENUS_H_ */
