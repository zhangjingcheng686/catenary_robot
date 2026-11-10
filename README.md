# 接触网机器人控制工程

基于 STM32H723、STM32 HAL 和 FreeRTOS，当前控制左臂 4 个、右臂 5 个达妙电机：5 个普通关节、2 个夹爪、2 个螺丝电机。

## 当前功能

- UART5 接收 SBUS 遥控数据，选择运行模式并控制螺丝电机。
- USART1 接收 Zigbee 示教数据，驱动普通关节跟随，并将夹爪输入映射为 MIT 纯力矩指令。
- CAN1 控制左臂，CAN2 控制右臂；CAN3 保留外设配置。
- BMI088 采集、40℃ 温控、陀螺仪零偏校准和 Mahony 姿态解算。
- RTT 日志输出。
- 自动动作任务保留状态机接口，Action1/2/3 当前为空，尚未实现自动步态。

## 代码结构

| 目录 | 内容 |
| --- | --- |
| `Core/` | CubeMX 外设配置、中断入口、RTOS 任务创建 |
| `source/task/` | 模式切换、遥控、示教、螺丝电机和 IMU 任务 |
| `source/module/Dbus/` | SBUS 遥控解析，沿用原 DBus 文件名 |
| `source/module/Zigbee/` | 7 路示教数据帧解析 |
| `source/module/motor/` | 达妙电机协议与公共反馈结构 |
| `source/module/IMU/` | IMU 温控、校准和 Mahony 算法 |
| `source/bsp/` | CAN、UART、BMI088、GPIO 中断、DWT、RTT |
| `source/library/` | 浮点编码与字符串转换 |
| `Drivers/`、`Middlewares/` | 芯片库、DSP 和 FreeRTOS |

## 编译

确保 CMake、Ninja、ARM GNU 工具链已加入 PATH：

```powershell
cmake --preset Debug
cmake --build --preset Debug --parallel 4
```

输出为 `build/Debug/RMtextH7.elf`。工程名沿用原框架名称。

Mahony 仍依赖 CMSIS-DSP 的三角函数；CMake 仅单独编译 `arm_atan2_f32.c`，sin/cos 使用现有 DSP 静态库。

## 未使用模块清理

2026-11-10 按当前任务调用关系清理了未使用的 DJI 电机、轮式底盘运动学、裁判系统功率控制、通用 PID/工具箱、低通滤波、EKF、外置 IMU、TOF、CAN 编码器、软件 CRC 和蜂鸣器封装，并移除了未使用的视觉、雷达和弹道配置宏。

清理前的完整 `source/`、CMakeLists 和两份 README 保存在 `cleanup-backup/before-unused-cleanup-20261110-161827.zip`。后续需要相应传感器或算法时可从备份恢复。

保留现有应用任务、自动动作接口、CubeMX 外设配置、芯片驱动和 FreeRTOS。本次只清理未使用代码，没有调整运动、通信或保护行为。

`doc/html/` 和 `doc/latex/` 是清理前生成的历史文档，包含已移除模块，不代表当前实现；当前功能和源码以本说明及 `source/` 为准。

代码规范见 [code_style.md](code_style.md)。
