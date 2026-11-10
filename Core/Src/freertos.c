/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : freertos.c
 * Description        : Code for freertos applications
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

// #include "DBusSys.h"
// #include "dma.h"
// #include "usart.h"
// #include "PIDtool.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
typedef StaticTask_t osStaticThreadDef_t;
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for IMU_TempCtrl */
osThreadId_t IMU_TempCtrlHandle;
uint32_t IMU_TempCtrlBuffer[ 256 ];
osStaticThreadDef_t IMU_TempCtrlControlBlock;
const osThreadAttr_t IMU_TempCtrl_attributes = {
  .name = "IMU_TempCtrl",
  .cb_mem = &IMU_TempCtrlControlBlock,
  .cb_size = sizeof(IMU_TempCtrlControlBlock),
  .stack_mem = &IMU_TempCtrlBuffer[0],
  .stack_size = sizeof(IMU_TempCtrlBuffer),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for Remoter */
osThreadId_t RemoterHandle;
uint32_t RemoterBuffer[ 256 ];
osStaticThreadDef_t RemoterControlBlock;
const osThreadAttr_t Remoter_attributes = {
  .name = "Remoter",
  .cb_mem = &RemoterControlBlock,
  .cb_size = sizeof(RemoterControlBlock),
  .stack_mem = &RemoterBuffer[0],
  .stack_size = sizeof(RemoterBuffer),
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for ArmControl */
osThreadId_t ArmControlHandle;
uint32_t ArmControlBuffer[ 2048 ];
osStaticThreadDef_t ArmControlControlBlock;
const osThreadAttr_t ArmControl_attributes = {
  .name = "ArmControl",
  .cb_mem = &ArmControlControlBlock,
  .cb_size = sizeof(ArmControlControlBlock),
  .stack_mem = &ArmControlBuffer[0],
  .stack_size = sizeof(ArmControlBuffer),
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for ClawControl */
osThreadId_t ClawControlHandle;
uint32_t ClawControlBuffer[ 256 ];
osStaticThreadDef_t ClawControlControlBlock;
const osThreadAttr_t ClawControl_attributes = {
  .name = "ClawControl",
  .cb_mem = &ClawControlControlBlock,
  .cb_size = sizeof(ClawControlControlBlock),
  .stack_mem = &ClawControlBuffer[0],
  .stack_size = sizeof(ClawControlBuffer),
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for Switch */
osThreadId_t SwitchHandle;
uint32_t SwitchBuffer[ 2048 ];
osStaticThreadDef_t SwitchControlBlock;
const osThreadAttr_t Switch_attributes = {
  .name = "Switch",
  .cb_mem = &SwitchControlBlock,
  .cb_size = sizeof(SwitchControlBlock),
  .stack_mem = &SwitchBuffer[0],
  .stack_size = sizeof(SwitchBuffer),
  .priority = (osPriority_t) osPriorityBelowNormal,
};
/* Definitions for Demonstrator */
osThreadId_t DemonstratorHandle;
uint32_t DemonstratorBuffer[ 512 ];
osStaticThreadDef_t DemonstratorControlBlock;
const osThreadAttr_t Demonstrator_attributes = {
  .name = "Demonstrator",
  .cb_mem = &DemonstratorControlBlock,
  .cb_size = sizeof(DemonstratorControlBlock),
  .stack_mem = &DemonstratorBuffer[0],
  .stack_size = sizeof(DemonstratorBuffer),
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for Zigbee */
osThreadId_t ZigbeeHandle;
uint32_t ZigbeeBuffer[ 256 ];
osStaticThreadDef_t ZigbeeControlBlock;
const osThreadAttr_t Zigbee_attributes = {
  .name = "Zigbee",
  .cb_mem = &ZigbeeControlBlock,
  .cb_size = sizeof(ZigbeeControlBlock),
  .stack_mem = &ZigbeeBuffer[0],
  .stack_size = sizeof(ZigbeeBuffer),
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for imuBinarySem01 */
osSemaphoreId_t imuBinarySem01Handle;
const osSemaphoreAttr_t imuBinarySem01_attributes = {
  .name = "imuBinarySem01"
};
/* Definitions for controlBinaryIMU */
osSemaphoreId_t controlBinaryIMUHandle;
const osSemaphoreAttr_t controlBinaryIMU_attributes = {
  .name = "controlBinaryIMU"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void IMU_TempCtrlTask(void *argument);
void Remoter_Task(void *argument);
void ArmControl_Task(void *argument);
void ClawControl_Task(void *argument);
void Switch_Task(void *argument);
void Demonstrator_Task(void *argument);
void Zigbee_Task(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* creation of imuBinarySem01 */
  imuBinarySem01Handle = osSemaphoreNew(1, 0, &imuBinarySem01_attributes);

  /* creation of controlBinaryIMU */
  controlBinaryIMUHandle = osSemaphoreNew(1, 0, &controlBinaryIMU_attributes);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of IMU_TempCtrl */
  IMU_TempCtrlHandle = osThreadNew(IMU_TempCtrlTask, NULL, &IMU_TempCtrl_attributes);

  /* creation of Remoter */
  RemoterHandle = osThreadNew(Remoter_Task, NULL, &Remoter_attributes);

  /* creation of ArmControl */
  ArmControlHandle = osThreadNew(ArmControl_Task, NULL, &ArmControl_attributes);

  /* creation of ClawControl */
  ClawControlHandle = osThreadNew(ClawControl_Task, NULL, &ClawControl_attributes);

  /* creation of Switch */
  SwitchHandle = osThreadNew(Switch_Task, NULL, &Switch_attributes);

  /* creation of Demonstrator */
  DemonstratorHandle = osThreadNew(Demonstrator_Task, NULL, &Demonstrator_attributes);

  /* creation of Zigbee */
  ZigbeeHandle = osThreadNew(Zigbee_Task, NULL, &Zigbee_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
 * @brief  Function implementing the defaultTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  UNUSED(argument);
  /* Infinite loop */
  for (;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_IMU_TempCtrlTask */
/**
* @brief Function implementing the IMU_TempCtrl thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_IMU_TempCtrlTask */
__weak void IMU_TempCtrlTask(void *argument)
{
  /* USER CODE BEGIN IMU_TempCtrlTask */
  UNUSED(argument);
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END IMU_TempCtrlTask */
}

/* USER CODE BEGIN Header_Remoter_Task */
/**
* @brief Function implementing the Remoter thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Remoter_Task */
__weak void Remoter_Task(void *argument)
{
  /* USER CODE BEGIN Remoter_Task */
  UNUSED(argument);
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END Remoter_Task */
}

/* USER CODE BEGIN Header_ArmControl_Task */
/**
* @brief Function implementing the ArmControl thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_ArmControl_Task */
__weak void ArmControl_Task(void *argument)
{
  /* USER CODE BEGIN ArmControl_Task */
  UNUSED(argument);
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END ArmControl_Task */
}

/* USER CODE BEGIN Header_ClawControl_Task */
/**
* @brief Function implementing the ClawControl thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_ClawControl_Task */
__weak void ClawControl_Task(void *argument)
{
  /* USER CODE BEGIN ClawControl_Task */
  UNUSED(argument);
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END ClawControl_Task */
}

/* USER CODE BEGIN Header_Switch_Task */
/**
* @brief Function implementing the Switch thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Switch_Task */
__weak void Switch_Task(void *argument)
{
  /* USER CODE BEGIN Switch_Task */
  UNUSED(argument);
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END Switch_Task */
}

/* USER CODE BEGIN Header_Demonstrator_Task */
/**
* @brief Function implementing the Demonstrator thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Demonstrator_Task */
__weak void Demonstrator_Task(void *argument)
{
  /* USER CODE BEGIN Demonstrator_Task */
  UNUSED(argument);
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END Demonstrator_Task */
}

/* USER CODE BEGIN Header_Zigbee_Task */
/**
* @brief Function implementing the Zigbee thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Zigbee_Task */
__weak void Zigbee_Task(void *argument)
{
  /* USER CODE BEGIN Zigbee_Task */
  UNUSED(argument);
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END Zigbee_Task */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

