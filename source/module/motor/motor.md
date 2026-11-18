# 电机模块说明

本目录保存电机公共数据结构及具体型号驱动。当前工程使用达妙电机，共九个执行电机。

## 文件分工

| 文件或目录 | 用途 |
| --- | --- |
| [motor_basic.h](motor_basic.h) | 公共 `can_motor_t` 反馈结构 |
| [motor_DM/](motor_DM/) | 达妙电机初始化、控制帧编码及反馈解码 |
| [damiao_motor.md](motor_DM/damiao_motor.md) | 达妙接口、模式和项目接入方式 |

## 公共反馈结构

`can_motor_t` 是驱动内部的反馈容器。

| 成员 | 当前达妙驱动中的含义 |
| --- | --- |
| `motor_angle` | 电机反馈角度，rad |
| `motor_speed` | 电机反馈速度，rad/s |
| `torque_current` | 电机反馈力矩，N·m；公共字段名兼容其他驱动用途 |
| `temp` | 驱动保存的电机温度信息 |
| `can_msg` | 接收缓存、CAN ID、端口及接收时间等信息 |

达妙驱动的 `DM_motor_t` 进一步组合这个反馈容器、发送配置、状态码及型号量程参数。具体结构见 [motor_DM.h](motor_DM/motor_DM.h)。

## 当前工程如何使用

1. 系统启动时，由 MotorControl 任务初始化九个电机对象。
2. CAN 中断按端口、反馈 ID 更新对应电机缓存。
3. MotorFeedback 任务调用驱动解码接口，打包九电机反馈快照。
4. 示教、拧螺丝及后续 RL 控制函数更新九电机目标数组。
5. MotorControl 统一调用驱动发送接口。

应用层的消息定义位于 [robot_messages.h](../../task/robot_messages.h)。当前反馈队列只发布位置、力矩、时间和状态等信息，未发布驱动内部的速度字段。

## 增加或调整电机

电机对象、CAN 端口和 ID、初始目标配置位于 [MotorControl.c](../../task/MotorControl.c)，数量及索引位于 [MotorControl.h](../../task/MotorControl.h)。调整时需要同时核对反馈数组顺序、示教七路映射和控制目标数组。

新增驱动应在本目录下使用独立子目录，提供头文件、实现及按功能命名的 Markdown 说明。其发送入口接入唯一电机执行任务，反馈解析结果由反馈任务汇总。当前三种控制模式与驱动用法见 [达妙电机说明](motor_DM/damiao_motor.md)。
