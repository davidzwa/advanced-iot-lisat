/*
 * Timer.cpp
 *
 *  Created on: Oct 4, 2020
 *      Author: david
 */

#include <Timer/Timer.h>
#include <ti/devices/msp432p4xx/inc/msp.h>

void (*timer_task)(void);   // a hook for user defined function

void timerA0_stop(void){
  TIMER_A0->CTL &= ~0x0030;       // halt Timer A0
  NVIC->ICER[0] = 0x00000400;   // disable TimerA1 interrupt
}

void TA0_0_IRQHandler(void){
  TIMER_A0->CCTL[0] &=~0x0001; // clear the interrupt flag
  (*timer_task)();
}

///** -------------- TimerA0_Init ---------------
// * Initialize Timer A0 to run user task periodically
// * Inputs:  task is a pointer to a user define function
// *          period specifies the interrupt frequency (every period/SMCLK sec)
// * Outputs: none
// */
//
//// sampling frequency = 500k/period
void timerA0_init(uint16_t period){
    TIMER_A0->CTL &=~0x0030;  // MC=00b, halt timer A1
    TIMER_A0->CTL |= 0x0200;  // TASSL=10b, use SMCLK (max speed 12MHz)
    TIMER_A0->CTL |= 0x0080;  // ID=10b, Clock /4

    TIMER_A0->CCTL[0] = 0x0010;   //bit4=1b, enable capture/compare interrupt request of CCIFG
                  //bit8=0b, compare mode
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

Timer::Timer()
{
    // TODO Auto-generated constructor stub

}

void Timer::init(void(*task)(void)) {
    // Hook the user task to the callback
    timer_task = task;

    // Setup the timer at a 'nice' 1/5000 frequency vs SMCLK at 12MHz/4/6 = 500kHz
    timerA0_init(5000);
}

