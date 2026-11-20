# SBUS 遥控器模块说明

本模块通过 UART5 接收 SBUS 遥控数据。目录保留原来的 `Dbus` 名称，当前协议和数据结构以 [DBusSys.h](DBusSys.h) 为准。接收任务见 [SbusReceive.c](../../task/SbusReceive.c)。

## 接入方式

SBUS 帧长 25 字节，当前校验帧头、帧尾和丢帧、失控标志。串口回调保存完整帧，`DBus_Refresh()` 在任务中解析并更新 `remoter`。当前实现解码 `ch[0]～ch[11]`，虽然数组容量是 16，不应认为其余四路已经接入。

```c
#include "DBusSys.h"
#include "cmsis_os2.h"

/* 说明调用顺序；项目已有 SbusReceive_Task，不另建重复接收任务。 */
void example_receive_task(void *argument)
{
    (void)argument;
    DBus_Init();
    for (;;) {
        DBus_Refresh();
        osDelay(1);
    }
}
```

HAL 已先完成 UART5 初始化。模块接管该串口的接收缓冲区和钩子，不再为同一串口注册其他接收者。

## 读取控制输入

```c
if (remoter.online) {
    int left_screw_input = remoter.rc.ch[0];
    int right_screw_input = remoter.rc.ch[1];
    int mode_switch = remoter.rc.ch[4];
    /* 根据输入生成本周期控制目标。 */
}
```

通道下标从 0 开始。当前拧螺丝逻辑使用 `ch[0]`、`ch[1]`；RobotManager 使用 `ch[4]` 判断 RL、中间档和示教模式。遥控状态按项目约定保留全局变量。

## 在线判断

只有正常且未过期的帧更新通道、在线标志及最近有效时间。带丢帧标志的帧丢弃，失控帧立即置离线，连续 1000 ms 没有有效帧则离线。离线时写入模块既有的默认通道值。

当前应用在离线时请求中间档，并将拧螺丝力矩置零。详细模式行为见 [控制架构](../../../doc/architecture/motor_control_architecture.md)。
