#ifndef __ZIGBEE_H__
#define __ZIGBEE_H__

#include "main.h"

#define ZIGBEE_HEADER 0x55AA
#define ZIGBEE_TAIL   0xA55A

#pragma pack(push, 1) 
typedef struct {
    uint16_t header;      
    float angles[7];      
    uint8_t checksum;     
    uint16_t tail;        
} Zigbee_Frame_t;
#pragma pack(pop)

extern float Remote_TargetAngles[7];
extern uint8_t Remote_Data_Valid;
extern uint32_t Zigbee_Last_Update_Time;

void Zigbee_Rx_Init(UART_HandleTypeDef *huart);

#endif /* __ZIGBEE_H__ */