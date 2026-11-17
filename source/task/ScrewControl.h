#ifndef SCREW_CONTROL_H
#define SCREW_CONTROL_H

#include "MotorControl.h"

/* 由 MotorControl_Task 每周期调用，更新两个拧螺丝电机的速度目标。 */
void ScrewControl_Update(const MotorFeedback_t *feedback,
                         MotorCommand_t commands[TOTAL_MOTOR_COUNT]);

#endif /* SCREW_CONTROL_H */
