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

/** \file updateMenu.c Defines the delete menu and operations it invokes.\n (C)
 *  Copyright 2015, Texas Instruments, Inc.
 */

/*----------------------------------------------------------------------------
 * Includes
 * --------------------------------------------------------------------------*/
#include <stdlib.h>
#include <stdbool.h>

#include <ti/devices/msp432p4xx/inc/msp.h>
#include <ti/voice_detection/vcd.h>

/* APIs used in delete menu */
#include "menus.h"

#include "audio_collect.h"
#include "lcd.h"

#if defined(KENTEC_TOUCH)
#include "touch.h"
#endif


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
extern Menu_Object UpdateMenu;
extern Menu_Object EnrollMenu;

/* Delete menu defined */
static char const * const deleteItems[] =
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

/* Delete menu item state 0=can be deleted, 1=no model available */
static int_least8_t itemState[9] = {0, 1, 1, 1, 1, 1, 1, 1, 1};

/* Delete event handling function declared */
Menu_Handle DeleteMenu_processEvent(Events event);

/* Delete menu instance */
Menu_Object DeleteMenu =
{
    "Delete Model",
    deleteItems,
    DeleteMenu_processEvent,
    // Number of items
    9,
    // Initial item
    0,

    // delete item state
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
Menu_Handle DeleteMenu_processEvent(Events event)
{
    VCD_MessageEnum vcdMsg;

    switch(event)
    {
#if defined(KENTEC_TOUCH)
        case EVENT_TOUCH:
        {
            Touch_Info touchLoc;
            int_least8_t item = DeleteMenu.numItems;

            /* Get normalized touch location */
            Touch_getLocation(Touch, &touchLoc, true);

            /* Determine which soft item button was pushed if any */
            if(touchLoc.validLocation)
            {
                for( item = 0; item < DeleteMenu.numItems; item ++)
                {
                    if( LCD_itemTouched(&DeleteMenu, item, &touchLoc) )
                    {
                        break;
                    }
                }
            }

            /* Reenable touch detections */
            Touch_enableDetect(Touch, true);

            /* Take action based on soft button that was pushed */
            if( item < DeleteMenu.numItems)
            {
                DeleteMenu.itemSelected = item;

                if(DeleteMenu.itemSelected == 0)
                {
                    /* Go back to main menu */
                    LCD_showMenu(&MainMenu);
                    return &MainMenu;
                }
                else
                {
                    /* Try to delete selected item if possible, and display
                     * resultant message.*/

                    /* Set selected item  */
                    LCD_updateSelectedItem(&DeleteMenu,
                        item, DeleteMenu.itemSelected,
                        true);

                    vcdMsg = VCD_removeModel(VCD_handle,
                        DeleteMenu.items[DeleteMenu.itemSelected], -1);

                    /* Display output of attempt to delete model */
                    if(vcdMsg != VCD_MsgNone)
                    {
                        LCD_showMessage("Delete Fail", 0);
                    }
                    else
                    {
                        LCD_showMessage("Deleted Model", 0);
                        DeleteMenu.itemState[DeleteMenu.itemSelected] = 1;
                        EnrollMenu.itemState[DeleteMenu.itemSelected] = 0;
                        UpdateMenu.itemState[DeleteMenu.itemSelected] = 1;

                    }

                    /* Set selected item to return to main menu */
                    DeleteMenu.itemSelected = 0;
                    LCD_updateSelectedItem(&DeleteMenu,
                        item, DeleteMenu.itemSelected,
                        false);
                }
            }
        }
        break;

#endif

    case EVENT_BUTTON1:
        {
            /* Go to next menu item */
            Menus_nextItem(&DeleteMenu);
        }
        break;

        case EVENT_BUTTON2:
        {
            if(DeleteMenu.itemSelected == 0)
            {
                /* Go back to main menu */
                LCD_showMenu(&MainMenu);
                return &MainMenu;
            }
            else
            {
                /* Try to delete selected item if possible, and display
                 * resultant message.*/
                vcdMsg = VCD_removeModel(VCD_handle,
                    DeleteMenu.items[DeleteMenu.itemSelected], -1);

                /* Display output of attempt to delete model */
                if(vcdMsg != VCD_MsgNone)
                {
                    LCD_showMessage("Delete Fail", 0);
                }
                else
                {
                    LCD_showMessage("Deleted Model", 0);
                    DeleteMenu.itemState[DeleteMenu.itemSelected] = 1;
                    EnrollMenu.itemState[DeleteMenu.itemSelected] = 0;
                    UpdateMenu.itemState[DeleteMenu.itemSelected] = 1;
                }


                /* Set selected item to return to main menu */
                LCD_updateSelectedItem(&DeleteMenu,
                    DeleteMenu.itemSelected, 0,
                    false);
                DeleteMenu.itemSelected = 0;


            }
        }
        break;


        case EVENT_AUDIOFRAME:
        {
            /* Should not get an audio frame while deleting */
            Audio_stopCollect(Audio);
            LCD_showMessage("Delete Error", 0);
        }
        break;
        default:
        break;
    }

    return &DeleteMenu;
}
