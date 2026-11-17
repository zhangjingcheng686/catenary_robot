#ifndef __DBUSSYS_H__
#define __DBUSSYS_H__

#include "main.h"
#include "uart_api.h"
#include <stdbool.h>

#define DBUS_BUFF_SIZE 25  
#define SBUS_HEAD 0x0F
#define SBUS_END  0x00
#define DBUS_TIMEOUT_MS 1000 
#define SBUS_FLAG_FRAME_LOST (1U << 2)
#define SBUS_FLAG_FAILSAFE   (1U << 3)

typedef enum {
    SBUS_FRAME_INVALID = 0,
    SBUS_FRAME_VALID,
    SBUS_FRAME_FAILSAFE
} SbusFrameResult_t;

typedef struct
{
    uint16_t online;
    uint32_t last_update_time;
    struct
    {
        int16_t ch[16];
    } rc;
    
} remoter_t;

extern remoter_t remoter;
extern uint8_t Dbus_rx_buff[DBUS_BUFF_SIZE];  

void DBus_Init(void);
void DBus_Refresh(void);
SbusFrameResult_t sbus_frame_parse(remoter_t *remoter, const uint8_t *buf, uint32_t size);
void DBus_SendChannelData(remoter_t *remoter, int channel);

#endif // __DBUSSYS_H__
