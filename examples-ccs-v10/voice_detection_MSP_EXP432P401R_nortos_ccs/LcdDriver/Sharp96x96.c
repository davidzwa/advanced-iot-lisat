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

//*****************************************************************************
//
// Sharp96x96.c
//
//*****************************************************************************
//
//! \addtogroup display_api
//! @{
//
//*****************************************************************************


#include <ti/grlib/grlib.h>
#include "Sharp96x96utils.h"
#include "Sharp96x96.h"
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

#define LCD_DISP_PIN_PORT      GPIO_PORT_P4
#define LCD_DISP_PIN           GPIO_PIN1
#define LCD_SPI_CS_PIN_PORT    GPIO_PORT_P4
#define LCD_SPI_CS_PIN         GPIO_PIN3
#define LCD_POWER_PIN_PORT     GPIO_PORT_P6
#define LCD_POWER_PIN          GPIO_PIN0

unsigned char DisplayBuffer[LCD_VERTICAL_MAX][LCD_HORIZONTAL_MAX/8];
unsigned char VCOMbit= 0x40;
unsigned char flagSendToggleVCOMCommand = 0;


//*****************************************************************************
//
//! Initializes the display driver.
//!
//! This function initializes the Sharp96x96 display. This function
//! configures the GPIO pins used to control the LCD display when the basic
//! GPIO interface is in use. On exit, the LCD has been reset and is ready to
//! receive command and data writes.
//!
//! \return None.
//
//*****************************************************************************
void
Sharp96x96_LCDInit(void)
{
    eUSCI_SPI_MasterConfig config =
            {
                    EUSCI_B_SPI_CLOCKSOURCE_SMCLK,
                                    12000000,
                                    6000000,
                                    EUSCI_B_SPI_MSB_FIRST,
                                    EUSCI_B_SPI_PHASE_DATA_CAPTURED_ONFIRST_CHANGED_ON_NEXT,
                                    EUSCI_B_SPI_CLOCKPOLARITY_INACTIVITY_LOW,
                                    EUSCI_B_SPI_3PIN
            };

    // ----Configure SPI Pins are UCB0 on P1.5, P1.6

    // Pin 1.5 = UCB0_SPI_CLK
    GPIO_setAsPeripheralModuleFunctionOutputPin( GPIO_PORT_P1, GPIO_PIN5, GPIO_PRIMARY_MODULE_FUNCTION);
    // Pin 1.6 = UCB0_SPI_SIMO
    GPIO_setAsPeripheralModuleFunctionOutputPin( GPIO_PORT_P1, GPIO_PIN6, GPIO_PRIMARY_MODULE_FUNCTION);


    // ----Configure output pin for DISP
    GPIO_setAsOutputPin(LCD_DISP_PIN_PORT, LCD_DISP_PIN);
    // Set DISP pin
    GPIO_setOutputHighOnPin(LCD_DISP_PIN_PORT, LCD_DISP_PIN);

    // ----Configure output pin for POWER
    GPIO_setAsOutputPin( LCD_POWER_PIN_PORT, LCD_POWER_PIN);
    // Set DISP pin
    GPIO_setOutputHighOnPin(LCD_POWER_PIN_PORT, LCD_POWER_PIN);

    // ----Configure CS Pin as P4.3 = SPI_CS
    GPIO_setAsOutputPin( LCD_SPI_CS_PIN_PORT, LCD_SPI_CS_PIN);
    // Set CS Pin
    GPIO_setOutputLowOnPin(LCD_SPI_CS_PIN_PORT, LCD_SPI_CS_PIN);

    SPI_initMaster(EUSCI_B0_BASE, &config);


    SPI_enableModule(EUSCI_B0_BASE);


}

//*****************************************************************************
//
//! Draws a pixel on the screen.
//!
//! \param pDisplay is a pointer to the driver-specific data for this
//! display driver.
//! \param lX is the X coordinate of the pixel.
//! \param lY is the Y coordinate of the pixel.
//! \param ulValue is the color of the pixel.
//!
//! This function sets the given pixel to a particular color.  The coordinates
//! of the pixel are assumed to be within the extents of the display.
//!
//! \return None.
//
//*****************************************************************************
static void
Sharp96x96_PixelDraw(const Graphics_Display *pDisplay, int16_t lX, int16_t lY,
                                   uint16_t ulValue)
{
    if( ClrBlack == ulValue)
        DisplayBuffer[lY][lX>>3] &= ~(0x80 >> (lX & 0x7));
    else
        DisplayBuffer[lY][lX>>3] |= (0x80 >> (lX & 0x7));

}
//*****************************************************************************
//
//! Draws a horizontal sequence of pixels on the screen.
//!
//! \param pDisplay is a pointer to the driver-specific data for this
//! display driver.
//! \param lX is the X coordinate of the first pixel.
//! \param lY is the Y coordinate of the first pixel.
//! \param lX0 is sub-pixel offset within the pixel data, which is valid for 1
//! or 4 bit per pixel formats.
//! \param lCount is the number of pixels to draw.
//! \param lBPP is the number of bits per pixel; must be 1, 4, or 8.
//! \param pucData is a pointer to the pixel data.  For 1 and 4 bit per pixel
//! formats, the most significant bit(s) represent the left-most pixel.
//! \param pucPalette is a pointer to the palette used to draw the pixels.
//!
//! This function draws a horizontal sequence of pixels on the screen, using
//! the supplied palette.  For 1 bit per pixel format, the palette contains
//! pre-translated colors; for 4 and 8 bit per pixel formats, the palette
//! contains 24-bit RGB values that must be translated before being written to
//! the display.
//!
//! \return None.
//
//*****************************************************************************
static void
Sharp96x96_DrawMultiple(const Graphics_Display *pDisplay, int16_t lX,
                                           int16_t lY, int16_t lX0, int16_t lCount,
                                           int16_t lBPP,
                                           const uint8_t *pucData,
                                           const uint32_t *pucPalette)
{  

    unsigned char *pData = &DisplayBuffer[lY][lX>>3];
    unsigned int xj = 0;

    //Write bytes of data to the display buffer
    for(xj=0;xj<lCount>>3;xj++)
        *pData++ = *pucData++;

    //Write last data byte to the display buffer
    *pData = (*pData & (0xFF >> (lCount & 0x7))) | *pucData;    

}
//*****************************************************************************
//
//! Draws a horizontal line.
//!
//! \param pDisplay is a pointer to the driver-specific data for this
//! display driver.
//! \param lX1 is the X coordinate of the start of the line.
//! \param lX2 is the X coordinate of the end of the line.
//! \param lY is the Y coordinate of the line.
//! \param ulValue is the color of the line.
//!
//! This function draws a horizontal line on the display.  The coordinates of
//! the line are assumed to be within the extents of the display.
//!
//! \return None.
//
//*****************************************************************************
static void
Sharp96x96_LineDrawH(const Graphics_Display *pDisplay, int16_t lX1, int16_t lX2,
                                   int16_t lY, uint16_t ulValue)
{
    volatile unsigned int xi = 0;
    volatile unsigned int x_index_min = lX1>>3;
    volatile unsigned int x_index_max = lX2>>3;
    volatile unsigned char *pucData, ucfirst_x_byte, uclast_x_byte;

    //calculate first byte
    ucfirst_x_byte = (0xFF >> (lX1 & 0x7));    //mod by 8 and shift this # bits
    //calculate last byte
    uclast_x_byte = (0xFF << (7-(lX2 & 0x7))); //mod by 8 and shift this # bits

    //check if more than one data byte
    if(x_index_min != x_index_max){

        //set buffer to correct location
        pucData = &DisplayBuffer[lY][x_index_min];

        //black pixels (clear bits)
        if(ClrBlack == ulValue)
        {
            //write first byte
            *pucData++ &= ~ucfirst_x_byte;

            //write middle bytes
            for(xi = x_index_min; xi < x_index_max-1; xi++)
            {
                *pucData++ = 0x00;
            }

            //write last byte
            *pucData &= ~uclast_x_byte;
        }
        //white pixels (set bits)
        else
        {
            //write first byte
            *pucData++ |= ucfirst_x_byte;

            //write middle bytes
            for(xi = x_index_min; xi < x_index_max-1; xi++)
            {
                *pucData++ = 0xFF;
            }

            //write last byte
            *pucData |= uclast_x_byte;
        }
    }
    //only one data byte
    else
    {
        //calculate value of single byte
        ucfirst_x_byte &= uclast_x_byte;

        //set buffer to correct location
        pucData = &DisplayBuffer[lY][x_index_min];

        //draw black pixels (clear bits)
        if(ClrBlack == ulValue)
        {
            *pucData++ &= ~ucfirst_x_byte;
        }
        //white pixels (set bits)
        else
        {
            *pucData++ |= ucfirst_x_byte;
        }
    }
}


//*****************************************************************************
//
//! Draws a vertical line.
//!
//! \param pDisplay is a pointer to the driver-specific data for this
//! display driver.
//! \param lX is the X coordinate of the line.
//! \param lY1 is the Y coordinate of the start of the line.
//! \param lY2 is the Y coordinate of the end of the line.
//! \param ulValue is the color of the line.
//!
//! This function draws a vertical line on the display.  The coordinates of the
//! line are assumed to be within the extents of the display.
//!
//! \return None.
//
//*****************************************************************************
static void
Sharp96x96_LineDrawV(const Graphics_Display *pDisplay, int16_t lX, int16_t lY1,
                                   int16_t lY2, uint16_t ulValue)
{

    volatile unsigned int yi = 0;
    volatile unsigned int x_index = lX>>3;
    volatile unsigned char data_byte;

    //calculate data byte
    data_byte = (0x80 >> (lX & 0x7));     //mod by 8 and shift this # bits

    //write data to the display buffer
    for(yi = lY1; yi <= lY2; yi++){

        //black pixels (clear bits)
        if(ClrBlack == ulValue)
        {
            DisplayBuffer[yi][x_index] &= ~data_byte;
        }
        //white pixels (set bits)
        else
        {
            DisplayBuffer[yi][x_index] |= data_byte;
        }
    }
    
}

//*****************************************************************************
//
//! Fills a rectangle.
//!
//! \param pDisplay is a pointer to the driver-specific data for this
//! display driver.
//! \param pRect is a pointer to the structure describing the rectangle.
//! \param ulValue is the color of the rectangle.
//!
//! This function fills a rectangle on the display.  The coordinates of the
//! rectangle are assumed to be within the extents of the display, and the
//! rectangle specification is fully inclusive (in other words, both xMin and
//! xMax are drawn, along with yMin and yMax).
//!
//! \return None.
//
//*****************************************************************************
static void
Sharp96x96_RectFill(const Graphics_Display *pDisplay, const Graphics_Rectangle *pRect,
                                  uint16_t ulValue)
{
    volatile unsigned int xi = 0;
    volatile unsigned int yi = 0;
    volatile unsigned int x_index_min = pRect->xMin>>3;
    volatile unsigned int x_index_max = pRect->xMax>>3;
    volatile unsigned char *pucData, ucfirst_x_byte, uclast_x_byte;

    //calculate first byte
    ucfirst_x_byte = (0xFF >> (pRect->xMin & 0x7));  //mod by 8 and shift this # bits

    //calculate last byte
    uclast_x_byte = (0xFF << (7-(pRect->xMax & 0x7)));   //mod by 8 and shift this # bits

    //check if more than one data byte
    if(x_index_min != x_index_max){

        //write bytes
        for (yi = pRect->yMin; yi<= pRect->yMax; yi++)
        {
            //set buffer to correct location
            pucData = &DisplayBuffer[yi][x_index_min];

            //black pixels (clear bits)
            if(ClrBlack == ulValue)
            {
                //write first byte
                *pucData++ &= ~ucfirst_x_byte;

                //write middle bytes
                for(xi = x_index_min; xi < x_index_max-1; xi++)
                {
                    *pucData++ = 0x00;
                }

                //write last byte
                *pucData &= ~uclast_x_byte;
            }
            //white pixels (set bits)
            else
            {
                //write first byte
                *pucData++ |= ucfirst_x_byte;

                //write middle bytes
                for(xi = x_index_min; xi < x_index_max-1; xi++)
                {
                    *pucData++ = 0xFF;
                }

                //write last byte
                *pucData |= uclast_x_byte;
            }
        }
    }
    //only one data byte
    else
    {
        //calculate value of single byte
        ucfirst_x_byte &= uclast_x_byte;

        //set buffer to correct location
        pucData = &DisplayBuffer[pRect->yMin][x_index_min];

        //black pixels (clear bits)
        if(ClrBlack == ulValue)
        {
            *pucData++ &= ~ucfirst_x_byte;
        }
        //white pixels (set bits)
        else
        {
            *pucData++ |= ucfirst_x_byte;
        }
    }    

}

//*****************************************************************************
//
//! Translates a 24-bit RGB color to a display driver-specific color.
//!
//! \param pDisplay is a pointer to the driver-specific data for this
//! display driver.
//! \param ulValue is the 24-bit RGB color.  The least-significant byte is the
//! blue channel, the next byte is the green channel, and the third byte is the
//! red channel.
//!
//! This function translates a 24-bit RGB color into a value that can be
//! written into the display's frame buffer in order to reproduce that color,
//! or the closest possible approximation of that color.
//!
//! \return Returns the display-driver specific color.
//
//*****************************************************************************
static uint32_t
Sharp96x96_ColorTranslate(const Graphics_Display *pDisplay,
                                        uint32_t ulValue)
{
    //
    // Translate from a 24-bit RGB color to a 5-6-5 RGB color.
    //
    return(DPYCOLORTRANSLATE(ulValue));
}



//*****************************************************************************
//
//! Initialize DisplayBuffer.
//!
//! \param pDisplay is a pointer to the driver-specific data for this
//! display driver.
//!
//!    \param ucValue is the foregroundcolor of the buffered data.
//!
//! This function initializes the display buffer and discards any cached data.
//!
//! \return None.
//
//*****************************************************************************
static void
Sharp96x96_InitializeDisplayBuffer(const Graphics_Display *pDisplay, unsigned char ucValue)
{
    unsigned int i=0,j=0;
    for(i =0; i< LCD_VERTICAL_MAX; i++)
    for(j =0; j< (LCD_HORIZONTAL_MAX>>3); j++)
        DisplayBuffer[i][j] = ucValue;    
}

//*****************************************************************************
//
//! Flushes any cached drawing operations.
//!
//! \param pDisplay is a pointer to the driver-specific data for this
//! display driver.
//!
//!
//! This functions flushes any cached drawing operations to the display.  This
//! is useful when a local frame buffer is used for drawing operations, and the
//! flush would copy the local frame buffer to the display.
//!
//! \return None.
//
//*****************************************************************************
static void
Sharp96x96_Flush (const Graphics_Display *pDisplay)
{
    volatile int iii;
unsigned char *pucData = &DisplayBuffer[0][0];
    long xi =0;
    long xj = 0;
    //image update mode(1X000000b)
    unsigned char command = SHARP_LCD_CMD_WRITE_LINE;

    //COM inversion bit
    command = command^VCOMbit;
    // Set P4.3 High for CS
    GPIO_setOutputHighOnPin(LCD_SPI_CS_PIN_PORT, LCD_SPI_CS_PIN);

    WriteCmdData(command);
    flagSendToggleVCOMCommand = SHARP_SKIP_TOGGLE_VCOM_COMMAND;
#ifdef LANDSCAPE
    for(xj=0; xj<LCD_VERTICAL_MAX; xj++)
        {
          WriteCmdData(reverse(xj + 1));

          for(xi=0; xi<(LCD_HORIZONTAL_MAX>>3); xi++)
          {
            WriteCmdData(*(pucData++));
          }
          WriteCmdData(SHARP_LCD_TRAILER_BYTE);
        }
#endif
#ifdef LANDSCAPE_FLIP
    pucData = &DisplayBuffer[LCD_VERTICAL_MAX-1][(LCD_HORIZONTAL_MAX>>3)-1];

    for(xj=1; xj<=LCD_VERTICAL_MAX; xj++)
    {
        WriteCmdData(reverse(xj));

      for(xi=0; xi < (LCD_HORIZONTAL_MAX>>3); xi++)
      {
        WriteCmdData(reverse(*pucData--));
      }
      WriteCmdData(SHARP_LCD_TRAILER_BYTE);
    }
#endif

    WriteCmdData(SHARP_LCD_TRAILER_BYTE);

    // Wait for last byte to be sent, then drop SCS
    WaitUntilLcdWriteFinished();

  // Wait for last byte to be sent, then drop SCS
  for ( iii =0;iii<(MCLK_FREQUENCY * 2);iii++);

    // Set P4.3 Low for CS
    GPIO_setOutputLowOnPin(LCD_SPI_CS_PIN_PORT, LCD_SPI_CS_PIN);
}

//*****************************************************************************
//
//! Send command to clear screen.
//!
//! \param pDisplay is a pointer to the driver-specific data for this
//! display driver.
//! \param ulValue is the foregroundcolor of the buffered data.
//!
//! This function simply does a clear screen. 
//!
//! \return None.
//
//*****************************************************************************
static void
Sharp96x96_ClearScreen (const Graphics_Display *pDisplay, uint16_t  ulValue)
{
  volatile int iii;
  unsigned char command = SHARP_LCD_CMD_CLEAR_SCREEN;                            //clear screen mode(0X100000b)
  command = command^VCOMbit;                    //COM inversion bit

  GPIO_setOutputHighOnPin(LCD_SPI_CS_PIN_PORT, LCD_SPI_CS_PIN);

  WriteCmdData(command);
  flagSendToggleVCOMCommand = SHARP_SKIP_TOGGLE_VCOM_COMMAND;
  WriteCmdData(SHARP_LCD_TRAILER_BYTE);

    // Wait for last byte to be sent, then drop SCS
    WaitUntilLcdWriteFinished();

  // Wait for last byte to be sent, then drop SCS
  for ( iii =0;iii<(MCLK_FREQUENCY * 2);iii++);


  GPIO_setOutputLowOnPin(LCD_SPI_CS_PIN_PORT, LCD_SPI_CS_PIN);
  
  Sharp96x96_InitializeDisplayBuffer(pDisplay, ulValue);

}

void
Sharp96x96_SendToggleVCOMCommand()
{
    volatile int iii;
    VCOMbit ^= SHARP_VCOM_TOGGLE_BIT;

    if(SHARP_SEND_TOGGLE_VCOM_COMMAND == flagSendToggleVCOMCommand)
    {
        unsigned char command = SHARP_LCD_CMD_CHANGE_VCOM;                            //clear screen mode(0X100000b)
        command = command^VCOMbit;                    //COM inversion bit

        GPIO_setOutputHighOnPin(LCD_SPI_CS_PIN_PORT, LCD_SPI_CS_PIN);

        WriteCmdData(command);
        WriteCmdData(SHARP_LCD_TRAILER_BYTE);

        // Wait for last byte to be sent, then drop SCS
        WaitUntilLcdWriteFinished();

      // Wait for last byte to be sent, then drop SCS
      for ( iii =0;iii<(MCLK_FREQUENCY * 2);iii++);

        GPIO_setOutputLowOnPin(LCD_SPI_CS_PIN_PORT, LCD_SPI_CS_PIN);
    }
    flagSendToggleVCOMCommand = SHARP_SEND_TOGGLE_VCOM_COMMAND;
}


//*****************************************************************************
//
//! The display structure that describes the driver for the 
//! sharpLCD panel 
//
//*****************************************************************************
Graphics_Display g_sharp96x96LCD =
{
    sizeof(Graphics_Display),
    0,
    LCD_HORIZONTAL_MAX,
    LCD_VERTICAL_MAX,
};

const Graphics_Display_Functions g_sharp96x96LCD_funcs =
{
    Sharp96x96_PixelDraw, //PixelDraw,
    Sharp96x96_DrawMultiple,
    Sharp96x96_LineDrawH,
    Sharp96x96_LineDrawV, //LineDrawV,
    Sharp96x96_RectFill, //RectFill,
    Sharp96x96_ColorTranslate,
    Sharp96x96_Flush, //Flush
    Sharp96x96_ClearScreen //Clear screen. Contents of display buffer unmodified
};


                                      
//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
