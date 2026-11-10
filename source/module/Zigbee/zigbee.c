#include "zigbee.h"
#include "uart_api.h"
#include <string.h>

float Remote_TargetAngles[7] = {0.0f};
uint8_t Remote_Data_Valid = 0;
uint32_t Zigbee_Last_Update_Time = 0;

static uart_rx_t zigbee_rx_msg;
static uart_msg_t zigbee_uart_msg;
static uint8_t zigbee_rx_buff[100]; 

void Zigbee_Rx_Hook(uint8_t *pData, uint32_t size)
{
    if (size >= sizeof(Zigbee_Frame_t)) 
    {
        for (uint32_t i = 0; i <= size - sizeof(Zigbee_Frame_t); i++) 
        {
            Zigbee_Frame_t *frame = (Zigbee_Frame_t *)(&pData[i]);

            if (frame->header == ZIGBEE_HEADER && frame->tail == ZIGBEE_TAIL) 
            {
                uint8_t sum = 0;
                uint8_t *raw_data = (uint8_t *)frame;
                for(int j = 0; j < (int)(sizeof(Zigbee_Frame_t) - 3); j++) {
                    sum += raw_data[j];
                }

                if (sum == frame->checksum) 
                {
                    for(int k = 0; k < 7; k++) {
                        Remote_TargetAngles[k] = frame->angles[k];
                    }
                    Remote_Data_Valid = 1;              
                    Zigbee_Last_Update_Time = HAL_GetTick(); 
                    break; 
                }
            }
        }
    }
}

void Zigbee_Rx_Init(UART_HandleTypeDef *huart)
{
    zigbee_uart_msg.huart = huart;
    zigbee_uart_msg.pBuffer = zigbee_rx_buff;
    zigbee_uart_msg.Len = 50; 

    zigbee_rx_msg.rx_msg = &zigbee_uart_msg;
    zigbee_rx_msg.count = 0;
    zigbee_rx_msg.time = 0;

    uart_rx_init(&zigbee_rx_msg);
    uart_rx_hook_reg(&zigbee_rx_msg, Zigbee_Rx_Hook);
}