//****************************************************************************
//
// main.c - MSP-EXP432P401R + Educational Boosterpack MkII - Microphone FFT
//
//          CMSIS DSP Software Library is used to perform 512-point FFT on
//          the audio samples collected with MSP432's ADC14 from the Education
//          Boosterpack's onboard microphone. The resulting frequency bin data
//          is displayed on the BoosterPack's 128x128 LCD.
//
//****************************************************************************

#include <ti/devices/msp432p4xx/inc/msp.h>
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include <ti/grlib/grlib.h>

#include "LcdDriver/Crystalfontz128x128_ST7735.h"
#include <stdio.h>
#include <arm_math.h>
#include <arm_const_structs.h>

#define SAMPLE_LENGTH 512
#define BLOCK_SIZE 512

/* ------------------------------------------------------------------
 * Global variables for FFT Bin Example
 * ------------------------------------------------------------------- */
uint32_t fftSize = SAMPLE_LENGTH;
uint32_t ifftFlag = 0;
uint32_t doBitReverse = 1;
volatile arm_status status;

#define SMCLK_FREQUENCY     48000000
#define SAMPLE_FREQUENCY    12000

/* DMA Control Table */
#if defined(__TI_COMPILER_VERSION__)
#pragma DATA_ALIGN(MSP_EXP432P401RLP_DMAControlTable, SAMPLE_LENGTH*2)
#elif defined(__IAR_SYSTEMS_ICC__)
#pragma data_alignment=SAMPLE_LENGTH*2
#elif defined(__GNUC__)
__attribute__ ((aligned (SAMPLE_LENGTH*2)))
#elif defined(__CC_ARM)
__align(1024)
#endif
static DMA_ControlTable MSP_EXP432P401RLP_DMAControlTable[32];

/* FFT data/processing buffers*/
float hann[SAMPLE_LENGTH];
int16_t data_array1[SAMPLE_LENGTH];
int16_t data_array2[SAMPLE_LENGTH];
int16_t data_input_filtered[SAMPLE_LENGTH];

int16_t data_input[SAMPLE_LENGTH * 2];
int16_t data_output[SAMPLE_LENGTH];

/* FIR data/state */
// http://t-filter.engineerjs.com/
#define NUM_TAPS 26
static q15_t firStateF32[BLOCK_SIZE + NUM_TAPS -1];
q15_t firCoeffs32LP[NUM_TAPS] = {
                                 272,
                                   449,
                                   -266,
                                   -540,
                                   -55,
                                   -227,
                                   -1029,
                                   745,
                                   3927,
                                   1699,
                                   -5616,
                                   -6594,
                                   2766,
                                   9228,
                                   2766,
                                   -6594,
                                   -5616,
                                   1699,
                                   3927,
                                   745,
                                   -1029,
                                   -227,
                                   -55,
                                   -540,
                                   -266,
                                   449
};
//                                 1,0,0,1,1,1,-1,-3,-5,-3,4,10,10,10,0,0,14,4,-3,-5,-3,-1,1,1,1,0,0,0};

volatile int switch_data = 0;

/* Timer_A PWM Configuration Parameter */
Timer_A_PWMConfig pwmConfig =
{
    TIMER_A_CLOCKSOURCE_SMCLK,
    TIMER_A_CLOCKSOURCE_DIVIDER_1,
    (SMCLK_FREQUENCY / SAMPLE_FREQUENCY),
    TIMER_A_CAPTURECOMPARE_REGISTER_1,
    TIMER_A_OUTPUTMODE_SET_RESET,
    (SMCLK_FREQUENCY / SAMPLE_FREQUENCY) / 2
};

int main(void)
{
    /* Halting WDT and disabling master interrupts */
    MAP_WDT_A_holdTimer();
    MAP_Interrupt_disableMaster();

    /* Set the core voltage level to VCORE1 */
    MAP_PCM_setCoreVoltageLevel(PCM_VCORE1);

    /* Set 2 flash wait states for Flash bank 0 and 1*/
    MAP_FlashCtl_setWaitState(FLASH_BANK0, 2);
    MAP_FlashCtl_setWaitState(FLASH_BANK1, 2);

    /* Initializes Clock System */
    MAP_CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_48);
    MAP_CS_initClockSignal(CS_MCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
    MAP_CS_initClockSignal(CS_HSMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
    MAP_CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
    MAP_CS_initClockSignal(CS_ACLK, CS_REFOCLK_SELECT, CS_CLOCK_DIVIDER_1);

    // Initialize Hann Window
    int n;
    for(n = 0; n < SAMPLE_LENGTH; n++)
    {
        hann[n] = 0.5f - 0.5f * cosf((2 * PI * n) / (SAMPLE_LENGTH - 1));
    }
    // Initialize FIR instance
    uint32_t blockSize = BLOCK_SIZE;
    arm_fir_instance_q15 S;
    arm_fir_init_q15(&S, NUM_TAPS, firCoeffs32LP, firStateF32, blockSize);

    /* Configuring Timer_A to have a period of approximately 500ms and
     * an initial duty cycle of 10% of that (3200 ticks)  */
    Timer_A_generatePWM(TIMER_A0_BASE, &pwmConfig);

    /* Initializing ADC (MCLK/1/1) */
    ADC14_enableModule();
    ADC14_initModule(ADC_CLOCKSOURCE_MCLK, ADC_PREDIVIDER_1, ADC_DIVIDER_1, 0);

    ADC14_setSampleHoldTrigger(ADC_TRIGGER_SOURCE1, false);

    /* Configuring GPIOs (4.3 A10) */
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P4, GPIO_PIN3,
                                               GPIO_TERTIARY_MODULE_FUNCTION);

    /* Configuring ADC Memory */
    ADC14_configureSingleSampleMode(ADC_MEM0, true);
    ADC14_configureConversionMemory(ADC_MEM0, ADC_VREFPOS_AVCC_VREFNEG_VSS,
                                    ADC_INPUT_A10, false);

    /* Set ADC result format to signed binary */
    ADC14_setResultFormat(ADC_SIGNED_BINARY);

    /* Configuring DMA module */
    DMA_enableModule();
    DMA_setControlBase(MSP_EXP432P401RLP_DMAControlTable);

    DMA_disableChannelAttribute(DMA_CH7_ADC14,
                                UDMA_ATTR_ALTSELECT | UDMA_ATTR_USEBURST |
                                UDMA_ATTR_HIGH_PRIORITY |
                                UDMA_ATTR_REQMASK);

    /* Setting Control Indexes. In this case we will set the source of the
     * DMA transfer to ADC14 Memory 0
     *  and the destination to the
     * destination data array. */
    MAP_DMA_setChannelControl(
        UDMA_PRI_SELECT | DMA_CH7_ADC14,
        UDMA_SIZE_16 | UDMA_SRC_INC_NONE |
        UDMA_DST_INC_16 | UDMA_ARB_1);
    MAP_DMA_setChannelTransfer(UDMA_PRI_SELECT | DMA_CH7_ADC14,
                               UDMA_MODE_PINGPONG, (void*) &ADC14->MEM[0],
                               data_array1, SAMPLE_LENGTH);

    MAP_DMA_setChannelControl(
        UDMA_ALT_SELECT | DMA_CH7_ADC14,
        UDMA_SIZE_16 | UDMA_SRC_INC_NONE |
        UDMA_DST_INC_16 | UDMA_ARB_1);
    MAP_DMA_setChannelTransfer(UDMA_ALT_SELECT | DMA_CH7_ADC14,
                               UDMA_MODE_PINGPONG, (void*) &ADC14->MEM[0],
                               data_array2, SAMPLE_LENGTH);

    /* Assigning/Enabling Interrupts */
    MAP_DMA_assignInterrupt(DMA_INT1, 7);
    MAP_Interrupt_enableInterrupt(INT_DMA_INT1);
    MAP_DMA_assignChannel(DMA_CH7_ADC14);
    MAP_DMA_clearInterruptFlag(7);
    MAP_Interrupt_enableMaster();

    /* Now that the DMA is primed and setup, enabling the channels. The ADC14
     * hardware should take over and transfer/receive all bytes */
    MAP_DMA_enableChannel(7);
    MAP_ADC14_enableConversion();

    volatile int i;
//    WDTCTL = WDTPW | WDTHOLD;
    P1DIR = 0x01; // set up bit 0 of P1 as output
    P1OUT = 0x00; // intialize bit 0 of P1 to 0

    while(1)
    {
        // toggle bit 0 of P1
        P1OUT ^= 0x01;
//        // delay for a while
//        for (i = 0; i < 0x600000; i++);

        MAP_PCM_gotoLPM0();

        int i = 0;
//        arm_iir_lattice_q15(S, pSrc, pDst, blockSize);
//        (&instance, data_array1, data_input);

        /* Computer real FFT using the completed data buffer */
        if(switch_data & 1)
        {
            arm_fir_q15(&S, &data_array1, &data_input_filtered, blockSize);
            for(i = 0; i < SAMPLE_LENGTH; i++)
            {
                data_input_filtered[i] = (int16_t)(hann[i] * data_input_filtered[i]);
            }
            arm_rfft_instance_q15 instance;
            status = arm_rfft_init_q15(&instance, fftSize, ifftFlag,
                                       doBitReverse);

            arm_rfft_q15(&instance, data_array1, data_input);
        }
        else
        {
            arm_fir_q15(&S, &data_array2, &data_input_filtered, blockSize);
            for(i = 0; i < SAMPLE_LENGTH; i++)
            {
                data_input_filtered[i] = (int16_t)(hann[i] * data_input_filtered[i]);
            }
            arm_rfft_instance_q15 instance;
            status = arm_rfft_init_q15(&instance, fftSize, ifftFlag,
                                       doBitReverse);

            arm_rfft_q15(&instance, data_array2, data_input);
        }

        /* Calculate magnitude of FFT complex output */
        for(i = 0; i < SAMPLE_LENGTH*2; i += 2)
        {
            data_output[i /
                        2] =
                (int32_t)(sqrtf((data_input[i] *
                                 data_input[i]) +
                                (data_input[i + 1] * data_input[i + 1])));
        }

        q15_t maxValue;
        uint32_t maxIndex = 0;

        arm_max_q15(data_output, fftSize, &maxValue, &maxIndex);

    }
}

/* Completion interrupt for ADC14 MEM0 */
void DMA_INT1_IRQHandler(void)
{
    /* Switch between primary and alternate buffers with DMA's PingPong mode */
    if(DMA_getChannelAttribute(7) & UDMA_ATTR_ALTSELECT)
    {
        DMA_setChannelControl(
            UDMA_PRI_SELECT | DMA_CH7_ADC14,
            UDMA_SIZE_16 | UDMA_SRC_INC_NONE |
            UDMA_DST_INC_16 | UDMA_ARB_1);
        DMA_setChannelTransfer(UDMA_PRI_SELECT | DMA_CH7_ADC14,
                               UDMA_MODE_PINGPONG, (void*) &ADC14->MEM[0],
                               data_array1, SAMPLE_LENGTH);
        switch_data = 1;
    }
    else
    {
        DMA_setChannelControl(
            UDMA_ALT_SELECT | DMA_CH7_ADC14,
            UDMA_SIZE_16 | UDMA_SRC_INC_NONE |
            UDMA_DST_INC_16 | UDMA_ARB_1);
        DMA_setChannelTransfer(UDMA_ALT_SELECT | DMA_CH7_ADC14,
                               UDMA_MODE_PINGPONG, (void*) &ADC14->MEM[0],
                               data_array2, SAMPLE_LENGTH);
        switch_data = 0;
    }
}
