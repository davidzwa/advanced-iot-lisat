#include <System/freeRunningTimer.h>
#include "common.h"

void initTimerParams(){
    Timer_Params_init(&params);
    params.periodUnits = Timer_PERIOD_COUNTS;
    params.timerMode = Timer_FREE_RUNNING;
}

void initTimerTacho() {
    /*
     * Setting up the timer in FREE RUNNING mode, allowing us to share it across multiple tasks.
     */
    initTimerParams();
    timer33Khz = Timer_open(TIMER_FREE_MEASURE, &params);
    if (timer33Khz == NULL) {
        /* Failed to initialized timer */
        while (1) {}
    }
}

void startTimerTacho() {
    Timer_start(timer33Khz);
}

void stopTimerTacho() {
    Timer_stop(timer33Khz);
}

uint32_t getTimerUsTacho() {
    return Timer_getCount(timer33Khz);
}


