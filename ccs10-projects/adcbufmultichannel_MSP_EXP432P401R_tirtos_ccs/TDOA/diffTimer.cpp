#include <TDOA/diffTimer.h>
#include "common.h"

void initTimerParams(){
    Timer_Params_init(&params);
//    params.period = 1000000;
    params.periodUnits = Timer_PERIOD_US;
    params.timerMode = Timer_FREE_RUNNING;
}

void initTimer() {
    /* Setting up the timer in continuous callback mode that calls the callback
     * function every 1,000,000 microseconds, or 1 second.
     */
    initTimerParams();
    timer0 = Timer_open(CONFIG_TIMER_0_US_MEASURE, &params);
    if (timer0 == NULL) {
        /* Failed to initialized timer */
        while (1) {}
    }
}

void startTimer() {
    Timer_start(timer0);
}

void stopTimer() {
    Timer_stop(timer0);
}

uint32_t getTimerUs() {
    return Timer_getCount(timer0);
}
