/**
 * @file Remoter.c
 * @brief 遥控器接收与系统数据流解析任务 (RC Receiver & Data Parsing)
 * @details
 * 1. 初始化 DWT（Data Watchpoint and Trace）用于精确的微秒级延时。
 * 2. 初始化 DBus 通信协议，处理遥控器原始数据帧。
 * 3. 在循环中持续刷新并解析 RC 数据，供下游控制任务读取。
 * @author [白衣思博]
 * @date 2026-03
 */

#include "main.h"
#include "cmsis_os2.h"
#include "DBusSys.h"
#include "dwt_bsp.h"

void SbusReceive_Task(void const *argument)
{
    UNUSED(argument);
    osDelay(100);
    DWT_Init(480); 
    DBus_Init(); 

    while (1)
    {     
        DBus_Refresh();

        osDelay(1);  
    }
}
