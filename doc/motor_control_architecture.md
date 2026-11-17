# 下位机统一电机控制方案

更新日期为 2026-11-16。

## 任务与队列

创建六个业务任务，另保留 defaultTask。FreeRTOS 内核自行创建的空闲任务及已启用的定时器任务另外计算。

| 任务 | 优先级 | 栈 Words | 职责 |
| --- | --- | --- | --- |
| RobotManager | Normal | 2048 | 每 20 ms 读取遥控器状态，提交模式请求 |
| MotorFeedback | Normal | 512 | 初始化完成后，每 20 ms 刷新九电机反馈并发布 |
| MotorControl | BelowNormal | 2048 | 初始化电机，每 20 ms 切换模式、计算目标并统一发送 CAN |
| SbusReceive | Low | 256 | 保留现有 SBUS 接收与遥控器全局变量 |
| TeachReceive | Low | 256 | 接收示教器七路目标，发布最新一帧 |
| HostComm | Low | 1024 | 保留空任务，上位机协议后续接入 |
| defaultTask | Low | 128 | 保留空循环与 osDelay(1) |

三个队列长度均为 1，消息通过值复制传递。

| 队列 | 生产者 | 消费者 | 读写方式 |
| --- | --- | --- | --- |
| TeachTargetQueue | TeachReceive | MotorControl 内的 TeachControl_Update | 覆盖写，非阻塞取出，本地保留并检查 500 ms 超时 |
| MotorFeedbackQueue | MotorFeedback | MotorControl、后续 HostComm | 覆盖写，非移除 Peek，支持多个读者 |
| RLCommandQueue | 后续 HostComm | 后续 MotorControl 内的 RLControl_Update | 当前仅保留创建，业务收发暂未接入 |

遥控器仍使用原有全局变量。模式请求是 RobotManager 单写、MotorControl 单读的对齐枚举变量，声明为 volatile，不增加互斥锁或应用临界区。实际模式及控制目标由 MotorControl 单独维护。

## 控制周期

1. MotorControl 启动后等待原有的 1000 ms，初始化九个电机并发送启动指令。电机类型、ID、模式、初始角度及初始化间隔沿用原实现。
2. 初始化完成后设置 MotorFeedback 的就绪线程标志。反馈任务随后每 20 ms 更新反馈队列。
3. 每个控制周期开始时读取一次模式请求，再读取一次反馈快照，并重新检查反馈时间。反馈有效期仍为 500 ms。
4. 模式改变时清空示教本地目标和原 RL 动作状态。进入示教或 RL 时，普通关节优先用最新有效位置建立保持目标，夹爪清零力矩及增益，拧螺丝电机清零速度。
5. 没有有效位置的普通关节暂不发送新的位置指令，有效反馈到达后建立保持目标。此时电机可能仍保持其内部上一条指令，此行为不等同于失能或急停。
6. 执行当前模式的控制函数，将结果写入九电机目标数组，再统一发送。通过 vTaskDelayUntil 维持暂定 20 ms 周期，实际周期与最坏耗时需上板测量。

模式请求在一个周期内发生变化时，本周期继续完成已选模式，下一周期再切换。正常调度下，采纳请求最多等待一个控制周期；从拨杆变化算起还包含 SBUS 接收和 RobotManager 的轮询时间。

## 模式与控制权

| 拨杆 ch[4] | 模式 | 本周期执行 |
| --- | --- | --- |
| 小于 980 | RL | RLControl_Update，预留九电机控制入口 |
| 大于 1020 | 示教 | TeachControl_Update 控制七个电机，ScrewControl_Update 控制另两个 |
| 980 至 1020 | 中间档 | 切入时建立原初始姿态，ScrewControl_Update 继续允许遥控拧螺丝 |

遥控器离线时管理任务请求中间档，拧螺丝函数输出零速度。这沿用原来的中间档初始姿态行为，不是故障急停策略。

示教目标超时或包含非有限值时，两夹爪输出零力矩，普通关节保留此前合法位置目标。重新进入示教后先清空模块内的旧目标，再读取队列中最新且未超过 500 ms 的目标。无需等待进入模式之后的新帧。

拧螺丝继续使用速度模式，速度为正负 2 rad/s；力矩绝对值连续五个控制周期达到 2 N·m 后锁定零速度，摇杆回中解锁。无有效力矩反馈时输出零速度。进入 RL 后不调用遥控拧螺丝函数。

RL 当前三个动作函数仍为空，尚不读取 RLCommandQueue。进入该模式后普通关节保持入口建立的位置，工具输出为零。MIT 统一转换和上位机协议不在本次改动中。

## 文件与重新生成

- `source/task/MotorControl.c/.h` 为唯一电机执行任务，维护电机对象、模式及九电机控制目标。
- `source/task/TeachControl.c/.h`、`ScrewControl.c/.h`、`RLControl.c/.h` 保留各自算法，以无任务循环、无延时的周期函数供 MotorControl 调用。
- `source/task/RobotManager.c/.h` 只提交模式，不直接发送 CAN，也不挂起或恢复控制任务。
- `RMtextH7.ioc` 的 FreeRTOS 任务配置已同步为上述七行；原 RLControl 行改为 MotorControl，删除独立 TeachControl、ScrewControl 行。三个队列和两个 IMU 信号量保持现有配置。
- `Core/Src/freertos.c` 已同步当前任务创建、静态缓冲区、入口声明及 weak 占位函数。具体实现位于 source/task，以强符号覆盖生成的 weak 入口。

重新生成时使用修改后的 `RMtextH7.ioc`。如果 CubeMX 已经打开旧配置，先重新加载磁盘上的文件，避免旧窗口保存覆盖本次修改。保留 ProjectManager 的 Keep User Code 设置，`freertos.c` 的 USER CODE Includes 中继续包含 `robot_messages.h`。

CMake 会自动收集 source 下的 C 文件及头文件目录，新增 MotorControl.c 会自动参与构建。用户通过 CubeMX 重新生成后，已核对任务、队列、用户代码区和实际链接入口均保留，并通过 Debug 清理后完整构建。

## 验证

Debug 构建和链接通过，链接器仍有原有 LOAD 段 RWX 权限警告。生产控制函数体的六组编译期断言检查及原 SBUS 六组检查通过。检查项详见 code_review_issues.md 的验证记录。

实机仍需验证模式切换、示教超时、两侧拧螺丝力矩停转、CAN 总线负载、任务栈余量及控制周期。原问题记录中的 CAN 滤波器、接收积压、发送失败上报和原始反馈一致性尚未处理。
