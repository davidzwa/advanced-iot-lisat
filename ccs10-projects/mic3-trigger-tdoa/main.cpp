#include "msp.h"
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
}
