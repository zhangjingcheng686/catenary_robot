#include "RobotManager.h"
#include "MotorControl.h"
#include "DBusSys.h"
#include "cmsis_os2.h"

/* 管理任务只提交模式，不挂起任务，不发送电机指令。 */
void RobotManager_Task(void *argument)
{
    (void)argument;
    for (;;) {
        RobotMode_t mode = ROBOT_MODE_IDLE;
        if (remoter.online) {
            int ch4 = remoter.rc.ch[4];
            if (ch4 < 980) {
                mode = ROBOT_MODE_RL;
            } else if (ch4 > 1020) {
                mode = ROBOT_MODE_TEACH;
            }
        }
        MotorControl_RequestMode(mode);
        osDelay(20);
    }
}
