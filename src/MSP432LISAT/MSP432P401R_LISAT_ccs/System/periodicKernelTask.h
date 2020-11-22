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

class PeriodicKernelTask
{
public:
    PeriodicKernelTask();
    void setupClockTask(uint32_t delayClockTicks, uint16_t periodClockTicks, void(*callback)());
    void setupClockMethod(uint32_t delayClockTicks, uint16_t periodClockTicks, void(*callback)(UArg), void* this_pointer);
    void startClockTask();
    void stopClockTask();
    void adjustClockCallback(void(*callback)());
    void setClockTimeout(uint32_t timeout);
    void setClockPeriod(uint16_t period);
private:
    Clock_Params clockParams;
    Clock_Handle myClock;
};

#endif /* SYSTEM_KERNELSINGLETASKCLOCK_H_ */
