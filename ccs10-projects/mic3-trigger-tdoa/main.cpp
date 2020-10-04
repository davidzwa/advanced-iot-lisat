#include "msp.h"

class LedToggler {
public:
    bool ledState = false;
    void toggle() {
        ledState = !ledState;
    }
} LedTogglers;

/**
 * main.c
 */
void main(void)
{
	WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;		// stop watchdog timer

	LedToggler* toggler = new LedToggler();
	toggler->toggle();
	toggler->toggle();
}
