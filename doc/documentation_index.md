# 项目文档索引

[根目录 README](../README.md) 介绍机器人用途、硬件组成、框架、任务与数据流。BSP 和模块文档与源码同目录，按功能命名。

## 项目级文档

| 文档 | 内容 |
| --- | --- |
| [统一电机控制方案](architecture/motor_control_architecture.md) | 模式切换、控制周期、任务与队列的详细设计 |
| [代码规范](guides/code_style.md) | 代码组织、接口、命名与注释约定 |
| [开发环境与工程维护](guides/development.md) | 编译、CubeMX 重新生成与 Doxygen 使用方法 |
| [下位机代码问题记录](reviews/code_review_issues.md) | 已知问题、修复状态与验证记录 |
| [历史模块清理记录](reviews/cleanup_history.md) | 原框架裁剪、备份与历史生成文档说明 |

## 源码旁的使用说明

| 层次 | 文档 | 内容 |
| --- | --- | --- |
| BSP | [CAN](../source/bsp/can/can_api.md) | 接收注册、发送接口与滤波 |
| BSP | [UART](../source/bsp/uart/uart_api.md) | 接收缓冲区、钩子和异步发送 |
| BSP | [DWT](../source/bsp/dwt/dwt_bsp.md) | 时间测量与延时 |
| BSP | [BMI088](../source/bsp/BMI088/bmi088_driver.md) | 保留的传感器驱动 |
| BSP | [RTT 日志](../source/bsp/rtt/rtt.md) | 初始化、日志等级、浮点输出及缓冲区行为 |
| 模块 | [SBUS](../source/module/Dbus/sbus.md) | 遥控器通道与在线状态 |
| 模块 | [Zigbee](../source/module/Zigbee/zigbee.md) | 示教器七路目标接收 |
| 模块 | [电机模块总览](../source/module/motor/motor.md) | 公共反馈结构、驱动组织与接入位置 |
| 模块 | [达妙电机](../source/module/motor/motor_DM/damiao_motor.md) | 初始化、控制帧编码与反馈解码 |
| 模块 | [IMU](../source/module/IMU/imu.md) | 保留的温控、校准与姿态算法 |

## 文档约定

- 根目录只保留一份 `README.md`，作为项目和框架入口。
- BSP、模块说明放在对应源码目录，以功能命名，如 `can_api.md`、`sbus.md`、`damiao_motor.md`。
- `doc/architecture/` 保存架构设计，`doc/guides/` 保存开发规范与维护方法，`doc/reviews/` 保存问题及历史记录。
- Markdown 文件名使用英文小写下划线，标题与正文以中文为主；`README.md` 和 `Doxyfile` 保留工具约定名称。
- 说明文档给出用途、文件、依赖、接口、用法及当前接入状态，使用相对链接关联源码。

在 VS Code 中按 `Ctrl + Shift + V` 查看排版；先按 `Ctrl + K`，再按 `V` 可打开侧边预览。
