#include <ti/drivers/Timer.h>

#ifndef TDOA_DIFFTIMER_H_
#define TDOA_DIFFTIMER_H_

Timer_Handle timer0;
Timer_Params params;

void initTimer();
void startTimer();
void stopTimer();
uint32_t getTimerUs();

#endif /* TDOA_DIFFTIMER_H_ */
