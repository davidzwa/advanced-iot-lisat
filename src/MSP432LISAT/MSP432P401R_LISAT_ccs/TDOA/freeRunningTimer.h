#include <ti/drivers/Timer.h>

#ifndef TDOA_DIFFTIMER_H_
#define TDOA_DIFFTIMER_H_

Timer_Handle timer0;
Timer_Handle timer1;
Timer_Params params;

void initTimer();
void startTimer();
void stopTimer();
uint32_t getTimerUs();

void initTimerTacho();
void startTimerTacho();
void stopTimerTacho();
uint32_t getTimerUsTacho();

#endif /* TDOA_DIFFTIMER_H_ */
