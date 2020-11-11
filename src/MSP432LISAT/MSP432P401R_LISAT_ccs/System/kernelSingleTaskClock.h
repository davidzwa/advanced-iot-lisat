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
    void setupClockHandler(uint32_t delay);
    Clock_Handle* getClockHandle();
    private:
    Clock_Params clockParams;
    Clock_Handle myClock;
};

#endif /* SYSTEM_KERNELSINGLETASKCLOCK_H_ */
