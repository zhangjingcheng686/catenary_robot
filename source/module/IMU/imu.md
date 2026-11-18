# IMU 模块说明

本目录保留 BMI088 温控、陀螺仪校准和 Mahony 姿态解算。主接口见 [IMUtool.h](IMUtool.h)，算法接口见 [MahonyAHRS.h](Mahony/MahonyAHRS.h)，传感器接口见 [BMI088 驱动](../../bsp/BMI088/bmi088_driver.md)。

## 当前状态

下位机不再创建原 IMU 任务，GPIO 回调中的信号量释放已注释。驱动、算法和两个 IMU 信号量仍保留，上位机 IMU 数据计划由上位机直接经串口接收。

## 主要接口

| 接口 | 用途 |
| --- | --- |
| `Mahony_Init(sampleFrequency)` | 设置算法采样频率，单位 Hz，并清空滤波状态 |
| `BMI088_init()` | 初始化传感器，返回 `uint8_t` 错误码 |
| `IMUsys()` | 等待数据就绪信号量，读取数据、温控、校准和解算 |
| `temple()` | 原有温控函数名称，更新加热 PWM |
| `getRoll()`、`getPitch()`、`getYaw()` | 读取算法欧拉角 |

解算结果保存在 `bmi088_data`，包含加速度、角速度、欧拉角和温度。温控目标及校准参数位于 [IMUtool.c](IMUtool.c)，偏航校正宏位于 [config.h](../../config.h)。

## 保留接口的使用条件

原流程先完成 HAL、SPI、GPIO 和相关 PWM 初始化，再初始化传感器与滤波器，由任务周期调用 `IMUsys()`。但该函数内部会等待 `imuBinarySem01Handle`；当前中断回调不释放该信号量，直接新增调用会等待数据就绪而阻塞。

如需重新启用整条处理链，需要同时安排采样节拍、信号量通知、任务和滤波采样频率。仅使用传感器底层读取接口可参照 BMI088 目录的说明。
