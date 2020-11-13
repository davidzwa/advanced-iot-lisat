/*
 * kernelSingleTaskClock.h
 *
 *  Created on: Nov 11, 2020
 *      Author: david
 */

#include <xdc/std.h>
#include <xdc/runtime/Error.h>
#include <xdc/runtime/System.h>
#include <ti/sysbios/knl/Clock.h>

#ifndef SYSTEM_KERNELSINGLETASKCLOCK_H_
#define SYSTEM_KERNELSINGLETASKCLOCK_H_

class KernelSingleTaskClock
{
public:
    KernelSingleTaskClock();
    void setupClockTask(uint32_t delayClockTicks, uint16_t periodClockTicks, Clock_FuncPtr clockCallback);
    void startClockTask();
    void stopClockTask();
    void setClockCallback(Clock_FuncPtr callback);
    void setClockTimeout(uint32_t timeout);
    void setClockPeriod(uint16_t period);
private:
    Clock_Params clockParams;
    Clock_Handle myClock;
};

#endif /* SYSTEM_KERNELSINGLETASKCLOCK_H_ */
