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

/** \file LCD.h Contains functions for displaying menus and recogntion
 * results.\n (C) Copyright 2015, Texas Instruments, Inc.
 */

#ifndef LCD_H_
#define LCD_H_

/*----------------------------------------------------------------------------
 * Includes
 * -------------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>

#include "menus.h"

#if defined (KENTEC_TOUCH)
#include "touch.h"
#endif

/*----------------------------------------------------------------------------
 * Defines and Typedefs
 * -------------------------------------------------------------------------*/
#define MAIN_MENU_BUTTON_HSPACE_PCT (5)
#define MAIN_MENU_BUTTON_WIDTH_PCT (42)
#define MAIN_MENU_BUTTON_VSPACE_PCT (5)
#define MAIN_MENU_BUTTON_HEIGHT_PCT (30)

#define MENU_BUTTON_HSPACE_PCT (2)
#define MENU_BUTTON_WIDTH_PCT (30)
#define MENU_BUTTON_VSPACE_PCT (1)
#define MENU_BUTTON_HEIGHT_PCT (15)

#define INACTIVE_COLOR GRAPHICS_COLOR_RED
#define LIVE_COLOR GRAPHICS_COLOR_ORANGE

/*----------------------------------------------------------------------------
 * Functions
 * --------------------------------------------------------------------------*/

/* Setup display */
void LCD_setup(void);

/*-------------------------------------------------------------------------*/
/* Shutdown the display */
void LCD_shutdown();

/*--------------------------------------------------------------------------*/
/* Display a menu */
void LCD_showMenu(Menu_Handle menu);

/*--------------------------------------------------------------------------*/
/* Switch the selected item to a new item. The menu item just chosen may
 * have special handling if the button is "live" */
void LCD_updateSelectedItem(Menu_Handle menu, int_least8_t oldItem,
    int_least8_t newItem, bool live);

/*--------------------------------------------------------------------------*/
/* Display a message at the bottom of the display. Menu items must not be in
 *  this area of the display. If message is NULL, the message area
 *  is cleared. */
void LCD_showMessage(char const *message, uint_least32_t color);

#if defined (KENTEC_TOUCH)
/*--------------------------------------------------------------------------*/
/* Determine if screen touch for menu button */
bool LCD_itemTouched(Menu_Handle menu, int_least8_t menuItem,
        Touch_Info *touch );
#endif


#endif /* LCD_H_ */
