#include <ti/devices/msp432p4xx/inc/msp.h>
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

#include "Timer/timer.h"

class LedToggler {
public:
    bool ledState = false;
    void toggle() {
        ledState = !ledState;
    }
} LedTogglers;

void timer_callback() {
    return;
}

/**
 * main.c
 */
void main(void)
{
	WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;		// stop watchdog timer

	Timer *timerA0 = new Timer();
	timerA0->init(timer_callback);

	LedToggler* toggler = new LedToggler();
	toggler->toggle();
	toggler->toggle();

    P1DIR = 0x01; // set up bit 0 of P1 as output
    P1OUT = 0x00; // intialize bit 0 of P1 to 0
    volatile int i = 0;
    while(1)
    {
        // toggle bit 0 of P1
        P1OUT ^= 0x01;
//        // delay for a while
        for (i = 0; i < 0x600000; i++);
    }
}
