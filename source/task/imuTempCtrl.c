/**
 * @file imuTempCtrl.c
 * @brief IMU 姿态解算与传感器温度控制任务 (IMU AHRS & Temperature Control)
 * @details
 * 1. 初始化并读取 BMI088 六轴传感器数据。
 * 2. 调用 Mahony 滤波算法，将原始加速度计与陀螺仪数据解算为欧拉角（Pitch, Roll, Yaw）。
 * 3. 开启 TIM3 PWM 输出，用于维持 IMU 传感器的恒定工作温度，消除零偏温漂。
 * @author [白衣思博]
 * @date 2026-03
 */

#include "usart.h"
#include "tim.h"
#include "IMUtool.h"
#include "cmsis_os2.h"
#include "BMI088driver.h"
#include "log.h"
#include <stdio.h>
#include "uint_float_convert.h"

extern float temp;

void IMU_TempCtrlTask(void const *argument)
{
  UNUSED(argument);
  osDelay(500);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);
  Mahony_Init(1000);
  BMI088_init();
  char temp_str[10] = {0};
  char pitch_str[10] = {0};
  char roll_str[10] = {0};
  char yaw_str[10] = {0};
  while (1)
  {
    IMUsys();
    float2string(temp, temp_str);
    float2string(bmi088_data.angles.pitch, pitch_str);
    float2string(bmi088_data.angles.roll, roll_str);
    float2string(bmi088_data.angles.yaw, yaw_str);
    // LOG_INFO("IMU temp:%s, pitch:%s, roll:%s, yaw:%s", temp_str, pitch_str, roll_str, yaw_str);
    // LOG_INFO("IMU temp:%s", float2string(temp, temp_str));
    osDelay(1);
  }
}

