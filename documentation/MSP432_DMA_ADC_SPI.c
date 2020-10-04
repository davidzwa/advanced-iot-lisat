/* --COPYRIGHT--,BSD
 * Copyright (c) 2015, Texas Instruments Incorporated
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
//****************************************************************************
//
// MSP432_DMA_ADC_SPI.c - MSP-EXP432P401R
//
//  Port interrupt on P1.1
//
//  On port interrupt Send burst via SPI and measure via ADC
//  Use DMA to load SPI (basic mode Channel 0) and read from ADC (basic mode
//  channel 7).
//
//  UP_SAMPLING        the number of burst/measurements with each button
//                     press P1.1
//  TOF_DELAY          delay between SPI burst and ADC measurement
//  UP_SAMPLING_DELAY  additional delay for each successive burst/measurement
//                     ie. if UP_SAMPLING is 4 the delays of each measurement
//                     relative to the spi burst are as follows
//                     (1) TOF_DELAY+0
//                     (2) TOF_DELAY+UP_SAMPLING_DELAY
//                     (3) TOF_DELAY+2*UP_SAMPLING_DLEAY
//                     (4) TOF_DELAY+3*UP_SAMPLING_DELAY
//*****************************************************************************

#include "msp.h"
#include <driverlib.h>
#include <stdio.h>
#include <arm_math.h>
#include "arm_const_structs.h"

#define TOF_DELAY           48          // Based upon SMCLK clock 48Mhz

/*
 * Timer Delay between group of samples.  This number is added to the
 * delay (TOF_DELAY) of subsequent samples.
 */
#define UP_SAMPLING_DELAY   24
/*
 * Defines the number of samples to be taken
 */
#define UP_SAMPLING         4
#define SAMPLE_LENGTH       128
#define SAMPLE_FREQUENCY    800000

#define SMCLK_FREQUENCY     48000000
#define SPI_CLK_FREQUENCY	4000000
#define SPI_LENGTH			500

/*
 * SPI Configuration Parameter
 * 4Mhz SPI based off of 48MHz SMCLK
 */
const eUSCI_SPI_MasterConfig spiMasterConfig =
{
		EUSCI_B_SPI_CLOCKSOURCE_SMCLK,
		SMCLK_FREQUENCY,
		SPI_CLK_FREQUENCY,
        EUSCI_B_SPI_MSB_FIRST,
        EUSCI_B_SPI_PHASE_DATA_CAPTURED_ONFIRST_CHANGED_ON_NEXT,
        EUSCI_B_SPI_CLOCKPOLARITY_INACTIVITY_HIGH, EUSCI_B_SPI_3PIN
};
/*
 * TA0 PWM Configuration
 * Timer used to Trigger ADC
 * SMCLK -> SMCLK_FREQUENCY
 * Sample Frequency -> SAMPLE_FREQUENCY
 */
const Timer_A_PWMConfig pwmConfig =
{
        TIMER_A_CLOCKSOURCE_SMCLK,              // 48Mhz
        TIMER_A_CLOCKSOURCE_DIVIDER_1,          // 48Mhz
        (SMCLK_FREQUENCY/SAMPLE_FREQUENCY),     // 60
        TIMER_A_CAPTURECOMPARE_REGISTER_1,      // CCR1
        TIMER_A_OUTPUTMODE_SET_RESET,
        (SMCLK_FREQUENCY/SAMPLE_FREQUENCY)/2    // 30
};

/*
 * TA0 Upmode Configuration
 * Timer used to delay ADC sample relative to SPI burst
 * ACLK -> REFO
 * timerPeriod -> TOF
 */
Timer_A_UpModeConfig upConfig =
{
		.clockSource = TIMER_A_CLOCKSOURCE_SMCLK,
	    .clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_1,
	    .timerPeriod = TOF_DELAY,
	    .timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_DISABLE,
	    .captureCompareInterruptEnable_CCR0_CCIE = TIMER_A_CCIE_CCR0_INTERRUPT_ENABLE,
	    .timerClear = TIMER_A_SKIP_CLEAR
};

//40KHZ pulse train
uint8_t mstxData[SPI_LENGTH] =
{
		255,255,255,255,255,255,192,0,0,0,0,0,
		15,255,255,255,255,255,252,0,0,0,0,0,0,
		255,255,255,255,255,255,192,0,0,0,0,0,
		31,255,255,255,255,255,252,0,0,0,0,0,
		1,255,255,255,255,255,255,192,0,0,0,0,0,
		15,255,255,255,255,255,248,0,0,0,0,0,0,
		255,255,255,255,255,255,192,0,0,0,0,0,
		15,255,255,255,255,255,248,0,0,0,0,0,
		1,255,255,255,255,255,255,192,0,0,0,0,0,
		15,255,255,255,255,255,248,0,0,0,0,0,0,
		255,255,255,255,255,255,128,0,0,0,0,0,
		15,255,255,255,255,255,252,0,0,0,0,0,0,
		255,255,255,255,255,255,192,0,0,0,0,0,
		15,255,255,255,255,255,252,0,0,0,0,0,0,
		255,255,255,255,255,255,192,0,0,0,0,0,
		15,255,255,255,255,255,252,0,0,0,0,0,0,
		255,255,255,255,255,255,192,0,0,0,0,0,
		15,255,255,255,255,255,252,0,0,0,0,0,0,
		255,255,255,255,255,255,192,0,0,0,0,0,
		15,255,255,255,255,255,252,0,0,0,0,0,0,
		255,255,255,255,255,255,128,0,0,0,0,0,
		31,255,255,255,255,255,252,0,0,0,0,0,
		1,255,255,255,255,255,255,192,0,0,0,0,0,
		15,255,255,255,255,255,248,0,0,0,0,0,
		1,255,255,255,255,255,255,192,0,0,0,0,0,
		31,255,255,255,255,255,248,0,0,0,0,0,0,
		255,255,255,255,255,255,192,0,0,0,0,0,
		31,255,255,255,255,255,252,0,0,0,0,0,0,
		255,255,255,255,255,255,128,0,0,0,0,0,
		15,255,255,255,255,255,252,0,0,0,0,0,
		1,255,255,255,255,255,255,192,0,0,0,0,0,
		15,255,255,255,255,255,248,0,0,0,0,0,
		1,255,255,255,255,255,255,192,0,0,0,0,0,
		15,255,255,255,255,255,248,0,0,0,0,0,0,
		255,255,255,255,255,255,192,0,0,0,0,0,
		31,255,255,255,255,255,252,0,0,0,0,0,0,
		255,255,255,255,255,255,128,0,0,0,0,0,
		15,255,255,255,255,255,252,0,0,0,0,0,
		1,255,255,255,255,255,255,192,0,0,0,0,0,
		15,255,255,255,255,255,248,0,0,0,0,0,0,0
};

volatile arm_status status;

/* DMA Control Table */
#if defined(__TI_COMPILER_VERSION__)
#pragma DATA_ALIGN(MSP_EXP432P401RLP_DMAControlTable, 1024)
#elif defined(__IAR_SYSTEMS_ICC__)
#pragma data_alignment=1024
#elif defined(__GNUC__)
__attribute__ ((aligned (1024)))
#elif defined(__CC_ARM)
__align(1024)
#endif
static DMA_ControlTable MSP_EXP432P401RLP_DMAControlTable[32];

int16_t data_array[UP_SAMPLING][SAMPLE_LENGTH];
int16_t dataUpSample[UP_SAMPLING*SAMPLE_LENGTH];
int16_t data_input[SAMPLE_LENGTH];

uint8_t activeCount;

void main(void)
{
    /* Halting WDT and disabling master interrupts */
    MAP_WDT_A_holdTimer();
    MAP_Interrupt_disableMaster();

/*
 * Debug
 */
//    memset(data_array,0x0000,SAMPLE_LENGTH);
//    memset(mstxData,0xFE,500);

    /* Set the core voltage level to VCORE1 */
    MAP_PCM_setCoreVoltageLevel(PCM_VCORE1);

    /* Set 2 flash wait states for Flash bank 0 and 1*/
    MAP_FlashCtl_setWaitState(FLASH_BANK0, 2);
    MAP_FlashCtl_setWaitState(FLASH_BANK1, 2);

    /* Initializes Clock System */
    MAP_CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_48);
    MAP_CS_initClockSignal(CS_MCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1 );
    MAP_CS_initClockSignal(CS_HSMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1 );
    MAP_CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1 );
    MAP_CS_initClockSignal(CS_ACLK, CS_REFOCLK_SELECT, CS_CLOCK_DIVIDER_1);

    /*
     * Debug, output TA0.1
     */
    P2->DIR |= BIT4;
    P2->SEL0 |= BIT4;
    P2->SEL1 &= ~BIT4;

    /*
     * Initializing ADC (MCLK/1/1)
     *
     * 4+16 Clock cycles/conversion -> 417ns
     */
    ADC14_enableModule();
    ADC14_initModule(ADC_CLOCKSOURCE_MCLK, ADC_PREDIVIDER_1, ADC_DIVIDER_1, 0);

    ADC14_setSampleHoldTrigger(ADC_TRIGGER_SOURCE1, false);
    /*
     * Setup Sample Pulse mode on ADC14, 4xMCLK sample time, 16xMCLK conversion.
     * When not in use the default is the TA0.1 signal and the sample time is
     * dictated by the high duty cycle.
     */
//    ADC14->CTL0 |= ADC14_CTL0_SHP;
//    ADC14->CTL0 &= ~(ADC14_CTL0_SHT1_MASK+ADC14_CTL0_SHT0_MASK);

    /* Configuring GPIOs (4.3 A10) */
    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P4, GPIO_PIN3,
    GPIO_TERTIARY_MODULE_FUNCTION);
    __no_operation();

    /* Configuring ADC Memory */
    MAP_ADC14_configureSingleSampleMode(ADC_MEM0, true);
    MAP_ADC14_configureConversionMemory(ADC_MEM0, ADC_VREFPOS_AVCC_VREFNEG_VSS,
    ADC_INPUT_A10, false);

    /* Set ADC result format to signed binary */
    //ADC14_setResultFormat(ADC_SIGNED_BINARY);

    /*
     * Setup SPI
     */

    MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P1,
            GPIO_PIN6, GPIO_PRIMARY_MODULE_FUNCTION);

    MAP_SPI_initMaster(EUSCI_B0_BASE, (eUSCI_SPI_MasterConfig*)&spiMasterConfig);

    /* Configuring DMA module */
    DMA_enableModule();
    DMA_setControlBase(MSP_EXP432P401RLP_DMAControlTable);

    /*
     * Setup the DMA + USCI:SPI interface
     */
    MAP_DMA_setChannelControl(DMA_CH0_EUSCIB0TX0 | UDMA_PRI_SELECT,
    UDMA_SIZE_8 | UDMA_SRC_INC_8 | UDMA_DST_INC_NONE | UDMA_ARB_1);
    MAP_DMA_setChannelTransfer(DMA_CH0_EUSCIB0TX0 | UDMA_PRI_SELECT,
    UDMA_MODE_BASIC, mstxData,
            (void *) MAP_SPI_getTransmitBufferAddressForDMA(EUSCI_B0_BASE),
    		SPI_LENGTH);

    DMA_disableChannelAttribute(0,UDMA_ATTR_HIGH_PRIORITY);
    MAP_DMA_assignChannel(DMA_CH0_EUSCIB0TX0);

    /*
     * Setup the DMA + ADC14 interface
     */
    DMA_disableChannelAttribute(DMA_CH7_ADC14,
                                 UDMA_ATTR_ALTSELECT | UDMA_ATTR_USEBURST |
                                 UDMA_ATTR_HIGH_PRIORITY |
                                 UDMA_ATTR_REQMASK);

    /* Setting Control Indexes. In this case we will set the source of the
     * DMA transfer to ADC14 Memory 0
     *  and the destination to the
     * destination data array. */
    MAP_DMA_setChannelControl(UDMA_PRI_SELECT | DMA_CH7_ADC14,
        UDMA_SIZE_16 | UDMA_SRC_INC_NONE | UDMA_DST_INC_16 | UDMA_ARB_1);
    MAP_DMA_setChannelTransfer(UDMA_PRI_SELECT | DMA_CH7_ADC14,
    	UDMA_MODE_BASIC, (void*) &ADC14->MEM[0],
        &data_array[0][0], SAMPLE_LENGTH);

    DMA_enableChannelAttribute(7,UDMA_ATTR_HIGH_PRIORITY);


    /* Assigning/Enabling Interrupts */
    MAP_DMA_assignInterrupt(DMA_INT1, 7);
    MAP_Interrupt_enableInterrupt(INT_DMA_INT1);
    MAP_DMA_assignChannel(DMA_CH7_ADC14);
    MAP_DMA_clearInterruptFlag(7);

    /* Configuring P1.1 as an input and enabling interrupts */
    MAP_GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN1);
    MAP_GPIO_clearInterruptFlag(GPIO_PORT_P1, GPIO_PIN1);
    MAP_GPIO_enableInterrupt(GPIO_PORT_P1, GPIO_PIN1);
    MAP_Interrupt_enableInterrupt(INT_PORT1);

    MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P5, GPIO_PIN5);
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P5, GPIO_PIN5);

    MAP_Interrupt_enableMaster();
    activeCount = 0;

    while(1)
    {
        SCB->SCR |= SCB_SCR_SLEEPDEEP_Pos;
        MAP_Interrupt_enableSleepOnIsrExit();
        MAP_PCM_gotoLPM0();

        volatile int i,j;

        P5->OUT ^= BIT5;
		for(i=0; i<SAMPLE_LENGTH; i++)
		{
			for(j=0; j<UP_SAMPLING; j++)
			{
				dataUpSample[i*UP_SAMPLING+j] = data_array[j][i];
			}
		}
        P5->OUT ^= BIT5;
    }
}

/* Completion interrupt for ADC14 MEM0 */
#if     __TI_COMPILER_VERSION__ >= 15009000
__attribute__((ramfunc))
#endif
void DMA_INT1_IRQHandler(void)
{
    /*
     * Stop ADC, SPI, DMA
     */

//    MAP_GPIO_toggleOutputOnPin(GPIO_PORT_P5, GPIO_PIN5);
	P5->OUT ^= BIT5;

    MAP_DMA_disableChannel(0);
    MAP_DMA_disableChannel(7);

    MAP_SPI_disableModule(EUSCI_B0_BASE);
    MAP_ADC14_disableConversion();

    MAP_ADC14_clearInterruptFlag(ADC14_IFGR0_IFG0);
    MAP_SPI_clearInterruptFlag(EUSCI_B0_BASE,EUSCI_B_IFG_TXIFG0);
    MAP_DMA_clearInterruptFlag(7);

    MAP_Timer_A_stopTimer(TIMER_A0_BASE);
	activeCount++;
    if(activeCount < UP_SAMPLING)
    {
    	//pwmConfig.dutyCycle += UP_SAMPLING_DELAY;
        upConfig.timerPeriod+= UP_SAMPLING_DELAY;
    	/*********************************************************************/
		/*
		 * Start Timer to kickoff ADC measurements
		 */
    	/*********************************************************************/
		TIMER_A1->CTL &= ~(TIMER_A_CTL_IFG);
		TIMER_A1->CCTL[0] &= ~(TIMER_A_CCTLN_CCIFG);
		MAP_Timer_A_configureUpMode(TIMER_A1_BASE,(Timer_A_UpModeConfig*)&upConfig);
		MAP_Interrupt_enableInterrupt(INT_TA1_0);
		/*
		 * Channel 0 Primary
		 */
		MSP_EXP432P401RLP_DMAControlTable[0].srcEndAddr = &mstxData[SPI_LENGTH];
		MSP_EXP432P401RLP_DMAControlTable[0].control =
				(MSP_EXP432P401RLP_DMAControlTable[0].control & 0xff000000 ) |
				(((SPI_LENGTH)-1)<<4) | 0x01;
		MAP_DMA_enableChannel(0);
		/*
		 * Channel7 Primary
		 */
		MSP_EXP432P401RLP_DMAControlTable[7].dstEndAddr = &data_array[activeCount][SAMPLE_LENGTH];
		MSP_EXP432P401RLP_DMAControlTable[7].control =
				(MSP_EXP432P401RLP_DMAControlTable[7].control & 0xff000000 ) |
				(((SAMPLE_LENGTH)-1)<<4) | 0x01;
		MAP_DMA_enableChannel(7);
        //MAP_GPIO_toggleOutputOnPin(GPIO_PORT_P5, GPIO_PIN5);
		MAP_SPI_enableModule(EUSCI_B0_BASE);
		//MAP_Timer_A_startCounter(TIMER_A0_BASE,TIMER_A_UP_MODE);
		TIMER_A1->CTL |= TIMER_A_CTL_MC__UP;
    	/*********************************************************************/
    }
    else
    {
        MAP_Interrupt_disableSleepOnIsrExit();
        activeCount = 0;
        upConfig.timerPeriod = TOF_DELAY;
    }
	P5->OUT ^= BIT5;
}

/* GPIO ISR */
#if     __TI_COMPILER_VERSION__ >= 15009000
__attribute__((ramfunc))
#endif
void PORT1_IRQHandler(void)
{
    uint32_t status;

    status = MAP_GPIO_getEnabledInterruptStatus(GPIO_PORT_P1);
    MAP_GPIO_clearInterruptFlag(GPIO_PORT_P1, status);

    /* Toggling the output on the LED */
    if(status & GPIO_PIN1)
    {
        /*
         * Start ADC, SPI, DMA
         */
        //MAP_GPIO_toggleOutputOnPin(GPIO_PORT_P5, GPIO_PIN5);
    	P5->OUT ^= BIT5;
    	while(status & GPIO_PIN1)
    	{
    	    status = MAP_GPIO_getEnabledInterruptStatus(GPIO_PORT_P1);
    	    MAP_GPIO_clearInterruptFlag(GPIO_PORT_P1, status);
    	}

        /*
         * Start Timer to kickoff ADC measurements
         */
    	/*********************************************************************/
        TIMER_A1->CTL &= ~(TIMER_A_CTL_IFG);
        TIMER_A1->CCTL[0] &= ~(TIMER_A_CCTLN_CCIFG);
        MAP_Timer_A_configureUpMode(TIMER_A1_BASE,(Timer_A_UpModeConfig*)&upConfig);
        MAP_Interrupt_enableInterrupt(INT_TA1_0);
        /*
         * Channel 0 Primary
         */
		MSP_EXP432P401RLP_DMAControlTable[0].srcEndAddr = &mstxData[SPI_LENGTH];
    	MSP_EXP432P401RLP_DMAControlTable[0].control =
    			(MSP_EXP432P401RLP_DMAControlTable[0].control & 0xff000000 ) |
				(((SPI_LENGTH)-1)<<4) | 0x01;
        MAP_DMA_enableChannel(0);
    	/*
    	 * Channel7 Primary
    	 */
    	MSP_EXP432P401RLP_DMAControlTable[7].control =
    			(MSP_EXP432P401RLP_DMAControlTable[7].control & 0xff000000 ) |
    			(((SAMPLE_LENGTH)-1)<<4) | 0x01;
        MAP_DMA_enableChannel(7);
        //MAP_GPIO_toggleOutputOnPin(GPIO_PORT_P5, GPIO_PIN5);
    	P5->OUT ^= BIT5;
        MAP_SPI_enableModule(EUSCI_B0_BASE);
        //MAP_Timer_A_startCounter(TIMER_A0_BASE,TIMER_A_UP_MODE);
        TIMER_A1->CTL |= TIMER_A_CTL_MC__UP;
    	/*********************************************************************/
    }
}

/* TA1_0 ISR */
#if     __TI_COMPILER_VERSION__ >= 15009000
__attribute__((ramfunc))
#endif
void TA1CCR0_IRQHandler(void)
{
    //MAP_GPIO_toggleOutputOnPin(GPIO_PORT_P5, GPIO_PIN5);
	P5->OUT ^= BIT5;
    //MAP_Timer_A_stopTimer(TIMER_A0_BASE);
	TIMER_A1->CTL &= ~TIMER_A_CTL_MC_3;
    MAP_ADC14_enableConversion();
	MAP_Timer_A_generatePWM(TIMER_A0_BASE, &pwmConfig);
    //MAP_GPIO_toggleOutputOnPin(GPIO_PORT_P5, GPIO_PIN5);
	P5->OUT ^= BIT5;
	TIMER_A1->CCTL[0] &= ~(TIMER_A_CCTLN_CCIFG+TIMER_A_CCTLN_CCIE);
	MAP_Interrupt_disableInterrupt(INT_TA1_0);
}
