# RTT 日志模块说明

本目录使用 SEGGER RTT 输出调试日志。程序将日志写入 MCU 内存缓冲区，由支持 RTT 的调试工具通过调试连接读取，不占用业务串口。

## 文件分工

| 文件 | 用途 |
| --- | --- |
| [log.h](log.h) | 应用使用的日志等级、颜色、初始化和输出宏 |
| [SEGGER_RTT.h](SEGGER_RTT.h) | RTT 底层接口 |
| [SEGGER_RTT.c](SEGGER_RTT.c) | 缓冲区和通道读写实现 |
| [SEGGER_RTT_printf.c](SEGGER_RTT_printf.c) | 精简格式化输出 |
| [SEGGER_RTT_Conf.h](Config/SEGGER_RTT_Conf.h) | 缓冲区、默认模式及锁配置 |

应用以 `log.h` 中的宏作为入口。

## 初始化

系统启动时调用一次 `LOG_INIT()`。当前工程已在 [main.c](../../../Core/Src/main.c) 中调用，业务任务直接输出日志即可。

```c
#include "log.h"
#include "dwt_bsp.h"

/* 启动阶段调用一次，项目已有此步骤，不在任务里重复初始化。 */
LOG_INIT();
```

当前宏依次初始化 RTT、调用 `DWT_Init(480)`、清屏并打印启动信息。这里的 480 是现有代码的 CPU 频率参数，移植或调整系统时钟时需要配套核对。

## 记录日志

```c
#include "log.h"

LOG_INFO("Teach mode entered");
LOG_WARN("Motor %d feedback timeout", 3);
LOG_ERROR("CAN configuration failed");
LOG("Target updated, motor=%d", 1);
```

当前日志输出到上行通道 0，宏自动添加等级、颜色和换行，不必在字符串末尾再次添加换行。

| 宏或配置 | 用途 |
| --- | --- |
| `LOG()` | DEBUG 等级输出，当前没有单独的 `LOG_DEBUG()` 宏 |
| `LOG_INFO()` | 一般信息 |
| `LOG_WARN()` | 警告信息 |
| `LOG_ERROR()` | 错误信息 |
| `LOG_CLEAR()` | 发送清屏控制字符 |
| `LOG_LEVEL` | 编译时日志等级，当前为 `LOG_LEVEL_DEBUG` |

## 浮点数输出

当前精简格式化实现支持整数、十六进制和字符串等格式，未提供 `%f` 分支。可以先转换为字符串，项目的夹爪日志也采用这种方式。

```c
#include "uint_float_convert.h"

char torque_text[32];
float2string(2.5f, torque_text);
LOG_INFO("Torque=%s", torque_text);
```

## 查看日志与缓冲区行为

将支持 RTT 的调试器连接到开发板，运行固件，再用相应 RTT 终端连接目标 MCU 并查看通道 0。若工具需要指定 MCU 或 RTT 控制块地址，应使用本工程目标和当前固件的信息。

当前配置上行缓冲区为 1024 字节，下行缓冲区为 16 字节，默认使用 `SEGGER_RTT_MODE_NO_BLOCK_SKIP`。缓冲区空间不足时数据可能被跳过，日志不能作为控制指令或可靠消息队列使用。

周期任务应控制打印频率和字符串长度，避免格式化和日志输出明显增加执行时间；优先在状态变化或需要定位问题时记录。
