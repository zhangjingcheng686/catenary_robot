# Zigbee 示教器模块说明

本模块通过 USART1 接收示教器的七路输入，处理帧校验、分段接收和完整目标复制。接口见 [zigbee.h](zigbee.h)，接收任务见 [TeachReceive.c](../../task/TeachReceive.c)。

## 文件与接口

| 文件或接口 | 用途 |
| --- | --- |
| [zigbee.h](zigbee.h) | 帧格式、七路输入数量、超时常量与公开接口 |
| [zigbee.c](zigbee.c) | 串口接收、连续字节组帧、校验及完整目标复制 |
| `Zigbee_Rx_Init()` | 注册 USART 接收对象和业务钩子，启动接收 |
| `Zigbee_TakeTarget()` | 由接收任务取得尚未取走的最新完整目标 |

## 当前控制范围

七路输入顺序为左臂电机 1、左臂电机 2、左夹爪、右臂电机 1、右臂电机 2、右臂电机 3、右夹爪。普通关节输入为角度，单位度；夹爪输入也按度编码，但由示教控制函数映射为目标力矩。两个拧螺丝电机不包含在示教帧中。

## 帧格式

`Zigbee_Frame_t` 按 1 字节对齐，共 33 字节。

| 字段 | 类型 | 含义 |
| --- | --- | --- |
| `header` | `uint16_t` | `0x55AA` |
| `angles[7]` | 七个 `float` | 七路输入 |
| `checksum` | `uint8_t` | 前 30 字节累加结果，取低 8 位 |
| `tail` | `uint16_t` | `0xA55A` |

发送端的整数、浮点编码及字节序需与当前 H7 结构体表示一致。模块检查帧头、帧尾、校验和与各输入是否为有限值；无效时继续逐字节寻找下一帧，并保留不完整数据等待后续接收。

## 调用方式

USART1 完成 HAL 初始化后，接收任务调用一次：

```c
Zigbee_Rx_Init(&huart1);
```

随后由该任务循环取得最新完整目标：

```c
TeachTarget_t target;

if (Zigbee_TakeTarget(target.input_deg, &target.receive_time_ms)) {
    xQueueOverwrite((QueueHandle_t)TeachTargetQueueHandle, &target);
}
```

示例用到的消息类型在 [robot_messages.h](../../task/robot_messages.h)，队列由 CubeMX 任务初始化创建。项目已有 TeachReceive 任务，接口不需要重复初始化。

`Zigbee_TakeTarget()` 适用于单个接收任务读取。它通过序号核对复制过程，避免串口中断更新到一半时返回混合目标；没有新帧时返回 0。一次取得最新帧后，不会重复返回同一帧。

示教控制函数从目标队列读取数据并检查 500 ms 超时。模块内保留的 `Remote_TargetAngles` 等全局量用于兼容，当前目标传递以队列为准。
