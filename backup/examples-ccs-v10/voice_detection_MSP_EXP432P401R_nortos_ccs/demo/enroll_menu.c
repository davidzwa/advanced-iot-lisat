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

/** \file enrollMenu.c Defines the enroll menu and operations it invokes.\n (C)
 *  Copyright 2015, Texas Instruments, Inc.
 */

/*----------------------------------------------------------------------------
 * Includes
 * --------------------------------------------------------------------------*/
#include <stdlib.h>
#include <stdbool.h>

#include <ti/devices/msp432p4xx/inc/msp.h>
#include <ti/grlib/grlib.h>
#include <ti/voice_detection/vcd.h>

/* APIs used in enroll menu */
#include "menus.h"
#include "audio_collect.h"
#include "lcd.h"


#if defined(KENTEC_TOUCH)
#include "touch.h"
#endif

/*----------------------------------------------------------------------------
 * Defines and Typedefs
 * -------------------------------------------------------------------------*/
/* Enroll menu state structure*/
typedef struct enrollStateStruct
{
    uint_least32_t frameCount;
    bool running;
} enrollStateObject;

/*----------------------------------------------------------------------------
 * Globals
 * -------------------------------------------------------------------------*/

/* Handle for using the Audio API instance */
extern Audio_Handle Audio;

/* Handle for using the MinHMM instance */
extern VCD_Handle VCD_handle;

#if defined (KENTEC_TOUCH)
/* Handle for manipulating touch screen */
extern Touch_Handle Touch;
#endif

/* Other menus used */
extern Menu_Object MainMenu;
extern Menu_Object UpdateMenu;
extern Menu_Object DeleteMenu;


/* Enroll menu defined */
static char const * const enrollItems[] =
{
    "Menu Up",
    "Word1",
    "Word2",
    "Word3",
    "Word4",
    "Word5",
    "Word6",
    "Word7",
    "Word8"
};

/* Enroll menu item state, 1=enrolled, 0=available */
static int_least8_t itemState[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};

/* Enroll menu state */
static enrollStateObject enrollState =
{
    0,
    false,
};

/* Enroll event handling function declared */
Menu_Handle EnrollMenu_processEvent(Events event);

/* Enroll menu instance */
Menu_Object EnrollMenu =
{
    "Enroll Model",
    enrollItems,
    EnrollMenu_processEvent,
    // Number of items
    9,
    // Initial active item
    0,

    // enroll item state
    itemState

#if defined (KENTEC)
    // Button spacings
    ,
    2,
    1,
    30,
    15
#endif
};



/*----------------------------------------------------------------------------
 * Functions
 * -------------------------------------------------------------------------*/
Menu_Handle EnrollMenu_processEvent(Events event)
{
    VCD_MessageEnum vcdMsg;
    int_least16_t *audioFrame;
    VCD_ModelInfoStruct mInfo;
    int_least16_t quality;
    char lcdMessage[17];


    switch(event)
    {
#if defined(KENTEC_TOUCH)
        case EVENT_TOUCH:
        {
            Touch_Info touchLoc;
            int_least8_t item = EnrollMenu.numItems;

            /* Touch occurred, so stop any ongoing enrollment. Need to do
             * this immediately so ADC freed to determine touch location */
            if(enrollState.running)
            {
                /* End enrollment, and show message stating so */
                Audio_stopCollect(Audio);
                vcdMsg = VCD_stopEnroll(VCD_handle);
                enrollState.running = false;

                /* No active menu item */
                LCD_updateSelectedItem(&EnrollMenu,
                    EnrollMenu.itemSelected, EnrollMenu.itemSelected,
                    false);

                if(vcdMsg != VCD_MsgNone)
                {
                    LCD_showMessage("Stop Fail", 0);
                }
                else
                {
                    LCD_showMessage("Ended Enroll", 0);
                }
            }

            /* Now get normalized touch location */
            Touch_getLocation(Touch, &touchLoc, true);

            /* Determine which soft item button was pushed if any */
            if(touchLoc.validLocation)
            {
                for( item = 0; item < EnrollMenu.numItems; item ++)
                {
                    if( LCD_itemTouched(&EnrollMenu, item, &touchLoc) )
                    {
                        break;
                    }
                }
            }

            /* Reenable touch detections */
            Touch_enableDetect(Touch, true);

            /* Take action based on soft button that was pushed */
            if( item < EnrollMenu.numItems)
            {
                if(item == 0)
                {
                    /* Return to menu up selection on exit from enroll menu */
                    uint_least8_t oldItem = EnrollMenu.itemSelected;
                    EnrollMenu.itemSelected = item;
                    LCD_updateSelectedItem(&EnrollMenu,
                            oldItem, EnrollMenu.itemSelected,
                            false);

                    /* Go back to main menu */
                    LCD_showMenu(&MainMenu);
                    return &MainMenu;
                }

                else
                {
                    /* Start enrollment of selected item if possible, and display
                     * resultant message. */
                    vcdMsg = VCD_startEnroll(VCD_handle,
                        EnrollMenu.items[item]);

                    if( vcdMsg != VCD_MsgNone)
                    {
                        LCD_showMessage("Start Fail", 0);
                    }
                    else
                    {
                        uint_least16_t oldItem = EnrollMenu.itemSelected;
                        EnrollMenu.itemSelected = item;
                        LCD_updateSelectedItem(&EnrollMenu,
                                oldItem, EnrollMenu.itemSelected,
                                true);

                        enrollState.running = true;
                        enrollState.frameCount = 0;
                        LCD_showMessage(" Enrolling!", 0);
                        Audio_startCollect(Audio);

                    }
                }
            }
        }
        break;

#endif

    case EVENT_BUTTON1:
        {
            if(!enrollState.running)
            {
                /* Go to next menu item if enroll not running */
                Menus_nextItem(&EnrollMenu);
            }
        }
        break;


        case EVENT_BUTTON2:
        {
            if(enrollState.running)
            {
                uint_least8_t oldItem;
                /* End enrollment, and show message stating so */
                Audio_stopCollect(Audio);
                vcdMsg = VCD_stopEnroll(VCD_handle);
                enrollState.running = false;

                /* Set selected item to go to main menu, no live item */
                oldItem = EnrollMenu.itemSelected;
                EnrollMenu.itemSelected = 0;
                LCD_updateSelectedItem(&EnrollMenu,
                    oldItem, EnrollMenu.itemSelected,
                    false);

                if(vcdMsg != VCD_MsgNone)
                {
                    LCD_showMessage("Stop Fail", 0);
                }
                else
                {
                    LCD_showMessage("Ended Enroll", 0);
                }

            }

            else if(EnrollMenu.itemSelected == 0)
            {
                /* Go back to main menu */
                LCD_showMenu(&MainMenu);
                return &MainMenu;
            }

            else
            {
                /* Start enrollment of selected item if possible, and display
                 * resultant message. */
                vcdMsg = VCD_startEnroll(VCD_handle,
                    EnrollMenu.items[EnrollMenu.itemSelected]);

                if( vcdMsg != VCD_MsgNone)
                {
                    LCD_showMessage("Start Fail", 0);
                }
                else
                {

#if defined (KENTEC_TOUCH)
                    /* Show live button */
                    LCD_updateSelectedItem(&EnrollMenu,
                            EnrollMenu.itemSelected,
                            EnrollMenu.itemSelected,
                            true);
#endif
                    Audio_startCollect(Audio);
                    enrollState.running = true;
                    enrollState.frameCount = 0;
                    LCD_showMessage(" Enrolling!", 0);
                }
            }
        }
        break;


        case EVENT_AUDIOFRAME:
        {
            if(enrollState.running)
            {
                /* Do not use first 5 frames allowing for audio to stabilize
                 * and button push noise to dissipate. */
                if(enrollState.frameCount < 5)
                {
                    enrollState.frameCount++;
                    break;
                }

                /* Process next frame of enrollment data. If enrollment status
                 * indicates some ending, then display a result message and
                 * shut down enrollment. */
                audioFrame = Audio_getActiveBuffer(Audio);
                vcdMsg = VCD_processFrame(VCD_handle, audioFrame);

                switch(vcdMsg)
                {
                    case VCD_MsgEnrollTooShort:
                    case VCD_MsgNone:
                    {
                        /* continue processing to find enrollment utterance */
                    }
                    break;

                    case VCD_MsgFail:
                    case VCD_MsgEnrollTooLong:
                    {
                        Audio_stopCollect(Audio);
                        VCD_stopEnroll(VCD_handle);
                        enrollState.running = false;

#if defined (KENTEC_TOUCH)
                        /* Turn off live item */
                        LCD_updateSelectedItem(&EnrollMenu,
                                EnrollMenu.itemSelected,
                                EnrollMenu.itemSelected,
                                false);
#endif

                        if(vcdMsg == VCD_MsgFail)
                        {
                            LCD_showMessage("Enroll Fail", 0);
                        }
                        else
                        {
                            LCD_showMessage("Enroll Long", 0);
                        }
                    }
                    break;

                    case VCD_MsgKeywordFound:
                    {
                        uint_least8_t oldItem;

                        Audio_stopCollect(Audio);
                        enrollState.running = false;
                        vcdMsg = VCD_addEnroll(VCD_handle);


                        if(vcdMsg != VCD_MsgNone)
                         {
                             VCD_stopEnroll(VCD_handle);
                             LCD_showMessage("Add Fail", 0);
                         }
                         else
                         {
                             // Set button item inactive - it has been successfully enrolled
                             EnrollMenu.itemState[EnrollMenu.itemSelected] = 1;
                             DeleteMenu.itemState[EnrollMenu.itemSelected] = 0;
                             UpdateMenu.itemState[EnrollMenu.itemSelected] = 0;

                             VCD_getModelInfo(VCD_handle, EnrollMenu.items[EnrollMenu.itemSelected],
                                     0, &mInfo);
                             VCD_getEnrollQuality(VCD_handle, &quality);
                             quality = (int_least16_t)( (((int_least32_t)quality)*10)/32768 + 1 );

                             sprintf(lcdMessage, "Success %d %d", mInfo.numObs*2, quality);
                             LCD_showMessage(lcdMessage, 0);

                             VCD_stopEnroll(VCD_handle);
                          }

                        /* Set selected item to go to main menu, no live item */
                        oldItem = EnrollMenu.itemSelected;
                        EnrollMenu.itemSelected = 0;
                        LCD_updateSelectedItem(&EnrollMenu,
                            oldItem, EnrollMenu.itemSelected,
                            false);


                    }
                    break;
                    default:
                    break;
                }
            }

            else
            {
                /* Not running, should not get an audio frame */
                Audio_stopCollect(Audio);
                enrollState.running = false;
                LCD_showMessage("Enroll Error", 0);
            }
        }
        break;
        default:
        break;
    }

    return &EnrollMenu;
}

