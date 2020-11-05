#include <ti/drivers/Timer.h>

#ifndef TDOA_DIFFTIMER_H_
#define TDOA_DIFFTIMER_H_

Timer_Handle timer33Khz;
Timer_Params params;

void initTimerTacho();
void startTimerTacho();
void stopTimerTacho();
uint32_t getTimerUsTacho();

#endif /* TDOA_DIFFTIMER_H_ */
