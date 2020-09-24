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
#include <stdbool.h>

#include <ti/devices/msp432p4xx/inc/msp.h>
#include <ti/grlib/grlib.h>
#include <ti/voice_detection/vcd.h>

/* APIs used in update menu */
#include "audio_collect.h"
#include "lcd.h"

#if defined(KENTEC_TOUCH)
#include "touch.h"
#endif

/*----------------------------------------------------------------------------
 * Defines and Typedefs
 * -------------------------------------------------------------------------*/

/* Update menu state structure*/
typedef struct updateStateStruct
{
    uint_least32_t frameCount;
    bool running;
} updateStateObject;

/*----------------------------------------------------------------------------
 * Globals
 * -------------------------------------------------------------------------*/

/* Handle for manipulating audio collection */
extern Audio_Handle Audio;

/* Handle for using the MinHMM instance */
extern VCD_Handle VCD_handle;

#if defined (KENTEC_TOUCH)
/* Handle for manipulating touch screen */
extern Touch_Handle Touch;
#endif

/* Other menus used */
extern Menu_Object MainMenu;

/* Update menu defined */
static char const * const updateItems[] =
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

/* Update menu item state 0=can be deleted, 1=no model available */
static int_least8_t itemState[9] = {0, 1, 1, 1, 1, 1, 1, 1, 1};

/* Update event handling function declared */
Menu_Handle UpdateMenu_processEvent(Events event);

/* Update menu instance */
Menu_Object UpdateMenu =
{
    "Update Model",
    updateItems,
    UpdateMenu_processEvent,
    // Number of items
    9,
    // Initial active item
    0,

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

/* Update menu state */
static updateStateObject updateState =
{
    0,
    false
};

/*----------------------------------------------------------------------------
 * Functions
 * -------------------------------------------------------------------------*/
Menu_Handle UpdateMenu_processEvent(Events event)
{
    VCD_MessageEnum vcdMsg;
    int_least16_t *audioFrame;
    VCD_ModelInfoStruct mInfo;

    switch(event)
    {
#if defined(KENTEC_TOUCH)
        case EVENT_TOUCH:
        {
            Touch_Info touchLoc;
            int_least8_t item = UpdateMenu.numItems;

            /* Touch occurred, so stop any ongoing update. Need to do
             * this immediately so ADC freed to determine touch location */
            if(updateState.running)
            {
                /* End update, and show message stating so. */
                Audio_stopCollect(Audio);
                vcdMsg = VCD_stopUpdate(VCD_handle);
                updateState.running = false;

                /* No active menu item */
                LCD_updateSelectedItem(&UpdateMenu,
                    UpdateMenu.itemSelected, UpdateMenu.itemSelected,
                    false);

                if(vcdMsg != VCD_MsgNone)
                {
                    LCD_showMessage("Stop Fail", 0);
                }
                else
                {
                    LCD_showMessage("Ended Update", 0);
                }
            }

            /* Now get normalized touch location */
            Touch_getLocation(Touch, &touchLoc, true);

            /* Determine which soft item button was pushed if any */
            if(touchLoc.validLocation)
            {
                for( item = 0; item < UpdateMenu.numItems; item ++)
                {
                    if( LCD_itemTouched(&UpdateMenu, item, &touchLoc) )
                    {
                        break;
                    }
                }
            }

            /* Reenable touch detections */
            Touch_enableDetect(Touch, true);

            /* Take action based on soft button that was pushed */
            if( item < UpdateMenu.numItems)
            {
                if(item == 0)
                {
                    /* Go back to main menu */
                    UpdateMenu.itemSelected = 0;
                    LCD_showMenu(&MainMenu);
                    return &MainMenu;
                }

                else
                {
                    /* Start update of selected item if possible, and display
                     * resultant message.*/
                    vcdMsg = VCD_startUpdate(VCD_handle,
                        UpdateMenu.items[item]);

                    if( vcdMsg != VCD_MsgNone)
                    {
                        LCD_showMessage("Start Fail", 0);
                    }
                    else
                    {
                          uint_least16_t oldItem = UpdateMenu.itemSelected;
                        UpdateMenu.itemSelected = item;
                        LCD_updateSelectedItem(&UpdateMenu,
                                oldItem, UpdateMenu.itemSelected,
                                true);

                        Audio_startCollect(Audio);
                        updateState.running = true;
                        updateState.frameCount = 0;
                        LCD_showMessage(" Updating!", 0);
                    }
                }
            }
        }
        break;

#endif


        case EVENT_BUTTON1:
        {
            if( !updateState.running)
            {
                /* Go to next menu item if update not running */
                Menus_nextItem(&UpdateMenu);
            }
        }
        break;

        case EVENT_BUTTON2:
        {
            if(updateState.running)
            {
                /* End update, and show message stating so. */
                Audio_stopCollect(Audio);
                vcdMsg = VCD_stopUpdate(VCD_handle);
                updateState.running = false;

                /* Set selected item to go to main menu, no live item */
                LCD_updateSelectedItem(&UpdateMenu,
                    UpdateMenu.itemSelected, 0,
                    false);

                UpdateMenu.itemSelected = 0;

                if(vcdMsg != VCD_MsgNone)
                {
                    LCD_showMessage("Stop Fail", 0);
                }
                else
                {
                    LCD_showMessage("Ended Update", 0);
                }
            }

            else if(UpdateMenu.itemSelected == 0)
            {
                /* Go back to main menu */
                LCD_showMenu(&MainMenu);
                return &MainMenu;
            }

            else
            {
                /* Start update of selected item if possible, and display
                 * resultant message.*/
                vcdMsg = VCD_startUpdate(VCD_handle,
                    UpdateMenu.items[UpdateMenu.itemSelected]);

                if( vcdMsg != VCD_MsgNone)
                {
                    LCD_showMessage("Start Fail", 0);
                }
                else
                {

#if defined (KENTEC_TOUCH)
                    /* Show live button */
                    LCD_updateSelectedItem(&UpdateMenu,
                            UpdateMenu.itemSelected,
                            UpdateMenu.itemSelected,
                            true);
#endif
                    Audio_startCollect(Audio);
                    updateState.running = true;
                    updateState.frameCount = 0;
                    LCD_showMessage(" Updating!", 0);
                }
            }
        }
        break;


        case EVENT_AUDIOFRAME:
        {
            if(updateState.running)
            {
                /* Do not use first 5 frames allowing for audio to stabilize
                 * and button push noise to dissipate. */
                if(updateState.frameCount < 5)
                {
                    updateState.frameCount++;
                    break;
                }

                /* Process next frame of update data. If update status
                 * indicates some ending, then display a result message and
                 * shut down update. */
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

                    case VCD_MsgUpdateNoMatch:
                    case VCD_MsgEnrollTooLong:
                    {
                        Audio_stopCollect(Audio);
                        VCD_stopUpdate(VCD_handle);
                        updateState.running = false;
#if defined (KENTEC_TOUCH)
                        /* Turn off live item */
                        LCD_updateSelectedItem(&UpdateMenu,
                                UpdateMenu.itemSelected,
                                UpdateMenu.itemSelected,
                                false);
#endif
                        if(vcdMsg == VCD_MsgUpdateNoMatch)
                        {
                            LCD_showMessage("Update Fail", 0);
                        }
                        else
                        {
                            LCD_showMessage("Update Long", 0);
                        }
                    }
                    break;

                    case VCD_MsgKeywordFound:
                    {
                        Audio_stopCollect(Audio);
                        updateState.running = false;

                        /* Get prior number of updates for model */
                        VCD_getModelInfo(VCD_handle,
                            UpdateMenu.items[UpdateMenu.itemSelected], 0,
                            &mInfo);

                        vcdMsg = VCD_addUpdate(VCD_handle, mInfo.numUpdates);
                        VCD_stopUpdate(VCD_handle);

                        if(vcdMsg != VCD_MsgNone)
                        {
                            LCD_showMessage("Add Fail", 0);
                        }
                        else
                        {
                            LCD_showMessage("Update Success", 0);
                        }

                        #if defined (KENTEC_TOUCH)
                        /* Set no live item, and reflect update */
                        LCD_updateSelectedItem(&UpdateMenu,
                                UpdateMenu.itemSelected,
                                UpdateMenu.itemSelected,
                                false);
                        #endif

                    }
                    break;
                    default:
                    break;
                }
            }

            else
            {
                /* Not running should not get an audio frame */
                Audio_stopCollect(Audio);
                updateState.running = false;
                LCD_showMessage("Update Error", 0);
            }
        }
        break;
        default:
        break;        
    }

    return &UpdateMenu;
}



