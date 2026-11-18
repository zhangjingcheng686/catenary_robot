# 达妙电机模块说明

本模块负责达妙电机参数初始化、控制帧编码与反馈解码。接口见 [motor_DM.h](motor_DM.h)。当前工程由 [MotorControl.c](../../../task/MotorControl.c) 初始化并发送电机指令，由 [MotorFeedback.c](../../../task/MotorFeedback.c) 汇总反馈。

## 接入顺序

1. 系统启动时先初始化 CAN 接收列表和外设。
2. 分配并清零具有持续生命周期的 `DM_motor_t` 对象。
3. 调用对应型号的 `Motor_DM6248_sign()`、`Motor_DM4340_sign()` 或 `Motor_DM4310_sign()`，传入控制 CAN ID、反馈 CAN ID 和端口。型号初始化会注册反馈接收项。
4. 通过 `Motor_DM_Enable()` 发送使能指令，再按已配置模式发送控制目标。

以上过程当前已在 MotorControl 中完成，不应在其他任务重复创建或注册相同电机。控制 ID 与反馈 ID 由电机配置决定，两者不一定相同。

## 模式与接口

| 模式 | 宏 | 接口 | 参数 |
| --- | --- | --- | --- |
| MIT | `MIT_MODE` | `MIT_CtrlMotorDM()` | 位置 rad、速度 rad/s、Kp、Kd、前馈力矩 N·m |
| 位置速度 | `POS_MODE` | `PosSpeed_CtrlMotorDM()` | 位置 rad、速度参数 rad/s |
| 速度 | `SPD_MODE` | `Speed_CtrlMotorDM()` | 速度 rad/s |

软件通过模式相关 CAN ID 发送报文，电机端工作模式应与控制方式配套。当前五个普通关节为位置速度模式、两个夹爪为 MIT 模式、两个拧螺丝电机为速度模式；统一 MIT 改造尚未进行。

## 在本工程中生成动作

示教、拧螺丝和 RL 控制函数接收 `MotorCommand_t` 数组，更新目标后返回。以普通关节为例：

```c
/* 在周期控制函数内更新左臂电机 1，实际角度还须经过关节限位检查。 */
commands[0].mode = MOTOR_COMMAND_POSITION_SPEED;
commands[0].target.position = 30.0f * (3.1415926f / 180.0f);
commands[0].target.velocity = 3.0f;
commands[0].enabled = 1;
```

`SendMotorCommands()` 在同一控制任务中读取目标数组并调用驱动接口。`enabled` 是软件发送标志，不是电机硬件使能状态。

## 反馈与状态

`Motor_DM_Refresh()` 将 CAN 缓存字节解码为 `motor_msg.motor_angle`、`motor_msg.motor_speed`、`motor_msg.torque_current` 及状态码。当前由 MotorFeedback 任务周期刷新，其他控制逻辑使用反馈队列中的副本。

上传和控制所用的九电机反馈消息只包含位置、力矩、时间与有效状态等字段，不包含速度；驱动内部仍保留解码速度。

使能、失能、清错和保存零点接口分别为 `Motor_DM_Enable()`、`Motor_DM_Disable()`、`Motor_DM_Clear_Error()`、`Motor_DM_Save_Zero()`，实际调用也应经过唯一电机执行任务。电机量程、反馈完整性及通信失败处理见 [问题记录](../../../../doc/reviews/code_review_issues.md)。
