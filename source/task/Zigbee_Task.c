#include "cmsis_os2.h"
#include "zigbee.h"
#include "usart.h" 

void Zigbee_Task(void *argument)
{
    (void)argument;
    osDelay(1500);

    Zigbee_Rx_Init(&huart1); 

    for (;;)
    {
        if (HAL_GetTick() - Zigbee_Last_Update_Time > 500) {
            Remote_Data_Valid = 0;
        }
        osDelay(50); 
    }
}