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

/** \file main.c A test program for exercising the enrollment, update and
recognition APIs along with some utility APIs of the MinHMM
speech recognizer\n (C) Copyright 2015, Texas Instruments, Inc.
 */

/*----------------------------------------------------------------------------
 * Includes
 * --------------------------------------------------------------------------*/
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <ti/devices/msp432p4xx/inc/msp.h>
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

/* APIs used in the application */
#include "demo/menus.h"
#include "demo/audio_collect.h"
#include "demo/lcd.h"
#include "demo/buttons.h"
#include "demo/leds.h"
#include "demo/vcd_user.h"
#include <ti/voice_detection/vcd.h>

#if defined(KENTEC_TOUCH)
#include "demo/touch.h"
#endif


/*----------------------------------------------------------------------------
 * Globals
 * -------------------------------------------------------------------------*/

extern uint32_t DmaControlTable[];

/* Global handles for APIs used in the application that retain
 * state information in the application */
Audio_Handle Audio;
Buttons_Handle Buttons;
VCD_Handle VCD_handle;

#if defined(KENTEC_TOUCH)
Touch_Handle Touch;
#endif

/* Memory allocation for VCD models and processing */
extern VCD_ConfigStruct VCDMemory;

/* Ping-pong audio buffers for Audio API setup */
int_least16_t AudioBuffer1[160];
int_least16_t AudioBuffer2[160];

/* demo menus */
extern Menu_Object MainMenu;
extern Menu_Object EnrollMenu;
extern Menu_Object UpdateMenu;
extern Menu_Object DeleteMenu;
extern Menu_Object RecognizeMenu;


/*----------------------------------------------------------------------------
 * Functions
 * --------------------------------------------------------------------------*/

/* Set up general system configuration */
static void systemSetup(void)
{
    /* Halting WDT */
    MAP_WDT_A_holdTimer();

    /* Set power mode with required flash wait states */
    MAP_PCM_setPowerState(PCM_AM_LDO_VCORE1);
    MAP_FlashCtl_setWaitState(FLASH_BANK0, 2);
    MAP_FlashCtl_setWaitState(FLASH_BANK1, 2);

    /* Set system clocks */
    MAP_CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_48);
    MAP_CS_initClockSignal(CS_MCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
    MAP_CS_initClockSignal(CS_HSMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_2);
    MAP_CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_4);

    /* Use REFO clock for driving color LED and LCD VCOM signal toggle  */
    MAP_CS_initClockSignal(CS_ACLK, CS_REFOCLK_SELECT, CS_CLOCK_DIVIDER_1);

}

/*-------------------------------------------------------------------------*/
void main(void)
{
    /* Start at the main level menu */
    Menu_Handle currentMenu = &MainMenu;

    /* Flag to stop processing */
    bool done;

    VCD_ModelInfoStruct modelInfo;
    VCD_MessageEnum msg;
    int_least16_t item;

    /* Set up MSP432 */
    systemSetup();

    /* Hold off all interrupts */
    MAP_Interrupt_disableMaster();

    /* Initialize to continue processing after ISR */
    Interrupt_disableSleepOnIsrExit();

    /* Prepare to use Launchpad LEDs */
    Leds_setup();

    /* Prepare to use Launchpad buttons */
    Buttons = Buttons_setup();

    /* Initialize models stored in non-volatile memory */
    VCD_initModelsList();

    /* Initialize VCD recognizer */
    msg = VCD_init(&VCD_handle, &VCDMemory);
    if(msg != VCD_MsgNone)
    {
        exit(1);
    }

    /* Set menu item states to indicate a model that exists */
    for( item = 1; item < EnrollMenu.numItems; item++)
    {
        msg = VCD_getModelInfo(VCD_handle, EnrollMenu.items[item], -1, &modelInfo);
        if( msg == VCD_MsgNone )
        {
            EnrollMenu.itemState[item] = 1;
            UpdateMenu.itemState[item] = 0;
            DeleteMenu.itemState[item] = 0;
        }
    }

    #ifdef KENTEC_TOUCH
    /* Prepare to start looking for touches of the LCD screen */
    Touch = Touch_setup();
    Touch_enableDetect(Touch, true);
    #endif

    /* Setup audio object, but all collection is disabled */
    Audio = Audio_setupCollect(DmaControlTable,
            AudioBuffer1, AudioBuffer2, 160, 8000);

    /* Once setup is done, interrupts can be allowed */
    MAP_Interrupt_enableMaster();

    /* Prepare to use display */
    LCD_setup();

    /* Display the main menu */
    LCD_showMenu(currentMenu);

    /* Ensure LEDs are off */
    Leds_off1();
    Leds_setColor2(0, 0, 0);


    /* Event processing loop */
    done = false;
    while(!done)
    {
        /* The following steps are a means to wait for specific ISRs to occur
         * before continuing to process in the event loop, while allowing
         * all ISRs to complete their respective tasks within their ISRs. */

        /* Step 1 - Disable all interrupt ISR handling by setting PRIMASK.
         * After this instruction completes no ISRs will be running, but
         * they can become pending. */
        MAP_Interrupt_disableMaster();

        /* Step 2 - See if any event processing is pending, such as an audio
         * frame available or button pushed. If so, no need to wait for ISR to
         *  pend an event for processing. Otherwise block waiting for an
         *  event to process. */
        if(!Audio_getActive(Audio) && !Buttons_getButton1(Buttons) &&
            !Buttons_getButton2(Buttons)
            #ifdef KENTEC_TOUCH
            && !Touch_getDetected(Touch)
            #endif
            )
        {
            /* Step 3 - Enable sleep on ISR exit. All ISRs will sleep on exit,
             * unless the ISR specifically disables sleep on ISR exit. */
            Interrupt_enableSleepOnIsrExit();

            /* Step 4 - Go to LPM0 waiting for an interrupt via WFI instruction.
             * Since all ISR handling is disabled via PRIMASK, when an interrupt
             * exception occurs it will cause wakeup from WFI LPM0, and start
             * processing. However, the interrupt ISR execution will still be
             * pending due to the PRIMASK. */
            PCM_gotoLPM0();
        }

        /* Step 5 - Enable interrupt ISR handling for interrupts
         * by resetting the PRIMASK. If sleep on ISR exit is enabled,
         * all interrupts will have their ISRs executed, but not execute any
         * further code in the event loop unless the ISR itself disables
         * sleep on exit. An ISR should disable sleep on exit only if it
         * pends an event. */
        MAP_Interrupt_enableMaster();

        /* Check if buttons events have been set and handle them first. */
        if(Buttons_getButton1(Buttons))
        {
            currentMenu = currentMenu->processEvent(EVENT_BUTTON1);
            Buttons_clearButton1(Buttons);
        }

        if(Buttons_getButton2(Buttons))
        {
            currentMenu = currentMenu->processEvent(EVENT_BUTTON2);
            Buttons_clearButton2(Buttons);
        }

        #ifdef KENTEC_TOUCH
        /* Check if a touch has been detected */
        if( Touch_getDetected(Touch))
        {
            currentMenu = currentMenu->processEvent(EVENT_TOUCH);
        }
        #endif

        /* Check if audio collection has a buffer pending with valid data. */
        if(Audio_getActive(Audio))
        {
            currentMenu = currentMenu->processEvent(EVENT_AUDIOFRAME);
            Audio_resetActive(Audio);
        }


        /* Main menu flagging to quit the loop */
        if(currentMenu == NULL)
        {
            done = true;
        }
    }

    /* Shutdown everything */
    Audio_shutdownCollect(Audio);
    Buttons_shutdown(Buttons);
    Leds_shutdown();
    LCD_shutdown();
    #if defined(KENTEC_TOUCH)
    Touch_shutdown(Touch);
    #endif
    VCD_shutdown(VCD_handle);
}
