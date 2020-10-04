/*
* This interrupt is fired whenever the sequence is completed
* Copied from https://e2e.ti.com/support/microcontrollers/msp430/f/166/t/526735
*
*/
__attribute__((ramfunc)) void ADC14_IRQHandler(void)
{
    uint64_t status;
    // Turn on LED

    // Toggle at 12Khz frequency, look like 6khz clock
    //MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);
    P1OUT |= BIT0;

    status = ADC14_getEnabledInterruptStatus();
    MAP_ADC14_clearInterruptFlag(status);
    if (status & ADC_INT8)
    {
        MAP_ADC14_disableConversion();
        /* Forcing a software transfer on DMA Channel 0 */
        MAP_DMA_requestSoftwareTransfer(0);
        /* Forcing a software transfer on DMA Channel 1 */
        MAP_DMA_requestSoftwareTransfer(1);
        /* Forcing a software transfer on DMA Channel 2 */
        MAP_DMA_requestSoftwareTransfer(2);

        MAP_ADC14_enableConversion();
    }
    //MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
    P1OUT &= ~BIT0;
}

/* Completion interrupt for ADC14 MEM6 */
__attribute__((ramfunc)) void DMA_INT1_IRQHandler(void)
{
    /* Switch between primary and alternate bufferes with DMA's PingPong mode */
    if (DMA_getChannelAttribute(0) & UDMA_ATTR_ALTSELECT)
    {
        DMA_setChannelControl(UDMA_PRI_SELECT | DMA_CH0_RESERVED0,
                              UDMA_SIZE_16 | UDMA_SRC_INC_NONE | UDMA_DST_INC_16 | UDMA_ARB_1);
        DMA_setChannelTransfer(UDMA_PRI_SELECT | DMA_CH0_RESERVED0,
                               UDMA_MODE_PINGPONG, (void *)&ADC14->MEM[6],
                               prim_buffer0, NUMBER_OF_SAMPLES);
    }
    else
    {
        DMA_setChannelControl(UDMA_ALT_SELECT | DMA_CH0_RESERVED0,
                              UDMA_SIZE_16 | UDMA_SRC_INC_NONE | UDMA_DST_INC_16 | UDMA_ARB_1);
        DMA_setChannelTransfer(UDMA_ALT_SELECT | DMA_CH0_RESERVED0,
                               UDMA_MODE_PINGPONG, (void *)&ADC14->MEM[6],
                               alt_buffer0, NUMBER_OF_SAMPLES);
    }
    // Toggle at 512 measures of adc6
    MAP_GPIO_toggleOutputOnPin(GPIO_PORT_P2, GPIO_PIN0);
}

/* Completion interrupt for ADC14 MEM7 */
__attribute__((ramfunc)) void DMA_INT2_IRQHandler(void)
{
    /* Switch between primary and alternate bufferes with DMA's PingPong mode */
    if (DMA_getChannelAttribute(1) & UDMA_ATTR_ALTSELECT)
    {
        DMA_setChannelControl(UDMA_PRI_SELECT | DMA_CH1_RESERVED0,
                              UDMA_SIZE_16 | UDMA_SRC_INC_NONE | UDMA_DST_INC_16 | UDMA_ARB_1);
        DMA_setChannelTransfer(UDMA_PRI_SELECT | DMA_CH1_RESERVED0,
                               UDMA_MODE_PINGPONG, (void *)&ADC14->MEM[7],
                               prim_buffer1, NUMBER_OF_SAMPLES);
    }
    else
    {
        DMA_setChannelControl(UDMA_ALT_SELECT | DMA_CH1_RESERVED0,
                              UDMA_SIZE_16 | UDMA_SRC_INC_NONE | UDMA_DST_INC_16 | UDMA_ARB_1);
        DMA_setChannelTransfer(UDMA_ALT_SELECT | DMA_CH1_RESERVED0,
                               UDMA_MODE_PINGPONG, (void *)&ADC14->MEM[7],
                               alt_buffer1, NUMBER_OF_SAMPLES);
    }
    // Toggle at 512 measures of adc7
    MAP_GPIO_toggleOutputOnPin(GPIO_PORT_P2, GPIO_PIN0);
}

/* Completion interrupt for ADC14 MEM8 */
__attribute__((ramfunc)) void DMA_INT3_IRQHandler(void)
{
    /* Switch between primary and alternate bufferes with DMA's PingPong mode */
    if (DMA_getChannelAttribute(2) & UDMA_ATTR_ALTSELECT)
    {
        DMA_setChannelControl(UDMA_PRI_SELECT | DMA_CH2_RESERVED0,
                              UDMA_SIZE_16 | UDMA_SRC_INC_NONE | UDMA_DST_INC_16 | UDMA_ARB_1);
        DMA_setChannelTransfer(UDMA_PRI_SELECT | DMA_CH2_RESERVED0,
                               UDMA_MODE_PINGPONG, (void *)&ADC14->MEM[8],
                               prim_buffer2, NUMBER_OF_SAMPLES);
        switch_data = 1;
    }
    else
    {
        DMA_setChannelControl(UDMA_ALT_SELECT | DMA_CH2_RESERVED0,
                              UDMA_SIZE_16 | UDMA_SRC_INC_NONE | UDMA_DST_INC_16 | UDMA_ARB_1);
        DMA_setChannelTransfer(UDMA_ALT_SELECT | DMA_CH2_RESERVED0,
                               UDMA_MODE_PINGPONG, (void *)&ADC14->MEM[8],
                               alt_buffer2, NUMBER_OF_SAMPLES);
        switch_data = 0;
    }
    // Toggle at 512 measures of adc8
    MAP_GPIO_toggleOutputOnPin(GPIO_PORT_P2, GPIO_PIN0);
    //MAP_Interrupt_disableSleepOnIsrExit();
    SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Pos;
}
