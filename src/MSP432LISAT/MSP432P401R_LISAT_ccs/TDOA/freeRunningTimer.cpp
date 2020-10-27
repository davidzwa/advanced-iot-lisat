#include <TDOA/freeRunningTimer.h>
#include "common.h"

void initTimerParams(){
    Timer_Params_init(&params);
    params.periodUnits = Timer_PERIOD_COUNTS;
    params.timerMode = Timer_FREE_RUNNING;
}

void initTimer() {
    /*
     * Setting up the timer in FREE RUNNING mode, allowing us to share it across multiple tasks.
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

void initTimerTacho() {
    /*
     * Setting up the timer in FREE RUNNING mode, allowing us to share it across multiple tasks.
     */
    initTimerParams();
    timer1 = Timer_open(TIMER_TACHO_MEASURE, &params);
    if (timer1 == NULL) {
        /* Failed to initialized timer */
        while (1) {}
    }
}

void startTimerTacho() {
    Timer_start(timer1);
}

void stopTimerTacho() {
    Timer_stop(timer1);
}

uint32_t getTimerUsTacho() {
    return Timer_getCount(timer1);
}


