/*
 * adc14_multiple_channel_no_repeat_timera_source_01.c
 *
 * Timer (300kHz) Triggered sequence of conversions (ADC14 6,7,8),
 * where each conversion in the sequence is performed immediately
 * after the previous.  After the last conversion in the sequence
 * the ADC14 waits for the timer trigger.
 *
 * The ISR moves information from ADCMEM6-ADCMEME8 to three
 * seperate buffers buffer0[], buffer1[], buffer2[]
 *
 */

#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

/* Standard Includes */
#include <stdint.h>
#include <string.h>

#define NUMBER_OF_SAMPLES   1024

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
static DMA_ControlTable MSP_EXP432P401RLP_DMAControlTable[16];

uint16_t prim_buffer0[NUMBER_OF_SAMPLES];
uint16_t prim_buffer1[NUMBER_OF_SAMPLES];
uint16_t prim_buffer2[NUMBER_OF_SAMPLES];

uint16_t alt_buffer0[NUMBER_OF_SAMPLES];
uint16_t alt_buffer1[NUMBER_OF_SAMPLES];
uint16_t alt_buffer2[NUMBER_OF_SAMPLES];


volatile int switch_data = 0;

/*
 * Timer_A Continuous Mode Configuration Parameter
 *
 * 3 Conversions
 * each takes approximately 23 ADC Clocks
 * 3*23/25Mhz = 2.76us (360ksps)
 *
 * 4 us to handle IRQ and request data transfer
 *
 * 6.76us -> 7us or 142khz
 *
 * 1/((199+1)/24M) = 120khz
 */
const Timer_A_PWMConfig timerA_PWM =
{
    .clockSource = TIMER_A_CLOCKSOURCE_SMCLK,
    .clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_1,
    .timerPeriod = 199,
    .compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_1,
    .compareOutputMode TIMER_A_OUTPUTMODE_SET_RESET,
    .dutyCycle = 100
};

uint16_t measureIndex;

int main(void)
{
    uint16_t ii;
    uint16_t *tempPtr0,*tempPtr1,*tempPtr2;

    uint32_t average0,average1,average2;

    /* Halting WDT  */
    MAP_WDT_A_holdTimer();
    MAP_Interrupt_disableMaster();
    MAP_Interrupt_enableSleepOnIsrExit();

    /* Zero-filling buffer */
    memset(prim_buffer0,0x0000,NUMBER_OF_SAMPLES);
    memset(prim_buffer1,0x0000,NUMBER_OF_SAMPLES);
    memset(prim_buffer2,0x0000,NUMBER_OF_SAMPLES);

    memset(alt_buffer0,0x0000,NUMBER_OF_SAMPLES);
    memset(alt_buffer1,0x0000,NUMBER_OF_SAMPLES);
    memset(alt_buffer2,0x0000,NUMBER_OF_SAMPLES);

    measureIndex = 0;

    /*
     * Use default settings for 48Mhz DCO
     */
    MAP_PCM_setCoreVoltageLevel(PCM_VCORE1);
    MAP_FlashCtl_setWaitState(FLASH_BANK0, 1);
    MAP_FlashCtl_setWaitState(FLASH_BANK1, 1);

    /*
     * Setting up clocks
     * MCLK = MCLK = 24MHz
     * SMCLK = MCLK/2 = 24Mhz
     * ACLK = REFO = 32Khz
     */
    MAP_CS_setDCOFrequency(48000000);
    MAP_CS_initClockSignal(CS_ACLK, CS_REFOCLK_SELECT, CS_CLOCK_DIVIDER_1);
    MAP_CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_2);
    MAP_CS_initClockSignal(CS_MCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);

    /* Configuring DMA module */
    MAP_DMA_enableModule();
    MAP_DMA_setControlBase(MSP_EXP432P401RLP_DMAControlTable);

    /* Setting Control Indexes. In this case we will set the source of the
     * DMA transfer to ADC14 Memory 6
     *  and the destination to the
     * destination data array. */
    MAP_DMA_setChannelControl(UDMA_PRI_SELECT | DMA_CH0_RESERVED0,
        UDMA_SIZE_16 | UDMA_SRC_INC_NONE | UDMA_DST_INC_16 | UDMA_ARB_1);
    MAP_DMA_setChannelTransfer(UDMA_PRI_SELECT | DMA_CH0_RESERVED0,
        UDMA_MODE_PINGPONG, (void*) &ADC14->MEM[6],
        prim_buffer0, NUMBER_OF_SAMPLES);

    MAP_DMA_setChannelControl(UDMA_ALT_SELECT | DMA_CH0_RESERVED0,
        UDMA_SIZE_16 | UDMA_SRC_INC_NONE | UDMA_DST_INC_16 | UDMA_ARB_1);
    MAP_DMA_setChannelTransfer(UDMA_ALT_SELECT | DMA_CH0_RESERVED0,
        UDMA_MODE_PINGPONG, (void*) &ADC14->MEM[6],
        alt_buffer0, NUMBER_OF_SAMPLES);

    /* Setting Control Indexes. In this case we will set the source of the
     * DMA transfer to ADC14 Memory 7
     *  and the destination to the
     * destination data array. */
    MAP_DMA_setChannelControl(UDMA_PRI_SELECT | DMA_CH1_RESERVED0,
        UDMA_SIZE_16 | UDMA_SRC_INC_NONE | UDMA_DST_INC_16 | UDMA_ARB_1);
    MAP_DMA_setChannelTransfer(UDMA_PRI_SELECT | DMA_CH1_RESERVED0,
        UDMA_MODE_PINGPONG, (void*) &ADC14->MEM[7],
        prim_buffer1, NUMBER_OF_SAMPLES);

    MAP_DMA_setChannelControl(UDMA_ALT_SELECT | DMA_CH1_RESERVED0,
        UDMA_SIZE_16 | UDMA_SRC_INC_NONE | UDMA_DST_INC_16 | UDMA_ARB_1);
    MAP_DMA_setChannelTransfer(UDMA_ALT_SELECT | DMA_CH1_RESERVED0,
        UDMA_MODE_PINGPONG, (void*) &ADC14->MEM[7],
        alt_buffer1, NUMBER_OF_SAMPLES);

    /* Setting Control Indexes. In this case we will set the source of the
     * DMA transfer to ADC14 Memory 8
     *  and the destination to the
     * destination data array. */
    MAP_DMA_setChannelControl(UDMA_PRI_SELECT | DMA_CH2_RESERVED0,
        UDMA_SIZE_16 | UDMA_SRC_INC_NONE | UDMA_DST_INC_16 | UDMA_ARB_1);
    MAP_DMA_setChannelTransfer(UDMA_PRI_SELECT | DMA_CH2_RESERVED0,
        UDMA_MODE_PINGPONG, (void*) &ADC14->MEM[8],
        prim_buffer2, NUMBER_OF_SAMPLES);

    MAP_DMA_setChannelControl(UDMA_ALT_SELECT | DMA_CH2_RESERVED0,
        UDMA_SIZE_16 | UDMA_SRC_INC_NONE | UDMA_DST_INC_16 | UDMA_ARB_1);
    MAP_DMA_setChannelTransfer(UDMA_ALT_SELECT | DMA_CH2_RESERVED0,
        UDMA_MODE_PINGPONG, (void*) &ADC14->MEM[8],
        alt_buffer2, NUMBER_OF_SAMPLES);

    /* Assigning/Enabling Interrupts */
    MAP_DMA_assignInterrupt(DMA_INT1, 0);
    MAP_DMA_assignInterrupt(DMA_INT2, 1);
    MAP_DMA_assignInterrupt(DMA_INT3, 2);

    MAP_Interrupt_enableInterrupt(INT_DMA_INT1);
    MAP_Interrupt_enableInterrupt(INT_DMA_INT2);
    MAP_Interrupt_enableInterrupt(INT_DMA_INT3);
    MAP_DMA_enableChannel(0);
    MAP_DMA_enableChannel(1);
    MAP_DMA_enableChannel(2);

    /* Setting reference voltage to 2.5 and enabling reference */
    MAP_REF_A_setReferenceVoltage(REF_A_VREF2_5V);
    MAP_REF_A_enableReferenceVoltage();

    /* Initializing ADC (MCLK/1/1) */
    MAP_ADC14_enableModule();
    MAP_ADC14_initModule(ADC_CLOCKSOURCE_ADCOSC, ADC_PREDIVIDER_1, ADC_DIVIDER_1,
            0);

    /* Configuring GPIOs for Analog In P4.5,4.6,4.7 */
    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P4,
            GPIO_PIN5 | GPIO_PIN6 | GPIO_PIN7, GPIO_TERTIARY_MODULE_FUNCTION);

    /* Debug: Configuring P1.0 as output */
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);

    /* Debug: Configure P2.0, P2.1 */
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN0);
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN1);

    /* Configuring ADC Memory (ADC_MEM6 - ADC_MEM8 (A6 - A8)  without repeat)
     * with internal 2.5v reference */
    MAP_ADC14_configureMultiSequenceMode(ADC_MEM6, ADC_MEM8, false);
    MAP_ADC14_configureConversionMemory(ADC_MEM6,
            ADC_VREFPOS_INTBUF_VREFNEG_VSS,
            ADC_INPUT_A6, ADC_NONDIFFERENTIAL_INPUTS);
    MAP_ADC14_configureConversionMemory(ADC_MEM7,
            ADC_VREFPOS_INTBUF_VREFNEG_VSS,
            ADC_INPUT_A7, ADC_NONDIFFERENTIAL_INPUTS);
    MAP_ADC14_configureConversionMemory(ADC_MEM8,
            ADC_VREFPOS_INTBUF_VREFNEG_VSS,
            ADC_INPUT_A8, ADC_NONDIFFERENTIAL_INPUTS);

    /*
     * Configuring the sample trigger to be sourced from Timer_A0 CCR1
     * and setting it to automatic iteration after it is triggered
     */
    MAP_ADC14_setSampleHoldTrigger(ADC_TRIGGER_SOURCE1, false);

    /*
     * Setting up the sample timer to automatically step through the sequence
     * convert.
     */
    MAP_ADC14_enableSampleTimer(ADC_AUTOMATIC_ITERATION);

    /*
     * Enabling the interrupt when a conversion on channel 8
     * and enabling conversions
     */
    MAP_ADC14_enableInterrupt(ADC_INT8);
    MAP_ADC14_enableConversion();
    /*
     * Clear IFGs before enabling interrupt
     */
    MAP_ADC14_clearInterruptFlag(0xFFFFFFFFFFFFFFFF);
    MAP_Interrupt_enableInterrupt(INT_ADC14);

    MAP_Interrupt_enableMaster();

    /* Starting the Timer */
    MAP_Timer_A_generatePWM(TIMER_A0_BASE, &timerA_PWM);

    while(1)
    {
        MAP_Interrupt_enableSleepOnIsrExit();
        MAP_PCM_gotoLPM0();
        __no_operation();
        MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN1);

        /* Computer real FFT using the completed data buffer */
        if (switch_data == 1)
        {
            tempPtr0 = prim_buffer0;
            tempPtr1 = prim_buffer1;
            tempPtr2 = prim_buffer2;
        }
        else
        {
            tempPtr0 = alt_buffer0;
            tempPtr1 = alt_buffer1;
            tempPtr2 = alt_buffer2;
        }
        average0 = 0;
        average1 = 0;
        average2 = 0;
        for (ii=0; ii<NUMBER_OF_SAMPLES; ii++)
        {
            average0 += tempPtr0[ii];
            average1 += tempPtr1[ii];
            average2 += tempPtr2[ii];
        }
        average0 = average0 >> 10;
        average1 = average1 >> 10;
        average2 = average2 >> 10;
        MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN1);
    }
}

/*
 * This interrupt is fired whenever the sequence is completed
 *
 */
__attribute__((ramfunc))
void ADC14_IRQHandler(void)
{
    uint64_t status;
    // Turn on LED

    //MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);
//    BITBAND_PERI(P1->OUT, 0) = 1;

    status = MAP_ADC14_getEnabledInterruptStatus();
    MAP_ADC14_clearInterruptFlag(status);
    if(status & ADC_INT8)
    {
        //MAP_ADC14_disableConversion();
        BITBAND_PERI(ADC14->CTL0, ADC14_CTL0_ENC_OFS) = 0;
        /* Forcing a software transfer on DMA Channel 0 */
        //MAP_DMA_requestSoftwareTransfer(0);
        BITBAND_PERI(DMA_Channel->SW_CHTRIG, DMA_SW_CHTRIG_CH0_OFS) = 1;
        /* Forcing a software transfer on DMA Channel 1 */
        //MAP_DMA_requestSoftwareTransfer(1);
        BITBAND_PERI(DMA_Channel->SW_CHTRIG, DMA_SW_CHTRIG_CH1_OFS) = 1;
        /* Forcing a software transfer on DMA Channel 2 */
        //MAP_DMA_requestSoftwareTransfer(2);
        BITBAND_PERI(DMA_Channel->SW_CHTRIG, DMA_SW_CHTRIG_CH2_OFS) = 1;
//        BITBAND_PERI(P1->OUT, 0) = 0;
        //MAP_ADC14_enableConversion();
        BITBAND_PERI(ADC14->CTL0, ADC14_CTL0_ENC_OFS) = 1;
    }
    //MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
}

/* Completion interrupt for ADC14 MEM6 */
__attribute__((ramfunc))
void DMA_INT1_IRQHandler(void)
{
    /* Switch between primary and alternate bufferes with DMA's PingPong mode */
    if (MAP_DMA_getChannelAttribute(0) & UDMA_ATTR_ALTSELECT)
    {
//        MAP_DMA_setChannelTransfer(UDMA_PRI_SELECT | DMA_CH0_RESERVED0,
//            UDMA_MODE_PINGPONG, (void*) &ADC14->MEM[6],
//            prim_buffer0, NUMBER_OF_SAMPLES);
        MSP_EXP432P401RLP_DMAControlTable[0].control =
                (MSP_EXP432P401RLP_DMAControlTable[0].control & 0xff000000 ) |
                (((NUMBER_OF_SAMPLES)-1)<<4) | UDMA_MODE_PINGPONG;
    }
    else
    {
//        MAP_DMA_setChannelTransfer(UDMA_ALT_SELECT | DMA_CH0_RESERVED0,
//            UDMA_MODE_PINGPONG, (void*) &ADC14->MEM[6],
//            alt_buffer0, NUMBER_OF_SAMPLES);
        MSP_EXP432P401RLP_DMAControlTable[8].control =
                (MSP_EXP432P401RLP_DMAControlTable[8].control & 0xff000000 ) |
                (((NUMBER_OF_SAMPLES)-1)<<4) | UDMA_MODE_PINGPONG;
    }
    // Toggle at n measures of adc6
//    MAP_GPIO_toggleOutputOnPin(GPIO_PORT_P2, GPIO_PIN0);
    P2->OUT ^= BIT0;
}

/* Completion interrupt for ADC14 MEM7 */
__attribute__((ramfunc))
void DMA_INT2_IRQHandler(void)
{
    /* Switch between primary and alternate bufferes with DMA's PingPong mode */
    if (MAP_DMA_getChannelAttribute(1) & UDMA_ATTR_ALTSELECT)
    {
//        DMA_setChannelTransfer(UDMA_PRI_SELECT | DMA_CH1_RESERVED0,
//            UDMA_MODE_PINGPONG, (void*) &ADC14->MEM[7],
//            prim_buffer1, NUMBER_OF_SAMPLES);
        MSP_EXP432P401RLP_DMAControlTable[1].control =
                (MSP_EXP432P401RLP_DMAControlTable[1].control & 0xff000000 ) |
                (((NUMBER_OF_SAMPLES)-1)<<4) | UDMA_MODE_PINGPONG;
    }
    else
    {
//        MAP_DMA_setChannelTransfer(UDMA_ALT_SELECT | DMA_CH1_RESERVED0,
//            UDMA_MODE_PINGPONG, (void*) &ADC14->MEM[7],
//            alt_buffer1, NUMBER_OF_SAMPLES);
        MSP_EXP432P401RLP_DMAControlTable[9].control =
                (MSP_EXP432P401RLP_DMAControlTable[9].control & 0xff000000 ) |
                (((NUMBER_OF_SAMPLES)-1)<<4) | UDMA_MODE_PINGPONG;
    }
    // Toggle at n measures of adc7
    //MAP_GPIO_toggleOutputOnPin(GPIO_PORT_P2, GPIO_PIN0);
    P2->OUT ^= BIT0;
}

/* Completion interrupt for ADC14 MEM8 */
__attribute__((ramfunc))
void DMA_INT3_IRQHandler(void)
{
    /* Switch between primary and alternate bufferes with DMA's PingPong mode */
    if (MAP_DMA_getChannelAttribute(2) & UDMA_ATTR_ALTSELECT)
    {
//        MAP_DMA_setChannelTransfer(UDMA_PRI_SELECT | DMA_CH2_RESERVED0,
//            UDMA_MODE_PINGPONG, (void*) &ADC14->MEM[8],
//            prim_buffer2, NUMBER_OF_SAMPLES);
        MSP_EXP432P401RLP_DMAControlTable[2].control =
                (MSP_EXP432P401RLP_DMAControlTable[2].control & 0xff000000 ) |
                (((NUMBER_OF_SAMPLES)-1)<<4) | UDMA_MODE_PINGPONG;
        switch_data = 1;
    }
    else
    {
//        MAP_DMA_setChannelTransfer(UDMA_ALT_SELECT | DMA_CH2_RESERVED0,
//            UDMA_MODE_PINGPONG, (void*) &ADC14->MEM[8],
//            alt_buffer2, NUMBER_OF_SAMPLES);
        MSP_EXP432P401RLP_DMAControlTable[10].control =
                (MSP_EXP432P401RLP_DMAControlTable[10].control & 0xff000000 ) |
                (((NUMBER_OF_SAMPLES)-1)<<4) | UDMA_MODE_PINGPONG;
        switch_data = 0;
    }
    // Toggle at n measures of adc8
    //MAP_GPIO_toggleOutputOnPin(GPIO_PORT_P2, GPIO_PIN0);
    P2->OUT ^= BIT0;
    MAP_Interrupt_disableSleepOnIsrExit();
}
