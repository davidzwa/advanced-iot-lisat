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

/*--------------------------------------------------------------------------
 Includes
 -------------------------------------------------------------------------*/
#include "vcd_user.h"

/*============================================================================
 Allocate global space for model memory
 ---------------------------------------------------------------------------*/

/* Example of allocating model storage in flash memory.
 * Place start of model memory on flash sector boundary. Flash
 * must be empty and reset, or must contain valid model data.
 */
#if defined(__ICCARM__)
#pragma data_alignment=TARGET_SECTOR_BYTES
#elif defined(__TI_ARM__)
#pragma DATA_ALIGN(VCD_modelsMemory, TARGET_SECTOR_BYTES)
#elif defined(__GNUC__)
__attribute__ ((aligned (TARGET_SECTOR_BYTES)))
#endif
int_least16_t const VCD_modelsMemory[VCD_MODELS_MEMORY_SIZE] = {0};


/*===========================================================================
 Allocate global space for VCD processing memory
 --------------------------------------------------------------------------*/

/* VCD persistent processing memory. This memory must persist, that is,
 * it can not be used for any other purpose while VCD is active (between
 * calls to VCD_init and VCD_shutdown.) */
void *VCD_memoryP[VCD_PERSIST_MEMORY];

/* VCD temporary scratch memory. This memory must be allocated as one
 * contiguous block.
 * This memory is only used while VCD is actively enrolling, updating,
 * or recognizing. Otherwise, this memory can be used for other purposes. */
int_least16_t VCD_memory16[VCD_M16_MEMORY];

/* Array of pointers that point to where each model is
 * stored in memory */
int_least16_t *VCD_modelsList[VCD_MODELS_NUMBER];

/* Allocation of the structure that holds the VCD configuration */
VCD_ConfigStruct VCDMemory = {
    VCD_MODELS_NUMBER,
    VCD_MODELS_OBS,
    VCD_MAXIMUM_MODELS_SIZE,
    VCD_modelsList,
    VCD_MODELS_NUMBER,
    VCD_memoryP,
    VCD_memory16 };


/*----------------------------------------------------------------------------
 * Functions
 * --------------------------------------------------------------------------*/
/* Iterates through flash sectors to initialize the VCD configuration
 * with an array of pointers containing addresses of any models that may be
 * stored in user memory and the number of models that were stored. */
void VCD_initModelsList(void)
{
    uint_fast16_t i;
    uint_fast16_t modelsNum = 0;

    // Start search at begining of allocated flash memory
    int_least16_t *address = (int_least16_t *)VCD_modelsMemory;
    VCD_MessageEnum msg;

    /* Iterate through the flash sectors up to the maximum number of models */
    for (i = 0; i < VCD_MODELS_NUMBER; i++)
    {
         msg = VCD_checkValidModel(address);

         /* If a sector contains a valid model, add it to the models list */
         if (msg == VCD_MsgNone)
         {
             VCD_modelsList[modelsNum++] = address;
         }

         /* Move to the next flash sector */
         address += VCD_MAXIMUM_MODELS_SIZE;
    }

    /* Set the number of models in the configuration structure */
    VCDMemory.numUserModels = modelsNum;
}

/*--------------------------------------------------------------------------*/
/* Required function that must be implemented by the user.
 * Writes a newly enrolled model's data to memory. */
VCD_MessageEnum VCD_writeModel(VCD_Handle vh,
                                      int_least16_t *ptrModel,
                                      int_least32_t mSize,
                                      int_least16_t **mAddress)
{
    uint32_t mask0, mask1, base;
    int_least16_t *secAddress, *tempAddress;
    int_least16_t modelSectors;
    int_fast16_t i, j;
    bool success = true;
    VCD_MessageEnum msg;

    /* Number of sectors assigned to each model */
    modelSectors = VCD_MAXIMUM_MODELS_SIZE / TARGET_SECTOR_SIZE;

    /* Determine sectors in lower bank containing the model */
    mask0 = 0x0;
    mask1 = 0x0;
    i = 0;

    /* Find next available sector in memory to store model */
    tempAddress = (int_least16_t *)VCD_modelsMemory;

    for(j = 0; j < VCD_MODELS_NUMBER; j++)
    {
         msg = VCD_checkValidModel(tempAddress);

         /* If a sector does not store a valid model, we can store a model there */
         if(msg != VCD_MsgNone)
         {
             break;
         }
         tempAddress += VCD_MAXIMUM_MODELS_SIZE;
    }
    secAddress = tempAddress;

    /* Determine sectors in lower bank containing the model */
    if(secAddress < TARGET_BANK1_ADDRESS)
    {
        base = (1 << ((uint32_t)secAddress >> TARGET_SECTOR_BITS));
        while(secAddress < TARGET_BANK1_ADDRESS && i < modelSectors)
        {
            mask0 |= base;
            i++;
            secAddress += TARGET_SECTOR_LEAST16_SIZE;
            base <<= 1;
        }
    }

    /* Determine sectors in the upper bank containing the model */
    if(i < modelSectors)
    {
        base = (1 << ((uint32_t)(secAddress - TARGET_BANK1_ADDRESS) >>
            TARGET_SECTOR_BITS));

        while(i < modelSectors)
        {
            mask1 |= base;
            i++;
            base <<= 1;
        }
    }

    /* Unprotect sectors of model, in each flash bank as necessary */
    if(mask0 != 0x0)
    {
        success = success &&
            MAP_FlashCtl_unprotectSector(FLASH_MAIN_MEMORY_SPACE_BANK0,
                mask0);
    }

    if(mask1 != 0x0)
    {
        success = success &&
            MAP_FlashCtl_unprotectSector(FLASH_MAIN_MEMORY_SPACE_BANK1,
                mask1);
    }

    /* Erase the flash */
    if(success)
    {
        success = success && MAP_FlashCtl_performMassErase();
    }

    /* Attempt to write model to memory */
    if(success)
    {
        secAddress = tempAddress;
        success = MAP_FlashCtl_programMemory((void *)ptrModel, (void *)secAddress, mSize);
        *mAddress = secAddress;
    }

    /* If failure at end of write, erase model data again */
    if(!success)
    {
        MAP_FlashCtl_performMassErase();
    }

    /* Protect the sectors after attempting to write */
    if(mask0 != 0x0)
    {
        MAP_FlashCtl_protectSector(FLASH_MAIN_MEMORY_SPACE_BANK0, mask0);
    }

    if(mask1 != 0x0)
    {
        MAP_FlashCtl_protectSector(FLASH_MAIN_MEMORY_SPACE_BANK1, mask1);
    }

    if(!success)
    {
        return VCD_MsgFail;
    }

return VCD_MsgNone;

}

/*--------------------------------------------------------------------------*/
/* Required function that must be implemented by the user.
 * Writes the updated model data to the memory address
 * containing the original model data. */
VCD_MessageEnum VCD_updateModel(VCD_Handle vh, int_least16_t *ptrModel,
                                      int_least32_t mSize, int_least16_t *mAddress)
{
    uint32_t mask0, mask1, base;
    int_least16_t *secAddress;
    int_least16_t modelSectors;
    int_fast16_t i;
    bool success = true;

    /* Number of sectors assigned to each model */
    modelSectors = VCD_MAXIMUM_MODELS_SIZE / TARGET_SECTOR_SIZE;

    /* Determine sectors in lower bank containing the model */
    mask0 = 0x0;
    mask1 = 0x0;
    i = 0;

    secAddress = mAddress;

    if(secAddress < TARGET_BANK1_ADDRESS)
    {
        base = (1 << ((uint32_t)secAddress >> TARGET_SECTOR_BITS));
        while(secAddress < TARGET_BANK1_ADDRESS && i < modelSectors)
        {
            mask0 |= base;
            i++;
            secAddress += TARGET_SECTOR_LEAST16_SIZE;
            base <<= 1;
        }
    }

    /* Determine sectors in the upper bank containing the model */
    if(i < modelSectors)
    {
        base = (1 << ((uint32_t)(secAddress - TARGET_BANK1_ADDRESS) >>
            TARGET_SECTOR_BITS));

        while(i < modelSectors)
        {
            mask1 |= base;
            i++;
            base <<= 1;
        }
    }

    /* Unprotect sectors of model, in each flash bank as necessary */
    if(mask0 != 0x0)
    {
        success = success &&
            MAP_FlashCtl_unprotectSector(FLASH_MAIN_MEMORY_SPACE_BANK0,
                mask0);
    }

    if(mask1 != 0x0)
    {
        success = success &&
            MAP_FlashCtl_unprotectSector(FLASH_MAIN_MEMORY_SPACE_BANK1,
                mask1);
    }

    /* Erase the flash */
    if(success)
    {
        success = success && MAP_FlashCtl_performMassErase();
    }

    /* Attempt to write model to memory */
    if(success)
    {
        success = MAP_FlashCtl_programMemory((void *)ptrModel, (void *)mAddress, mSize);
    }

    /* If failure at end of write, erase model data again */
    if(!success)
    {
        MAP_FlashCtl_performMassErase();
    }

    /* Protect the sectors after attempting to write */
    if(mask0 != 0x0)
    {
        MAP_FlashCtl_protectSector(FLASH_MAIN_MEMORY_SPACE_BANK0, mask0);
    }

    if(mask1 != 0x0)
    {
        MAP_FlashCtl_protectSector(FLASH_MAIN_MEMORY_SPACE_BANK1, mask1);
    }

    if(!success)
    {
        return VCD_MsgFail;
    }

    return VCD_MsgNone;
}

/*--------------------------------------------------------------------------*/
/* Required function that must be implemented by the user.
 * Clears a single model from memory */
VCD_MessageEnum VCD_clearModel(int_least16_t *mAddress)
{
    bool success = true;
    int_least32_t i;
    uint32_t mask0, mask1, base;
    int_least16_t *secAddress;
    int_least16_t modelSectors;

    /* Number of sectors assigned to each model */
        modelSectors = VCD_MAXIMUM_MODELS_SIZE / TARGET_SECTOR_SIZE;

    /* Determine sectors in lower bank containing the model */
    mask0 = 0x0;
    mask1 = 0x0;
    i = 0;

    secAddress = mAddress;

    if(secAddress < TARGET_BANK1_ADDRESS)
    {
        base = (1 << ((uint32_t)secAddress >> TARGET_SECTOR_BITS));
        while(secAddress < TARGET_BANK1_ADDRESS && i < modelSectors)
        {
            mask0 |= base;
            i++;
            secAddress += TARGET_SECTOR_SIZE;
            base <<= 1;
        }
    }

    /* Determine sectors in the upper bank contraining the model */
    if(i < modelSectors)
    {
        base = (1 << ((uint32_t)(secAddress - TARGET_BANK1_ADDRESS) >>
            TARGET_SECTOR_BITS));

        while(i < modelSectors)
        {
            mask1 |= base;
            i++;
            base <<= 1;
        }
    }

    /* Unprotect sectors of model, in each flash bank as necessary */
    if(mask0 != 0x0)
    {
        success = success &&
            MAP_FlashCtl_unprotectSector(FLASH_MAIN_MEMORY_SPACE_BANK0,
                mask0);
    }

    if(mask1 != 0x0)
    {
        success = success &&
            MAP_FlashCtl_unprotectSector(FLASH_MAIN_MEMORY_SPACE_BANK1,
                mask1);
    }

    /* Erase the flash */
    if(success)
    {
        success = success && MAP_FlashCtl_performMassErase();
    }

    /* Protect the sectors after erasing */
    if(mask0 != 0x0)
    {
        MAP_FlashCtl_protectSector(FLASH_MAIN_MEMORY_SPACE_BANK0, mask0);
    }

    if(mask1 != 0x0)
    {
        MAP_FlashCtl_protectSector(FLASH_MAIN_MEMORY_SPACE_BANK1, mask1);
    }

    if(!success)
    {
        return VCD_MsgFail;
    }

    return VCD_MsgNone;
}
