#include <stdint.h>
#include "msp.h"




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


void adc_init_channel_2(void){
     ADC14->CTL0 &= ~ADC14_CTL0_ENC;        // to allow programming
  while(ADC14->CTL0&0x00010000){};          // wait for BUSY to be zero
  ADC14->CTL0 = ADC14_CTL0_ON | ADC14_CTL0_MSC | ADC14_CTL0_SHT0__32 | ADC14_CTL0_SHT1__32 | ADC14_CTL0_CONSEQ_1 |
                ADC14_CTL0_SSEL__SMCLK | ADC14_CTL0_DIV__1 | ADC14_CTL0_SHP ;          //  single, SMCLK, on, disabled, /1, 32 SHM
  ADC14->CTL1 = 0x00000030;                 // ADC14MEM0, 14-bit, ref on, regular power
  ADC14->MCTL[3] = ADC14_MCTLN_INCH_2;          // 0 to 3.3V, channel 17
  ADC14->IER0 = 0; // no interrupts
  ADC14->IER1 = 0; // no interrupts

  P5->SEL1 |= BIT3;
  P5->SEL0 |= BIT3;

  ADC14->CTL0 |= ADC14_CTL0_ENC;         // enable
}

volatile uint32_t  debug_reg = 0;
volatile uint32_t adc14_mem_3 =0;

void read_adc_2(){
    while(ADC14->CTL0&0x00010000){};    // wait for BUSY to be zero

      //Todo optimize when you like
      ADC14->CTL0 |=  ADC14_CTL0_SC;
      while(debug_reg != 0x08){
          debug_reg = ADC14->IFGR0;
      };
      adc14_mem_3 = ADC14->MEM[3];      // return result 0 to 16383

        // disable P3.3 interrupt
        P3->IE &= ~BIT3;  // arm (enable) interrupt on P3.3 and P3.3

        if (number of samples == what we need){
            // enable P3.3 interrupt
            P3->IE |= BIT3;  // arm (enable) interrupt on P3.3 and P3.3
            timerA0_stop();
        }
}



void (*timerA1_task)(void);   // a hook for user defined function

/** -------------- TimerA0_Init ---------------
 * Initialize Timer A0 to run user task periodically
 * Inputs:  task is a pointer to a user define function
 *          period specifies the interrupt frequency (every period/SMCLK sec)
 * Outputs: none
 */

// sampling frequency = 500k/period
void timerA0_init(void(*task)(void), uint16_t period){

  // Hook the user task to the callback
    timerA1_task = task;

  TIMER_A0->CTL &=~0x0030;  // MC=00b, halt timer A1
  TIMER_A0->CTL |= 0x0200;  // TASSL=10b, use SMCLK (max speed 12MHz)
  TIMER_A0->CTL |= 0x0080;  // ID=10b, Clock /4

  TIMER_A0->CCTL[0] = 0x0010;   //bit4=1b, enable capture/compare interrupt request of CCIFG
                  //bit8=0b, campare mode
                  //bit15-14=00b, no capture mode
  TIMER_A0->CCR[0]=(period-1);  // compare match value
  TIMER_A0->EX0 = 0x0005;     // divide the clock by 6
  // To set the priority
  // // Look at table 5.1 from the book "Real-Time Interfacing to the MSP432 Microcontoller"
  // // First, select the priority register number from the column "NNIC priority"
  // // Then specify the priority bits position from the columnm "Prioity bits"
  // // set the priority. Notice 00000000b highest, 00100000b (0x20) = priority 1, and 0xe0 = priority 7 (lowest)
  NVIC->IP[2] = (NVIC->IP[2]& 0xff00ffff) | 0x0040000;  
  NVIC->ISER[0] = 0x00000400;   // enable interrupt 10 in NVIC
}


/** --------------timerA1_stop---------------
 * Deactivate the interrupt running a user task periodically.
 * Input: none
 * Output: none
 */
void timerA0_stop(void){
  TIMER_A0->CTL &= ~0x0030;       // halt Timer A0
  NVIC->ICER[0] = 0x00000400;   // disable TimerA1 interrupt 
}

void TA0_0_IRQHandler(void){
  TIMER_A0->CCTL[0] &=~0x0001; // clear the interrupt flag
  (*timerA0_task)();
}


int main(int argc, char const *argv[])
{
    adc_init_channel_2();
    timerA0_init(read_adc_2, 5000);
    port_3_pin_3_int_init();
    
  return 0;
}
