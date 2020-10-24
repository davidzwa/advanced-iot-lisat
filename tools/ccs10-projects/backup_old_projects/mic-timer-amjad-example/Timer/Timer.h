/*
 * Timer.h
 *
 *  Created on: Oct 4, 2020
 *      Author: david
 */

#include <stdint.h>

#ifndef TIMER_H_
#define TIMER_H_

class Timer
{
public:
    Timer();
    void init(void(*task)(void));
};

#endif /* TIMER_H_ */
