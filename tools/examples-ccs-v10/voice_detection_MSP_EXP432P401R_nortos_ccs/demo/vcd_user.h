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

/* This header file ontains the parameters whereby a user configures and
 calculates memory required specific to the users application for
 VCD for operation. */

#ifndef VCD_USER_H_
#define VCD_USER_H_

/*---------------------------------------------------------------------------
 Includes
 --------------------------------------------------------------------------*/
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include <ti/voice_detection/vcd.h>

/*===========================================================================
 Application-Specific Defines: Edit these based on application requirements

 Specify the application program-specific information about the VCD models
 here. This information is used to calculate and allocate the global space to
 store the models, usually in some form of non-volatile memory.

 The information is also used to calculate and allocate a global variables
 used by VCD to use during signal processing of the models. The reason for
 this is that RAM needed for processing is dependent on the number and size
 of models. These global variables are input to VCD in the VCD_init
 function call.
 --------------------------------------------------------------------------*/

/* Specify the maxmum number of phrase models to accommodate */
#define VCD_MODELS_NUMBER (8)

/* Specify the maximum speech time in milliseconds for a single phrase */
#define VCD_MODELS_MSEC (2500)

#if defined(__MSP432P401R__)
/* Number of bytes in a flash sector */
#define TARGET_SECTOR_BYTES (4096)
#endif

/*==========================================================================
 Derived Defines: These calculate VCD memory allocation requirements
 --------------------------------------------------------------------------*/

/* Parameters based on target hardware */

/** Bytes per int_least16_t */
#define TARGET_LEAST16_BYTES (sizeof(int_least16_t))

/** Bytes per char */
#define TARGET_CHAR_BYTES (sizeof(char))

/*--------------------------------------------------------------------------
  Determine the maximum size of a model in int_least16_t
  ----------------------------------------------------------------------- */

/* Maximum number of observations in a model */
#define VCD_MODELS_OBS \
 ((VCD_SAMPLE_RATE * VCD_MODELS_MSEC) / VCD_FRAME_SAMPLES / 1000 )

/* Maximum size of data in a model in int_least16_t  */
#define VCD_MODELS_DATA_SIZE \
    (VCD_MODELS_OBS * VCD_MODELS_OBS_DATA)

/* Number of int_least16_t in the maximum length model name */
#define VCD_MODELS_NAME_SIZE \
    (((VCD_MODELS_NAME_CHARS * TARGET_CHAR_BYTES) - 1) / TARGET_LEAST16_BYTES + 1)

/* Largest model size in int_least16_t */
#define VCD_MODELS_SIZE \
    (VCD_MODELS_OVERHEAD + VCD_MODELS_DATA_SIZE + VCD_MODELS_NAME_SIZE)

/*===========================================================================
 Calculate the memory requirements for models based on platform
 and location of model data.
 --------------------------------------------------------------------------*/

/** Target number of bits in sector size. */
#define TARGET_SECTOR_BITS (12)

/** Target bytes per int_least16_t */
#define TARGET_LEAST16_SIZE (sizeof(int_least16_t))

/** Target number of int_least16_t in sector */
#define TARGET_SECTOR_LEAST16_SIZE (TARGET_SECTOR_SIZE / TARGET_LEAST16_SIZE)

/** MSP432 flash bank 0 */
#define TARGET_BANK0_ADDRESS ((int_least16_t *)(0x0))

/** MSP432 flash bank 1 */
#define TARGET_BANK1_ADDRESS ((int_least16_t *)(0x20000))

/* Size of a flash sector in_least16_t */
#define TARGET_SECTOR_SIZE (TARGET_SECTOR_BYTES / TARGET_LEAST16_BYTES)

/* Sectors needed for largest model */
#define VCD_MODELS_SECTORS \
    ( ((VCD_MODELS_SIZE - 1) / TARGET_SECTOR_SIZE) + 1)

/* Size of each model in int_least16_t which is a multiple of sectors */
#define VCD_MAXIMUM_MODELS_SIZE (VCD_MODELS_SECTORS * TARGET_SECTOR_SIZE)

/* Size of flash memory in int_least16_t required for models */
#define VCD_MODELS_MEMORY_SIZE \
    (VCD_MAXIMUM_MODELS_SIZE * VCD_MODELS_NUMBER)


/*===========================================================================
 Calculate the memory requirements for RAM processing memory.
 --------------------------------------------------------------------------*/

#define VCD_PERSIST_MEMORY (VCD_P_MULT * (VCD_MODELS_NUMBER + 1) )

#define VCD_M16_PROC  (VCD_MODELS_OBS * VCD_M16_MULT + VCD_M16_ADD)
#define VCD_M16_OBS   (VCD_MODELS_OBS * VCD_MODELS_NUMBER)

#if (VCD_M16_OBS < VCD_M16_PROC)
#define VCD_M16_MEMORY (VCD_M16_PROC)
#else
#define VCD_M16_MEMORY (VCD_M16_OBS)
#endif

/*----------------------------------------------------------------------------
 * Functions
 * --------------------------------------------------------------------------*/
/* Iterates through flash sectors to initialize the modelsList (array of
 * pointers containing addresses of any models that may be stored in user
 * memory) and numUserModels (the number of models that were stored)
 * parameters of the VCD configuration structure. The VCD config
 * struct must be filled before the call to VCD_init. */
void VCD_initModelsList(void);


#endif /* VCD_USER_H_ */
