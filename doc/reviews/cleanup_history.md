# 历史模块清理记录

## 2026-11-10 模块裁剪

按当时任务调用关系，清理了未使用的 DJI 电机、轮式底盘运动学、裁判系统功率控制、通用 PID/工具箱、低通滤波、EKF、外置 IMU、TOF、CAN 编码器、软件 CRC 和蜂鸣器封装，并移除了未使用的视觉、雷达和弹道配置宏。

清理前完整 `source/`、CMakeLists 和两份 README 的备份位置为 `cleanup-backup/before-unused-cleanup-20261110-161827.zip`。该路径用于本机历史恢复，备份是否存在应以本地文件为准。

以上是当时清理范围。当前任务和数据流见 [控制架构](../architecture/motor_control_architecture.md)，修复情况见 [问题记录](code_review_issues.md)。

## 历史自动生成文档

`doc/html/` 和 `doc/latex/` 是原框架生成的历史资料，包含已移除模块和旧源码路径。保留生成器文件名及内部链接；当前接口以源码旁的功能文档和头文件为准。
