#include "common.h"
#include <stdio.h>
#include "Timer/timer.h"
#include "Utils/LedDebug.h"
#include <ti/drivers/UART.h>

void timer_callback() {
    return;
}

void adc_init_channel_2(void) {
    ADC14->CTL0 &= ~ADC14_CTL0_ENC;             // to allow programming
    while(ADC14->CTL0&0x00010000){};            // wait for BUSY to be zero
    ADC14->CTL0 = ADC14_CTL0_ON | ADC14_CTL0_MSC | ADC14_CTL0_SHT0__32 | ADC14_CTL0_SHT1__32 | ADC14_CTL0_CONSEQ_1 |
                ADC14_CTL0_SSEL__SMCLK | ADC14_CTL0_DIV__1 | ADC14_CTL0_SHP ;          //  single, SMCLK, on, disabled, /1, 32 SHM
    ADC14->CTL1 = 0x00000030;                   // ADC14MEM0, 14-bit, ref on, regular power
    ADC14->MCTL[3] = ADC14_MCTLN_INCH_2;          // 0 to 3.3V, channel 17
    ADC14->IER0 = 0; // no interrupts
    ADC14->IER1 = 0; // no interrupts

    P5->SEL1 |= BIT3;
    P5->SEL0 |= BIT3;

    ADC14->CTL0 |= ADC14_CTL0_ENC;         // enable
}

void port_3_pin_3_int_init(){
    // configure pin 3.3 as input and enable interrupt
    P3->SEL0 &= ~BIT3;
    P3->SEL1 &= ~BIT3;
    P3->DIR &= ~BIT3;

    P3->IFG &= ~BIT3; // ensure that the interrupt flag is initially clear
    P3->IES &= ~BIT3;  // rising edge
    P3->IE |= BIT3;  // arm (enable) interrupt on P3.3 and P3.3
}

void PORT3_IRQHandler(void){
    if(P3->IFG & BIT3){
        P3->IFG &= ~BIT3;
        TIMER_A0->CTL |= 0x0014;      // reset and start Timer A1 in up mode
    }
}

/**
 * main.c
 */
void main(void)
{
	WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;		// stop watchdog timer

    adc_init_channel_2();

	Timer *timerA0 = new Timer();
	timerA0->init(timer_callback);

	LedDebug* toggler = new LedDebug();
	toggler->setupPort1();

    port_3_pin_3_int_init();

    printf("asd");

    volatile int i = 0;
    while(1)
    {

        toggler->toggle();
        // delay for a while
        for (i = 0; i < 0x10000; i++);
    }
}
