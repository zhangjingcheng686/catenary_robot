# CAN 接口说明

本目录封装 CAN 初始化、硬件滤波、消息注册和收发。接口见 [can_api.h](can_api.h)，数据类型见 [can_struct.h](can_struct.h)，中断实现见 [can_bsp.c](can_bsp.c)。

## 当前工程中的位置

CAN1 连接左臂四个电机，CAN2 连接右臂五个电机，CAN3 保留配置。H7 使用 FDCAN 外设发送经典 CAN 帧，数据区最多 8 字节。

接收路径为：硬件 FIFO → 接收中断 → 按 CAN ID 查找注册项 → 更新电机缓存。反馈解析由 MotorFeedback 任务调用达妙驱动完成。运行期间，电机控制发送统一由 MotorControl 任务发起。

## 初始化与滤波

HAL 外设初始化后，在系统启动阶段调用一次 `can_sys_init()`，建立三个接收列表，配置滤波并启动外设。当前调用位于项目的 `main.c`。

每个实例有一个标准滤波器，编号均为 0，允许标准数据帧 ID `0x000～0x7FF` 进入 FIFO0。扩展帧和远程帧被拒绝。

## 消息结构

| 成员 | 用途 |
| --- | --- |
| `port` | `CAN1_PORT`、`CAN2_PORT` 或 `CAN3_PORT` |
| `id` | CAN 报文 ID |
| `data[8]` | 报文字节 |
| `len` | 数据长度；发送时按驱动要求使用 DLC 常量 |
| `cnt` | 当前接收回调递增的计数，8 位回绕 |
| `last_receive_time_ms` | 最近接收时间，HAL 毫秒计数 |

## 接收注册

将具有持续生命周期的 `can_msg_t` 交给 `can_msg_add_item()`，回调按端口与 ID 更新该对象。达妙驱动的型号初始化函数已经完成注册，应用不要重复注册同一端口和 ID。

```c
static can_msg_t example_rx = {
    .port = CAN3_PORT,
    .id = 0x123
};

/* 示例：在初始化阶段注册一个独立接收对象。 */
can_status_t result = can_msg_add_item(&example_rx);
```

初始化前应先完成 `can_sys_init()`。注册对象不能是函数返回后失效的局部变量。

## 主要接口

| 接口 | 作用 |
| --- | --- |
| `can_sys_init()` | 初始化接收列表和底层外设 |
| `can_msg_add_item()` | 按端口和 ID 注册接收对象 |
| `can_msg_del_item()` | 移除注册项 |
| `can_msg_send_classical()` | 发送标准经典 CAN 消息 |
| `can_msg_send_extended()` | 扩展 ID 发送封装，当前电机链路不使用 |
| `can_msg_send_fd()` | FD 发送封装，当前电机链路不使用 |

接收过滤策略与底层硬件配置需要配套。发送封装返回值目前尚未完整传递 HAL 入队失败状态，其他已知限制见 [问题记录](../../../doc/reviews/code_review_issues.md)。
