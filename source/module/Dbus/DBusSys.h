#ifndef __DBUSSYS_H__
#define __DBUSSYS_H__

#include "main.h"
#include "uart_api.h"
#include <stdbool.h>

#define DBUS_BUFF_SIZE 25  
#define SBUS_HEAD 0x0F
#define SBUS_END  0x00
#define DBUS_TIMEOUT_MS 1000 

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
void sbus_frame_parse(remoter_t *remoter, uint8_t *buf);
void DBus_SendChannelData(remoter_t *remoter, int channel);

#endif // __DBUSSYS_H__
