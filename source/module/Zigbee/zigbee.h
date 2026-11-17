#ifndef __ZIGBEE_H__
#define __ZIGBEE_H__

#include "main.h"

#define ZIGBEE_HEADER 0x55AA
#define ZIGBEE_TAIL   0xA55A
#define ZIGBEE_TARGET_COUNT 7U
#define ZIGBEE_TARGET_TIMEOUT_MS 500U

#pragma pack(push, 1) 
typedef struct {
    uint16_t header;      
    float angles[ZIGBEE_TARGET_COUNT];
    uint8_t checksum;     
    uint16_t tail;        
} Zigbee_Frame_t;
#pragma pack(pop)

extern float Remote_TargetAngles[ZIGBEE_TARGET_COUNT];
extern uint8_t Remote_Data_Valid;
extern uint32_t Zigbee_Last_Update_Time;

void Zigbee_Rx_Init(UART_HandleTypeDef *huart);

/* 由接收任务调用，复制最近一帧尚未取走的完整目标。 */
uint8_t Zigbee_TakeTarget(float angles[ZIGBEE_TARGET_COUNT], uint32_t *receive_time_ms);

#endif /* __ZIGBEE_H__ */
