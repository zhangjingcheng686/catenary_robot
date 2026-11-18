# 开发环境与工程维护

项目与框架介绍见 [项目 README](../../README.md)，控制设计见 [详细控制架构](../architecture/motor_control_architecture.md)。

## 编译工程

确保 CMake、Ninja、ARM GNU 工具链已加入 PATH，在项目根目录执行：

```powershell
cmake --preset Debug
cmake --build --preset Debug --parallel 4
```

输出为 `build/Debug/RMtextH7.elf`。构建目标沿用 `RMtextH7` 名称。

保留的 Mahony 算法依赖 CMSIS-DSP。CMake 单独编译 `arm_atan2_f32.c`，sin/cos 使用已有 DSP 静态库。

## CubeMX 重新生成

使用根目录的 `RMtextH7.ioc`。任务名称、优先级、栈和队列应同时保存到 `.ioc`。

保留 Keep User Code 设置；`freertos.c` 的 USER CODE Includes 中包含 `robot_messages.h`。实际任务实现位于 `source/task/`，生成代码中的 weak 入口由对应实现覆盖。当前配置见 [详细控制架构](../architecture/motor_control_architecture.md)。

## 生成 API 文档

安装 Doxygen 后，从项目根目录执行：

```powershell
doxygen doc/Doxyfile
```

配置使用相对路径，读取项目说明、`doc/`、`source/` 和 `Core/`。各 BSP、模块目录中的 Markdown 会随源码扫描。新的 HTML 和 LaTeX 输出位于 `build/docs/doxygen/`。

`doc/html/` 和 `doc/latex/` 是历史结果，已从生成输入中排除，说明见 [历史模块清理记录](../reviews/cleanup_history.md)。
