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
#include "robot_messages.h"
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
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for SbusReceive */
osThreadId_t SbusReceiveHandle;
uint32_t SbusReceiveBuffer[ 256 ];
osStaticThreadDef_t SbusReceiveControlBlock;
const osThreadAttr_t SbusReceive_attributes = {
  .name = "SbusReceive",
  .cb_mem = &SbusReceiveControlBlock,
  .cb_size = sizeof(SbusReceiveControlBlock),
  .stack_mem = &SbusReceiveBuffer[0],
  .stack_size = sizeof(SbusReceiveBuffer),
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for MotorControl */
osThreadId_t MotorControlHandle;
uint32_t MotorControlBuffer[ 2048 ];
osStaticThreadDef_t MotorControlControlBlock;
const osThreadAttr_t MotorControl_attributes = {
  .name = "MotorControl",
  .cb_mem = &MotorControlControlBlock,
  .cb_size = sizeof(MotorControlControlBlock),
  .stack_mem = &MotorControlBuffer[0],
  .stack_size = sizeof(MotorControlBuffer),
  .priority = (osPriority_t) osPriorityBelowNormal,
};
/* Definitions for RobotManager */
osThreadId_t RobotManagerHandle;
uint32_t RobotManagerBuffer[ 2048 ];
osStaticThreadDef_t RobotManagerControlBlock;
const osThreadAttr_t RobotManager_attributes = {
  .name = "RobotManager",
  .cb_mem = &RobotManagerControlBlock,
  .cb_size = sizeof(RobotManagerControlBlock),
  .stack_mem = &RobotManagerBuffer[0],
  .stack_size = sizeof(RobotManagerBuffer),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for TeachReceive */
osThreadId_t TeachReceiveHandle;
uint32_t TeachReceiveBuffer[ 256 ];
osStaticThreadDef_t TeachReceiveControlBlock;
const osThreadAttr_t TeachReceive_attributes = {
  .name = "TeachReceive",
  .cb_mem = &TeachReceiveControlBlock,
  .cb_size = sizeof(TeachReceiveControlBlock),
  .stack_mem = &TeachReceiveBuffer[0],
  .stack_size = sizeof(TeachReceiveBuffer),
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for MotorFeedback */
osThreadId_t MotorFeedbackHandle;
uint32_t MotorFeedbackBuffer[ 512 ];
osStaticThreadDef_t MotorFeedbackControlBlock;
const osThreadAttr_t MotorFeedback_attributes = {
  .name = "MotorFeedback",
  .cb_mem = &MotorFeedbackControlBlock,
  .cb_size = sizeof(MotorFeedbackControlBlock),
  .stack_mem = &MotorFeedbackBuffer[0],
  .stack_size = sizeof(MotorFeedbackBuffer),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for HostComm */
osThreadId_t HostCommHandle;
uint32_t HostCommBuffer[ 1024 ];
osStaticThreadDef_t HostCommControlBlock;
const osThreadAttr_t HostComm_attributes = {
  .name = "HostComm",
  .cb_mem = &HostCommControlBlock,
  .cb_size = sizeof(HostCommControlBlock),
  .stack_mem = &HostCommBuffer[0],
  .stack_size = sizeof(HostCommBuffer),
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for TeachTargetQueue */
osMessageQueueId_t TeachTargetQueueHandle;
const osMessageQueueAttr_t TeachTargetQueue_attributes = {
  .name = "TeachTargetQueue"
};
/* Definitions for RLCommandQueue */
osMessageQueueId_t RLCommandQueueHandle;
const osMessageQueueAttr_t RLCommandQueue_attributes = {
  .name = "RLCommandQueue"
};
/* Definitions for MotorFeedbackQueue */
osMessageQueueId_t MotorFeedbackQueueHandle;
const osMessageQueueAttr_t MotorFeedbackQueue_attributes = {
  .name = "MotorFeedbackQueue"
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
void SbusReceive_Task(void *argument);
void MotorControl_Task(void *argument);
void RobotManager_Task(void *argument);
void TeachReceive_Task(void *argument);
void MotorFeedback_Task(void *argument);
void HostComm_Task(void *argument);

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

  /* Create the queue(s) */
  /* creation of TeachTargetQueue */
  TeachTargetQueueHandle = osMessageQueueNew (1, sizeof(TeachTarget_t), &TeachTargetQueue_attributes);

  /* creation of RLCommandQueue */
  RLCommandQueueHandle = osMessageQueueNew (1, sizeof(RLCommand_t), &RLCommandQueue_attributes);

  /* creation of MotorFeedbackQueue */
  MotorFeedbackQueueHandle = osMessageQueueNew (1, sizeof(MotorFeedback_t), &MotorFeedbackQueue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of SbusReceive */
  SbusReceiveHandle = osThreadNew(SbusReceive_Task, NULL, &SbusReceive_attributes);

  /* creation of MotorControl */
  MotorControlHandle = osThreadNew(MotorControl_Task, NULL, &MotorControl_attributes);

  /* creation of RobotManager */
  RobotManagerHandle = osThreadNew(RobotManager_Task, NULL, &RobotManager_attributes);

  /* creation of TeachReceive */
  TeachReceiveHandle = osThreadNew(TeachReceive_Task, NULL, &TeachReceive_attributes);

  /* creation of MotorFeedback */
  MotorFeedbackHandle = osThreadNew(MotorFeedback_Task, NULL, &MotorFeedback_attributes);

  /* creation of HostComm */
  HostCommHandle = osThreadNew(HostComm_Task, NULL, &HostComm_attributes);

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

/* USER CODE BEGIN Header_SbusReceive_Task */
/**
* @brief Function implementing the SbusReceive thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_SbusReceive_Task */
__weak void SbusReceive_Task(void *argument)
{
  /* USER CODE BEGIN SbusReceive_Task */
  UNUSED(argument);
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END SbusReceive_Task */
}

/* USER CODE BEGIN Header_MotorControl_Task */
/**
* @brief Function implementing the MotorControl thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_MotorControl_Task */
__weak void MotorControl_Task(void *argument)
{
  /* USER CODE BEGIN MotorControl_Task */
  UNUSED(argument);
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END MotorControl_Task */
}

/* USER CODE BEGIN Header_RobotManager_Task */
/**
* @brief Function implementing the RobotManager thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_RobotManager_Task */
__weak void RobotManager_Task(void *argument)
{
  /* USER CODE BEGIN RobotManager_Task */
  UNUSED(argument);
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END RobotManager_Task */
}

/* USER CODE BEGIN Header_TeachReceive_Task */
/**
* @brief Function implementing the TeachReceive thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_TeachReceive_Task */
__weak void TeachReceive_Task(void *argument)
{
  /* USER CODE BEGIN TeachReceive_Task */
  UNUSED(argument);
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END TeachReceive_Task */
}

/* USER CODE BEGIN Header_MotorFeedback_Task */
/**
* @brief Function implementing the MotorFeedback thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_MotorFeedback_Task */
__weak void MotorFeedback_Task(void *argument)
{
  /* USER CODE BEGIN MotorFeedback_Task */
  UNUSED(argument);
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END MotorFeedback_Task */
}

/* USER CODE BEGIN Header_HostComm_Task */
/**
* @brief Function implementing the HostComm thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_HostComm_Task */
__weak void HostComm_Task(void *argument)
{
  /* USER CODE BEGIN HostComm_Task */
  UNUSED(argument);
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END HostComm_Task */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

