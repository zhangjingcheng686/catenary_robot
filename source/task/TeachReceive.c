#include "cmsis_os2.h"
#include "zigbee.h"
#include "usart.h" 
#include "FreeRTOS.h"
#include "queue.h"
#include "robot_messages.h"

extern osMessageQueueId_t TeachTargetQueueHandle;

_Static_assert(TEACH_TARGET_COUNT == ZIGBEE_TARGET_COUNT,
               "Teach queue must contain every Zigbee target");

void TeachReceive_Task(void *argument)
{
    (void)argument;
    configASSERT(TeachTargetQueueHandle != NULL);
    configASSERT(osMessageQueueGetCapacity(TeachTargetQueueHandle) == 1U);
    configASSERT(osMessageQueueGetMsgSize(TeachTargetQueueHandle) == sizeof(TeachTarget_t));
    osDelay(1500);

    Zigbee_Rx_Init(&huart1); 

    TeachTarget_t target = {0};

    for (;;)
    {
        if (Zigbee_TakeTarget(target.input_deg, &target.receive_time_ms)) {
            /* 长度为 1，未处于示教模式时也持续保留最近一帧目标。 */
            xQueueOverwrite((QueueHandle_t)TeachTargetQueueHandle, &target);
        }

        if (HAL_GetTick() - Zigbee_Last_Update_Time > ZIGBEE_TARGET_TIMEOUT_MS) {
            Remote_Data_Valid = 0;
        }
        osDelay(5);
    }
}
