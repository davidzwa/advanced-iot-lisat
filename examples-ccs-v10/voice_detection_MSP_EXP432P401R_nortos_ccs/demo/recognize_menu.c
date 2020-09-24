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

/** \file updateMenu.c Defines the update menu and operations it invokes.\n (C)
 *  Copyright 2015, Texas Instruments, Inc.
 */

/*----------------------------------------------------------------------------
 * Includes
 * --------------------------------------------------------------------------*/
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <ti/devices/msp432p4xx/inc/msp.h>

#include <ti/grlib/grlib.h>
#include <ti/voice_detection/vcd.h>

/* APIs used in recognize menu */
#include "menus.h"

#include "audio_collect.h"
#include "leds.h"
#include "lcd.h"

#if defined(KENTEC_TOUCH)
#include "touch.h"
#endif

/*----------------------------------------------------------------------------
 * Defines and Typedefs
 * -------------------------------------------------------------------------*/
/* For specifying maximum name string length plus optional '?' for low
 * confidence score. Includes trailing NULL */
#define MAX_NAME_LENGTH (16+1)

/* Confidence threshold before questioning result */
#define RECOGNIZE_CONF_THRESHOLD (20)

/* Confidence thresholds for scoring */
#define RECOGNIZE_CONF_REJECT (-100)
#define RECOGNIZE_CONF_ACCEPT (20)

/* Recognize menu state structure*/
typedef struct recognizeStateStruct
{
    int_least16_t ledCount;
    bool running;
} recognizeStateObject;

/*----------------------------------------------------------------------------
 * Globals
 * -------------------------------------------------------------------------*/

/* Handles for manipulating audio and stream collection */
extern Audio_Handle Audio;

/* Handle for using the MinHMM instance */
extern VCD_Handle VCD_handle;

#if defined (KENTEC_TOUCH)
/* Handle for manipulating touch screen */
extern Touch_Handle Touch;
#endif

/* Other menus used */
extern Menu_Object MainMenu;

/* Recognize menu defined */
static char const * const recognizeItems[] =
{
    "Menu Up",
    "Start/Stop"
};

/* Recognize menu item state, 1=inactive, 0=available */
static int_least8_t itemState[2] = {0, 0};

/* Recognize event handling function declared */
Menu_Handle RecognizeMenu_processEvent(Events event);

/* Update menu instance */
Menu_Object RecognizeMenu =
{
    "Recognize Menu",
    recognizeItems,
    RecognizeMenu_processEvent,
    //Number of items
    2,
    // Initial active item
    0,

    itemState
#if defined (KENTEC)
    // Button spacings
    ,
    5,
    5,
    42,
    30
#endif
};

/* Recognize menu state instance */
static recognizeStateObject recognizeState =
{
    0,
    false
};

/*----------------------------------------------------------------------------
 * Functions
 * -------------------------------------------------------------------------*/
Menu_Handle RecognizeMenu_processEvent(Events event)
{
    VCD_MessageEnum vcdMsg;
    VCD_RecoStruct recoResult;
    int_least16_t *audioFrame;

#if ! defined(KENTEC)
    char nameString[MAX_NAME_LENGTH];
#endif

    switch(event)
    {

#if defined(KENTEC_TOUCH)
        case EVENT_TOUCH:
        {
            Touch_Info touchLoc;
            int_least8_t item = RecognizeMenu.numItems;
            bool wasRunning = false;

            /* Touch occurred, so stop any ongoing recognition. Need to do
             * this immediately so ADC freed to determine touch location */
            if(recognizeState.running)
            {
                wasRunning = true;
                /* End recognition, and show message stating so */
                Audio_stopCollect(Audio);
                vcdMsg = VCD_stopSearch(VCD_handle);
                recognizeState.running = false;
                recognizeState.ledCount = 0;
                Leds_off1();
                Leds_setColor2(0, 0, 0);


                 if(vcdMsg != VCD_MsgNone)
                 {
                     LCD_showMessage("Stop Fail", 0);
                 }
                 else
                 {
                     LCD_showMessage("Ended Recognize", 0);
                 }

                 /* Recognition shut down, not active */
                 LCD_updateSelectedItem(&RecognizeMenu, 1, 1, false );

            }

            /* Now get normalized touch location */
            Touch_getLocation(Touch, &touchLoc, true);

            /* Determine which soft item button was pushed if any */
            if(touchLoc.validLocation)
            {
                for( item = 0; item < RecognizeMenu.numItems; item ++)
                {
                    if( LCD_itemTouched(&RecognizeMenu, item, &touchLoc) )
                    {
                        break;
                    }
                }
            }

            /* Reenable touch detections */
            Touch_enableDetect(Touch, true);

            /* Take action based on soft button that was pushed */
            if( item < RecognizeMenu.numItems)
            {
                if(item == 0)
                {
                    /* Go back to main menu */
                    RecognizeMenu.itemSelected = item;
                    LCD_showMenu(&MainMenu);
                    return &MainMenu;
                }

                else if( !wasRunning )
                {
                    /* Try to start recognition and display resultant message */
                    vcdMsg = VCD_startSearch(VCD_handle);

                    if( vcdMsg != VCD_MsgNone)
                    {
                        /* Return to main menu if failure to start recognition */
                        RecognizeMenu.itemSelected = 0;
                        LCD_showMessage("Start Fail", 0);
                        LCD_showMenu(&MainMenu);
                        return &MainMenu;
                    }
                    else
                    {
                        /* Show live button */
                        uint_least8_t oldItem = RecognizeMenu.itemSelected;
                        RecognizeMenu.itemSelected = item;
                        LCD_updateSelectedItem(&RecognizeMenu,
                                oldItem,
                                RecognizeMenu.itemSelected,
                                true);

                        Audio_startCollect(Audio);
                        recognizeState.running = true;
                        recognizeState.ledCount = 0;
                        LCD_showMessage(" Recognizing!", 0);

                    }
                }
                else
                {
                    /* Request to stop the recognizer. Change button color. */
                    LCD_updateSelectedItem(&RecognizeMenu,
                            RecognizeMenu.itemSelected,
                            RecognizeMenu.itemSelected,
                            false);
                }
            }
        }
        break;

#endif

        case EVENT_BUTTON1:
        {
            if( !recognizeState.running)
            {
                /* Go to next menu item if reco not running. */
                Menus_nextItem(&RecognizeMenu);
            }
        }
        break;


        case EVENT_BUTTON2:
        {
            /* Recognizer running and therefore start/stop item selected */
            if(recognizeState.running)
            {
                /* End recognition, and show message stating so */
                Audio_stopCollect(Audio);
                vcdMsg = VCD_stopSearch(VCD_handle);
                recognizeState.running = false;
                recognizeState.ledCount = 0;
                Leds_off1();
                Leds_setColor2(0, 0, 0);

                /* Set start/stop item to not be live */
                LCD_updateSelectedItem(&RecognizeMenu,
                     RecognizeMenu.itemSelected,
                     RecognizeMenu.itemSelected,
                     false);

                 if(vcdMsg != VCD_MsgNone)
                 {
                     LCD_showMessage("Stop Fail", 0);
                 }
                 else
                 {
                     LCD_showMessage("Ended Recognize", 0);
                 }

//                 LCD_showMenu(&MainMenu);
//                 return &MainMenu;
            }

            else if(RecognizeMenu.itemSelected == 0)
            {
                /* Go back to main menu */
                LCD_showMenu(&MainMenu);
                return &MainMenu;
            }

            else
            {
                /* Try to start recognition and display resultant message */
                vcdMsg = VCD_startSearch(VCD_handle);

                if( vcdMsg != VCD_MsgNone)
                {
                    /* Used to return to main menu if failure to start recognition */
                    LCD_showMessage("Start Fail", 0);
//                    LCD_showMenu(&MainMenu);
//                    return &MainMenu;
                }
                else
                {
#if defined (KENTEC_TOUCH)
                    /* Show live button */
                    LCD_updateSelectedItem(&RecognizeMenu,
                            RecognizeMenu.itemSelected,
                            RecognizeMenu.itemSelected,
                            true);
#endif
                    Audio_startCollect(Audio);
                    recognizeState.running = true;
                    recognizeState.ledCount = 0;
                    LCD_showMessage(" Recognizing!", 0);
                }
            }
        }
        break;


        case EVENT_AUDIOFRAME:
        {
            if(recognizeState.running)
            {
                /* Shut off recognition LED after one second */
                if(recognizeState.ledCount > 0)
                {
                    recognizeState.ledCount--;
                    if(recognizeState.ledCount == 0)
                    {
                        Leds_setColor2(0, 0, 0);
                        LCD_showMessage(NULL, 0);
                    }
                }

                /* Process next frame of recognition data. Display status
                 * based on result of processing. */
                audioFrame = Audio_getActiveBuffer(Audio);
                vcdMsg = VCD_processFrame(VCD_handle, audioFrame);

                switch(vcdMsg)
                {
                    case VCD_MsgRecoVAD:
                    {
                        /* continue processing to find start of utterance */
                        Leds_off1();
                    }
                    break;

                    case VCD_MsgRecoRunning:
                    {
                        /* recognition now ongoing */
                        Leds_on1();
                    }
                    break;

                    case VCD_MsgRecoEnd:
                    {
                        Audio_stopCollect(Audio);
                        VCD_stopSearch(VCD_handle);
                        recognizeState.running = false;
                        recognizeState.ledCount = 0;
                        Leds_off1();
                        Leds_setColor2(0, 0, 0);

                    #if defined (KENTEC_TOUCH)
                        /* Turn off live item */
                        LCD_updateSelectedItem(&RecognizeMenu,
                                RecognizeMenu.itemSelected,
                                RecognizeMenu.itemSelected,
                                false);
                    #endif
                        LCD_showMessage("Recognize Fail", 0);
//                        LCD_showMenu(&MainMenu);
//                        return &MainMenu;
                    }
                    break;

                    case VCD_MsgRecoComplete:
                    {
                        VCD_getResult(VCD_handle, &recoResult);

                        if( recoResult.status == VCD_RecoSuccess ||
                            (recoResult.status == VCD_RecoLowScore &&
                                recoResult.name != NULL))
                        {
                            /* Light color LED if non-filler model */
                            if( recoResult.modelIndex != 0)
                            {
                                if(recoResult.confScore < RECOGNIZE_CONF_REJECT)
                                {
                                    Leds_setColor2(255, 0, 0);
#if defined(KENTEC)
                                    LCD_showMessage( recoResult.name, GRAPHICS_COLOR_RED);
#endif
                                }
                                else if( recoResult.confScore < RECOGNIZE_CONF_ACCEPT)
                                {
                                    Leds_setColor2(255, 255, 0);
#if defined(KENTEC)
                                    LCD_showMessage( recoResult.name, GRAPHICS_COLOR_ORANGE);
#endif
                                }
                                else
                                {
                                    Leds_setColor2(0, 255, 0);
#if defined(KENTEC)
                                    LCD_showMessage( recoResult.name, GRAPHICS_COLOR_LAWN_GREEN);
#endif
                                }

#if ! defined(KENTEC)
                                /* Display recognized non-filler result */
                                if(recoResult.confScore > RECOGNIZE_CONF_THRESHOLD)
                                {
                                    LCD_showMessage(recoResult.name, 0);
                                }
                                else
                                {
                                    strcpy(nameString, recoResult.name);
                                    strcat(nameString, "?");
                                    LCD_showMessage(nameString, 0);
                                }
#endif

                                /* Set led 1 sec timeout */
                                recognizeState.ledCount = 50;
                            }
                        }

                        /* Start search for next recognized phrase */
                        VCD_resetSearch(VCD_handle);
                        Leds_off1();
                    }
                    break;
                    default:
                    break;
                }
            }

            else
            {
                /* Should not get an audio frame while not recognizing */
                Audio_stopCollect(Audio);
                recognizeState.ledCount = 0;
                Leds_off1();
                Leds_setColor2(0, 0, 0);

                LCD_showMessage("Recognize Error", 0);
//                LCD_showMenu(&MainMenu);
//               return &MainMenu;
            }
        }
        break;
        default:
        break;
    }

    return &RecognizeMenu;
}
