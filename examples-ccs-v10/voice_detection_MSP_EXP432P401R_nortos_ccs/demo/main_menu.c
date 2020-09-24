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

/** \file mainMenu.c Defines the main menu and operations it invokes.\n (C)
 *  Copyright 2015, Texas Instruments, Inc.
 */

/*----------------------------------------------------------------------------
 * Includes
 * --------------------------------------------------------------------------*/
#include <stdlib.h>
#include <stdbool.h>

#include <ti/devices/msp432p4xx/inc/msp.h>

/* APIs used in main menu */
#include "menus.h"

#include "audio_collect.h"
#include "lcd.h"

#if defined(KENTEC_TOUCH)
#include "touch.h"
#endif

/*----------------------------------------------------------------------------
 * Defines and Typedefs
 * -------------------------------------------------------------------------*/
/* Main menu state structure*/
#if defined (KENTEC_TOUCH)
typedef struct mainStateStruct
{

    Touch_Info  touchHist[4];

} mainStateObject;
#endif

/*----------------------------------------------------------------------------
 * Globals
 * -------------------------------------------------------------------------*/

/* Handle for manipulating audio collection */
extern Audio_Handle Audio;


#if defined (KENTEC_TOUCH)
/* Handle for manipulating touch screen */
extern Touch_Handle Touch;
#endif

/* Other menus used */
extern Menu_Object EnrollMenu;
extern Menu_Object UpdateMenu;
extern Menu_Object DeleteMenu;
extern Menu_Object RecognizeMenu;

/* Main menu defined */
static char const * const mainItems[] =
{
    "Enroll Model",
    "Update Model",
    "Delete Model",
    "Recognize",
#ifdef ALLOWEXIT
    "Exit"
#endif
};


/* Main menu item state 0=active, 1=inactive */
static int_least8_t mainItemState[5] = {0, 0, 0, 0, 0};

#if defined (KENTEC_TOUCH)
/* Main state contains last four touch items for calibration if using Kentec touch */
static mainStateObject  mainState =
{
        {
                { 0, 0, false},
                { 0, 0, false},
                { 0, 0, false},
                { 0, 0, false}
        }

};
#endif


Menu_Handle MainMenu_processEvent(Events event);

Menu_Object MainMenu =
{
    "Main Menu",
    mainItems,
    MainMenu_processEvent,

    // Number of items in menu
#ifdef ALLOWEXIT
    5,
#else
    4,
#endif

    // Initial selected menu item
    0,

    // State of each item
    mainItemState

#if defined (KENTEC)
    // Button spacings
    ,
    5,
    5,
    42,
    30
#endif
};

/*----------------------------------------------------------------------------
 * Functions
 * -------------------------------------------------------------------------*/
Menu_Handle MainMenu_processEvent(Events event)
{
    switch(event)
    {
#if defined(KENTEC_TOUCH)
        case EVENT_TOUCH:
        {
            int_fast16_t i;
            Touch_Info touchLoc;
            int_least8_t item = MainMenu.numItems;

            /* Main state touch info */
            Touch_Info *touchHist = mainState.touchHist;

            /* Immediately get raw touch location */
            Touch_getLocation(Touch, &touchLoc, false);

            /* Update the history array */
            for(i = 0; i < 3; i++ )
            {
                touchHist[i] = touchHist[i+1];
            }
            touchHist[3] = touchLoc;

            if(touchLoc.validLocation)
            {
                /* Check for automatic calibration */
                Touch_calibrate(Touch, touchHist);

                /* Determine which soft item button was pushed if any */
                Touch_normalizeLocation( Touch, &touchLoc );
                for( item = 0; item < MainMenu.numItems; item ++)
                {
                    if( LCD_itemTouched(&MainMenu, item, &touchLoc) )
                    {
                        break;
                    }
                }
            }

            /* Reenable touch detections */
            Touch_enableDetect(Touch, true);

            /* Go to new menu if a valid soft button was pushed */
            if( item < MainMenu.numItems)
            {
                MainMenu.itemSelected = item;

                /* Go to the next menu level */
                switch(MainMenu.itemSelected)
                {
                case 0:
                    LCD_showMenu(&EnrollMenu);
                    return &EnrollMenu;
                case 1:
                    LCD_showMenu(&UpdateMenu);
                    return &UpdateMenu;
                case 2:
                    LCD_showMenu(&DeleteMenu);
                    return &DeleteMenu;
                case 3:
                    LCD_showMenu(&RecognizeMenu);
                    return &RecognizeMenu;
                }
            }
        }
        break;

#endif

        case EVENT_BUTTON1:
        {
            /* Next menu item */
            Menus_nextItem(&MainMenu);
        }
        break;

        case EVENT_BUTTON2:
        {
            /* Go to the next menu level */
            switch(MainMenu.itemSelected)
            {
                case 0:
                    LCD_showMenu(&EnrollMenu);
                    return &EnrollMenu;
                case 1:
                    LCD_showMenu(&UpdateMenu);
                    return &UpdateMenu;
                case 2:
                    LCD_showMenu(&DeleteMenu);
                    return &DeleteMenu;
                case 3:
                    LCD_showMenu(&RecognizeMenu);
                    return &RecognizeMenu;
                case 4:
                    return (Menu_Handle)(NULL);
            }
        }
        break;

        case EVENT_AUDIOFRAME:
        {
            /* Should not happen in main menu */
            Audio_stopCollect(Audio);
            Audio_shutdownCollect(Audio);
        }
        break;
        default:
        break;        
    }

    return &MainMenu;
}


