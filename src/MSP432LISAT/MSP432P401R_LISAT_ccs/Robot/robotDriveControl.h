/*
 * robotDriveControl.h
 *
 *  Created on: Nov 22, 2020
 *      Author: d.zwart
 */
#include <common.h>
#include <System/kernelSingleTaskClock.h>

#ifndef ROBOT_ROBOTDRIVECONTROL_H_
#define ROBOT_ROBOTDRIVECONTROL_H_

class RobotDriveControl
{
public:
    RobotDriveControl();
private:
    KernelSingleTaskClock* controlTaskClock = new KernelSingleTaskClock();
};

#endif /* ROBOT_ROBOTDRIVECONTROL_H_ */
