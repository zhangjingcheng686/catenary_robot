#ifndef SCREW_CONTROL_H
#define SCREW_CONTROL_H

#include "MotorControl.h"

#define SCREW_WORK_TORQUE_NM          1.5f
#define SCREW_TORQUE_THRESHOLD_NM     2.0f
#define SCREW_TORQUE_CONFIRM_COUNT    5U
#define SCREW_STARTUP_GRACE_MS        200U
#define SCREW_STALL_WINDOW_MS         300U
#define SCREW_STALL_POSITION_RAD      0.01f
#define SCREW_FEEDBACK_MAX_GAP_MS     60U
/* 两个拧螺丝电机均按 DM4340 初始化，反馈范围为 -12.5 至 12.5 rad。 */
#define SCREW_POSITION_PERIOD_RAD     25.0f

/* 切换模式时重新开始运动监测，保留已经锁定的停止状态。 */
void ScrewControl_ResetMonitoring(void);

/* 由 MotorControl_Task 每周期调用，更新两个拧螺丝电机的 MIT 力矩目标。 */
void ScrewControl_Update(const MotorFeedback_t *feedback,
                         MotorCommand_t commands[TOTAL_MOTOR_COUNT]);

#endif /* SCREW_CONTROL_H */
