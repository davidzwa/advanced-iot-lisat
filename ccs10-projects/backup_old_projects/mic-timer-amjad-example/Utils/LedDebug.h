/*
 * LedDebug.h
 *
 *  Created on: Oct 10, 2020
 *      Author: david
 */

#ifndef UTILS_LEDDEBUG_H_
#define UTILS_LEDDEBUG_H_

class LedDebug
{
public:
    LedDebug();
    bool ledState = false;
    void setupPort1();
    void toggle();
};

#endif /* UTILS_LEDDEBUG_H_ */
