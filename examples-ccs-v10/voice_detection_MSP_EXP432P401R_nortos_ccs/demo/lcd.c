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

/** \file display.c Contains functions for displaying menus and recognition
 * results on the LCD.\n (C) Copyright 2015, Texas Instruments, Inc.
 */

/*----------------------------------------------------------------------------
 * Includes
 * --------------------------------------------------------------------------*/
#include <stdlib.h>
#include <stdbool.h>


#include <ti/devices/msp432p4xx/inc/msp.h>
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

#include <ti/grlib/grlib.h>

#include "lcd.h"

/* LCD Driver module */
#if defined(KENTEC)
#include "LcdDriver/kitronix320x240x16_ssd2119_spi.h"
#else
#include "LcdDriver/Sharp96x96.h"

#endif

#if defined(KENTEC_TOUCH)
#include <ti/grlib/button.h>
#endif


#include "menus.h"

#include "touch.h"

/*----------------------------------------------------------------------------
 * Globals
 * -------------------------------------------------------------------------*/

/* Display context */
Graphics_Context g_sContext;

#if defined(KENTEC_TOUCH)
Graphics_Button g_sButton;
#endif

/*----------------------------------------------------------------------------
 * Functions
 * --------------------------------------------------------------------------*/
#if defined(KENTEC_TOUCH)
static void LCD_fillButtonInfo( Graphics_Context *grContext,  Menu_Handle menu,
        int_least8_t menuItem, Graphics_Button *grButton);

static void LCD_drawButton( Graphics_Context *grContext,  Menu_Handle menu,
        int_least8_t menuItem, Graphics_Button *grButton, bool inactive, bool live);
#endif

/*--------------------------------------------------------------------------*/
/* Set up the LCD */
void LCD_setup()
{

#if defined(KENTEC)

    // LCD setup using Graphics Library API calls
    Kitronix320x240x16_SSD2119Init();
    Graphics_initContext(&g_sContext, &g_sKitronix320x240x16_SSD2119, &g_sKitronix320x240x16_SSD2119_funcs);
    Graphics_setBackgroundColor(&g_sContext, ClrWhite);
    Graphics_setForegroundColor(&g_sContext, ClrBlack);
    Graphics_setFont(&g_sContext, &g_sFontCmss20b);
    Graphics_clearDisplay(&g_sContext);

#if defined(KENTEC_TOUCH)
    /* Default button */
    g_sButton.borderWidth = 1;
    g_sButton.borderColor = GRAPHICS_COLOR_BLACK;
    g_sButton.fillColor = GRAPHICS_COLOR_LAWN_GREEN;
    g_sButton.font = &g_sFontCmss20b;
    g_sButton.selected = false;
    g_sButton.selectedColor = GRAPHICS_COLOR_LAWN_GREEN;
    g_sButton.selectedTextColor = GRAPHICS_COLOR_WHITE;
    g_sButton.textColor = GRAPHICS_COLOR_LIGHT_GRAY;
#endif

#else
    /* Sharp VCOM toggle refresh clock signal */
    Timer_A_UpModeConfig upConfig =
    {
            TIMER_A_CLOCKSOURCE_ACLK,
            TIMER_A_CLOCKSOURCE_DIVIDER_1,
            546,
            TIMER_A_TAIE_INTERRUPT_DISABLE,
            TIMER_A_CCIE_CCR0_INTERRUPT_ENABLE,
            TIMER_A_DO_CLEAR
    };


    /* Initialize GPIO, and SPI EUSCI_B0 for LCD display */
    Sharp96x96_LCDInit();

    /* Set up initial graphics context for LCD */
    Graphics_initContext(&g_sContext, &g_sharp96x96LCD, &g_sharp96x96LCD_funcs);
    Graphics_setForegroundColor(&g_sContext, ClrBlack);
    Graphics_setBackgroundColor(&g_sContext, ClrWhite);
    Graphics_setFont(&g_sContext, &g_sFontFixed6x8);

    /* Set up timer A2 for LCD VCOM toggle signal */
    /* Nominal 29Hz VCOM cycle frequency */
     upConfig.timerPeriod = CS_getACLK()/58;
     MAP_Interrupt_disableInterrupt(INT_TA2_0);
     MAP_Interrupt_disableInterrupt(INT_TA2_N);
     MAP_Timer_A_disableCaptureCompareInterrupt(TIMER_A2_BASE,
         TIMER_A_CAPTURECOMPARE_REGISTER_0);
     MAP_Timer_A_stopTimer(TIMER_A2_BASE);
     MAP_Timer_A_clearCaptureCompareInterrupt(TIMER_A2_BASE,
         TIMER_A_CAPTURECOMPARE_REGISTER_0);


     /* set VCOM interrupt to 7, lowest app priority */
     MAP_Interrupt_setPriority(INT_TA2_0, 7<<5);
     MAP_Timer_A_configureUpMode(TIMER_A2_BASE, &upConfig);

     MAP_Timer_A_startCounter(TIMER_A2_BASE, TIMER_A_UP_MODE);
     MAP_Interrupt_enableInterrupt(INT_TA2_0);
#endif
}

/*-------------------------------------------------------------------------*/
/* Shutdown the display */
void LCD_shutdown()
{
#if defined(KENTEC)
#else
    MAP_Interrupt_disableInterrupt(INT_TA2_0);
    MAP_Timer_A_disableCaptureCompareInterrupt(TIMER_A2_BASE,
        TIMER_A_CAPTURECOMPARE_REGISTER_0);
    MAP_Timer_A_stopTimer(TIMER_A2_BASE);
    MAP_Timer_A_clearCaptureCompareInterrupt(TIMER_A2_BASE,
        TIMER_A_CAPTURECOMPARE_REGISTER_0);
#endif
}

/*--------------------------------------------------------------------------*/
/* Clear the display */
static void LCD_clear()
{
#if defined(KENTEC)
    Graphics_clearDisplay(&g_sContext);
#else
    /* Clear display; must disable timer interrupt during write */
    MAP_Interrupt_disableInterrupt( INT_TA2_0);
    Graphics_clearDisplay(&g_sContext);
    MAP_Interrupt_enableInterrupt(INT_TA2_0);
#endif
}

/*--------------------------------------------------------------------------*/
/* Flush the display buffer to the display */
static void LCD_flush()
{
#if defined(KENTEC)
    Graphics_flushBuffer(&g_sContext);
#else
    /* Flush display buffer; must disable timer interrupt during write */
    MAP_Interrupt_disableInterrupt( INT_TA2_0);
    Graphics_flushBuffer(&g_sContext);
    MAP_Interrupt_enableInterrupt(INT_TA2_0);
#endif
}

/*--------------------------------------------------------------------------*/
/* Clear the display and show the full menu */
void LCD_showMenu(Menu_Handle menu)
{
    uint_fast8_t item;

    uint_fast8_t fontHeight;
    uint_fast16_t displayWidth;
    uint_fast16_t height = 0;

    /* Parameters needed to draw menu with given font */
    displayWidth = Graphics_getDisplayWidth(&g_sContext);
    fontHeight = Graphics_getFontHeight(g_sContext.font);


    /* Remove any prior graphics */
    LCD_clear();

    /* Write header to buffer if it exists */
    if( menu->header != NULL)
    {
        Graphics_drawStringCentered(&g_sContext, (int8_t *)(menu->header),
            GRAPHICS_AUTO_STRING_LENGTH, displayWidth/2, fontHeight/2,
            GRAPHICS_TRANSPARENT_TEXT);
        height += fontHeight + 1;
    }

    /* Write menu items to buffer */
    for(item = 0; item < menu->numItems; item++)
    {

#if defined (KENTEC_TOUCH)
        /* Fill button context with button info for item and draw it */
        LCD_fillButtonInfo(&g_sContext, menu, item, &g_sButton);

        LCD_drawButton(&g_sContext, menu, item, &g_sButton, menu->itemState[item], false);

#else
        if(item == menu->itemSelected)
        {
            /* Selected item in reverse text */
            Graphics_setForegroundColor(&g_sContext, ClrWhite);
            Graphics_setBackgroundColor(&g_sContext, ClrBlack);

            Graphics_drawString(&g_sContext, (int8_t *)(menu->items[item]),
                GRAPHICS_AUTO_STRING_LENGTH, 1, height,
                GRAPHICS_OPAQUE_TEXT);

            /* Return to normal text style */
            Graphics_setForegroundColor(&g_sContext, ClrBlack);
            Graphics_setBackgroundColor(&g_sContext, ClrWhite);

            height += fontHeight + 1;
        }

        else
        {
            /* Non-selected menu items */
            Graphics_drawString(&g_sContext, (int8_t *)(menu->items[item]),
                GRAPHICS_AUTO_STRING_LENGTH, 1, height,
                GRAPHICS_TRANSPARENT_TEXT);

             height += fontHeight + 1;
        }
#endif
    }

    /* Output menu to display*/
    LCD_flush();
}

/*--------------------------------------------------------------------------*/
/* Switch the selected item to a new item */
void LCD_updateSelectedItem(Menu_Handle menu, int_least8_t oldItem,
    int_least8_t newItem, bool live)
{
#if ! defined(KENTEC_TOUCH)
    uint_fast16_t height = 0;
    uint_fast16_t heightStep;
    uint_fast16_t heightBase = 0;
#endif

#if defined(KENTEC_TOUCH)

    /* Return button to original unselected text color and active/inactive background */
    if(oldItem != newItem)
    {
        LCD_fillButtonInfo(&g_sContext, menu, oldItem, &g_sButton);
        LCD_drawButton(&g_sContext, menu, oldItem, &g_sButton, menu->itemState[oldItem], false);
    }

    /* New selected button text color, live/active/inactive background */
    LCD_fillButtonInfo(&g_sContext, menu, newItem, &g_sButton);
    LCD_drawButton(&g_sContext, menu, newItem, &g_sButton, menu->itemState[newItem], live);


#else
    /* Parameters needed to draw menu with given font */
    heightStep = Graphics_getFontHeight(g_sContext.font) + 1;
    if(menu->header != NULL)
    {
        heightBase = heightStep;
    }

    /* Write the old menu item as normal text */
    height = heightBase + (heightStep * oldItem);

    Graphics_drawString(&g_sContext, (int8_t *)(menu->items[oldItem]),
        GRAPHICS_AUTO_STRING_LENGTH, 1, height,
        GRAPHICS_OPAQUE_TEXT);

    /* Write the new menu item as reverse text */
    Graphics_setForegroundColor(&g_sContext, ClrWhite);
    Graphics_setBackgroundColor(&g_sContext, ClrBlack);

    height = heightBase + (heightStep * newItem);

    Graphics_drawString(&g_sContext, (int8_t *)(menu->items[newItem]),
        GRAPHICS_AUTO_STRING_LENGTH, 1, height,
        GRAPHICS_OPAQUE_TEXT);

    /* Return to normal text style */
    Graphics_setForegroundColor(&g_sContext, ClrBlack);
    Graphics_setBackgroundColor(&g_sContext, ClrWhite);
#endif

    /* Flush the changes */
    /* Output menu; must disable timer interrupt during write */
    LCD_flush();
}

/*--------------------------------------------------------------------------*/
/* Display a message at the bottom of the display. Menus must not be in this
 * area of the display. If message is NULL, the message area is cleared. */
void LCD_showMessage(char const *message, uint_least32_t color)
{
    uint_fast16_t displayHeight;
    Graphics_Rectangle rect;

    /* Parameters needed to draw menu with given font */
    displayHeight = Graphics_getDisplayHeight(&g_sContext);

    /* Clear message area */
    rect.xMin = 0;
    rect.xMax = Graphics_getDisplayWidth(&g_sContext) - 1;
    rect.yMin = displayHeight - Graphics_getFontHeight(g_sContext.font);
    rect.yMax = displayHeight - 1;

    Graphics_setForegroundColor(&g_sContext, ClrWhite);
    Graphics_fillRectangle(&g_sContext, &rect);
    Graphics_setForegroundColor(&g_sContext, ClrBlack);

    /* Write new message to bottom of display */
    if( message != NULL)
    {
        Graphics_setForegroundColor(&g_sContext, color);
        Graphics_drawString(&g_sContext, (int8_t *)(message),
            GRAPHICS_AUTO_STRING_LENGTH, 1, rect.yMin,
            GRAPHICS_TRANSPARENT_TEXT);
        Graphics_setForegroundColor(&g_sContext, ClrBlack);
    }

    /* Flush the changes */
    /* Output menu; must disable timer interrupt during write */
    LCD_flush();
}

#if defined(KENTEC_TOUCH)
/*--------------------------------------------------------------------------*/
/* Determine if menu button has been pressed. Input is the menu,
 * menu item to check, and normalized touch location. */
bool LCD_itemTouched(Menu_Handle menu, int_least8_t menuItem,
        Touch_Info *touch)
{
    uint_fast16_t displayHeight;
    uint_fast16_t displayWidth;
    uint_least16_t touchX;
    uint_least16_t touchY;

    /* Graphics context parameters */
    /* Parameters needed to draw menu with given font */
    displayWidth = Graphics_getDisplayWidth(&g_sContext);
    displayHeight = Graphics_getDisplayHeight(&g_sContext);

    LCD_fillButtonInfo( &g_sContext, menu, menuItem, &g_sButton );

    touchX = (uint16_t)( ((uint_least32_t)(displayWidth)*touch->locationX) >> 14);
    touchY = (uint16_t)( ((uint_least32_t)(displayHeight)*touch->locationY) >> 14);

    return Graphics_isButtonSelected( &g_sButton, touchX, touchY);
}
#endif


#if defined (KENTEC_TOUCH)
/*--------------------------------------------------------------------------*/
/* Fill a button context with its dimensions and text. */
void LCD_fillButtonInfo( Graphics_Context *grContext,  Menu_Handle menu,
        int_least8_t menuItem, Graphics_Button *grButton)
{
    uint_fast16_t displayHeight;
    uint_fast16_t displayWidth;
    uint_fast8_t fontHeight;

    uint_fast16_t hSpace;
    uint_fast16_t vSpace;
    uint_fast16_t buttonWidth;
    uint_fast16_t buttonHeight;

    uint_fast16_t buttonsPerRow;
    uint_fast16_t buttonRow;
    uint_fast16_t buttonCol;
    int_least32_t textWidth;

    /* Graphics context parameters */
    /* Parameters needed to draw menu with given font */
    displayWidth = Graphics_getDisplayWidth(grContext);
    fontHeight = Graphics_getFontHeight(grContext->font);
    displayHeight = Graphics_getDisplayHeight(grContext);

    hSpace = ((uint_least32_t)(displayWidth) * menu->buttonHspacePct / 100);
    buttonWidth = ((uint_least32_t)(displayWidth) * menu->buttonWidthPct / 100);
    vSpace = ((uint_least32_t)(displayHeight) * menu->buttonVspacePct / 100);
    buttonHeight = ((uint_least32_t)(displayHeight) * menu->buttonHeightPct / 100);


    /* Zero-based row button is in */
    buttonsPerRow = displayWidth/(hSpace + buttonWidth);
    buttonRow = menuItem/buttonsPerRow;
    buttonCol = menuItem - buttonsPerRow*buttonRow;

    /* Horizontal button location */
    grButton->xMin = (buttonCol + 1)* hSpace + buttonCol*buttonWidth;
    grButton->xMax = grButton->xMin + buttonWidth - 1;

    /* Vertical button location */
    if(menu->header != NULL)
    {
        grButton->yMin = fontHeight;
    }
    else
    {
        grButton->yMin = 1;
    }
    grButton->yMin += (buttonRow + 1)*vSpace + buttonRow*buttonHeight;
    grButton->yMax = grButton->yMin + buttonHeight - 1;

    grButton->text = (int8_t *)(menu->items[menuItem]);
    textWidth = Graphics_getStringWidth(grContext, grButton->text, -1);
    grButton->textXPos = grButton->xMin + (buttonWidth - textWidth)/2;
    grButton->textYPos = grButton->yMin + (buttonHeight - fontHeight)/2;

    grButton->selected = (menu->itemSelected == menuItem);
}


/*--------------------------------------------------------------------------*/
/* Draw a button context with its color based on model state */

void LCD_drawButton( Graphics_Context *grContext,  Menu_Handle menu,
        int_least8_t menuItem, Graphics_Button *grButton, bool inactive, bool live)
{
    uint32_t fillColor;
    uint32_t selColor;


    /* Retain original button colors */
    fillColor = grButton->fillColor;
    selColor = grButton->selectedColor;

    /* Phrase buttons: color unenrolled phrases green */
    if( inactive )
    {
        grButton->fillColor = INACTIVE_COLOR;
        grButton->selectedColor = INACTIVE_COLOR;
    }
    else if(live)
    {
        grButton->selectedColor = LIVE_COLOR;
    }

    Graphics_drawButton(grContext, grButton);

    /* Reinstate original button colors */
    grButton->fillColor = fillColor;
    grButton->selectedColor = selColor;
}

#endif

#if defined(KENTEC)
#else
/*--------------------------------------------------------------------------*/
/* Interrupt handler for software LCD VCOM toggle */
void TA2_0_IRQHandler(void)
{
    /* Toggle VCOM signal at approximately 29 Hz rate */
    Sharp96x96_SendToggleVCOMCommand();

    MAP_Timer_A_clearCaptureCompareInterrupt(TIMER_A2_BASE,
        TIMER_A_CAPTURECOMPARE_REGISTER_0);
}
#endif
