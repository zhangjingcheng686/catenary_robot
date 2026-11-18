# UART 接口说明

本目录提供空闲事件接收、中断发送及接收钩子。接口见 [uart_api.h](uart_api.h)，中断和接收重启逻辑见 [uart_bsp.c](uart_bsp.c)。

## 当前串口分工

| 串口 | 用途 |
| --- | --- |
| UART5 | SBUS 遥控器 |
| USART1 | Zigbee 示教器 |
| USART10 | 预留 RDK X5 通信，业务待接入 |

接收初始化当前使用 `HAL_UARTEx_ReceiveToIdle_IT()`，属于中断接收；CubeMX 配置 DMA 不表示这条接口已经使用 DMA。

## 数据与生命周期

`uart_msg_t` 保存外设句柄、缓冲区地址与 `Len`，`uart_rx_t` 保存消息指针及接收统计。消息结构体和缓冲区在接收期间必须持续存在，可以使用文件静态变量。每个串口实例只注册一个接收对象。

`Len` 在初始化时必须为缓冲区实际容量，通常传 `sizeof(buffer)`；接收回调会改为本次收到的长度，底层使用另外保存的容量重启接收。发送时，`Len` 表示待发送字节数。

## 初始化示例

下面展示预留 USART10 的接收方式，实际 HostComm 尚未接入；不要在同一个串口重复初始化。HAL 外设初始化后，在所属任务的初始化阶段调用一次。

```c
#include "uart_api.h"
#include "usart.h"

static uint8_t rx_buffer[100];
static uart_msg_t rx_message;
static uart_rx_t rx_context;

static void receive_hook(uint8_t *data, uint32_t size)
{
    /* 在中断中复制数据或发出通知，后续由任务解析。 */
    (void)data;
    (void)size;
}

static void init_example(void)
{
    rx_message.huart = &huart10;
    rx_message.pBuffer = rx_buffer;
    rx_message.Len = sizeof(rx_buffer);
    rx_context.rx_msg = &rx_message;

    /* 先注册业务钩子，再开启接收，避免启动间隙遗漏业务处理。 */
    if (uart_rx_hook_reg(&rx_context, receive_hook) != UART_OK) {
        Error_Handler();
    }
    if (uart_rx_init(&rx_context) != UART_OK) {
        Error_Handler();
    }
}
```

钩子在 UART 中断路径内执行，应尽快返回，不在其中延时或等待任务。按传入的 `size` 处理字节，接收数据不保证以字符串结束符结尾。

## 发送与返回值

通过 `uart_tx_send_IT()` 发起中断发送。发送缓冲区在传输完成前必须保持有效，不能立即覆盖；接口返回不表示整帧已经发完。

当前封装主要返回参数检查结果，部分 HAL 启动失败或忙状态尚未向上传递，调用时需要结合 HAL 状态与完成回调判断。
