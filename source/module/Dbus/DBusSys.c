#include "DBusSys.h"
#include "usart.h"
#include "string.h"
#include "stdlib.h"
#include <stdio.h>
#include "uart_api.h"
#include "cmsis_os2.h" 
#include "dwt_bsp.h"

remoter_t remoter;
uart_rx_t DBus_msg;
uart_msg_t Dbus_rx_msg;
uint8_t Dbus_rx_buff[DBUS_BUFF_SIZE];
uint32_t Last_Count;

void DBus_Init(void)
{
    DBus_msg.rx_msg = &Dbus_rx_msg;
    DBus_msg.rx_msg->pBuffer = Dbus_rx_buff;
    DBus_msg.rx_msg->huart = &huart5;         
    DBus_msg.rx_msg->Len = DBUS_BUFF_SIZE * 2;
    uart_rx_init(&DBus_msg);

    remoter.online = 0;
    remoter.last_update_time = 0;
    Last_Count = 0;
}

void DBus_Refresh(void)
{
    static uint32_t last_update_time = 0;
    uint32_t current_time = DWT_GetTimeline_ms();

    if (DBus_msg.count != Last_Count)
    {
        sbus_frame_parse(&remoter, Dbus_rx_buff);
        Last_Count = DBus_msg.count;
        last_update_time = current_time; 
        remoter.online = 1; 
        DBus_msg.time = 0;
    }
     else
    {
        DBus_msg.time = (DBus_msg.time >= 0xffff) ? 0xffff : (DBus_msg.time + 1);
        if (current_time  - last_update_time > DBUS_TIMEOUT_MS)
        {
            remoter.online = 0; 
            
            remoter.rc.ch[2] = 992; 
            remoter.rc.ch[4] = 992;  
            remoter.rc.ch[5] = 992;  
            remoter.rc.ch[6] = 992;  
            remoter.rc.ch[7] = 272;  
            remoter.rc.ch[8] = 272;  
            remoter.rc.ch[9] = 272; 
        }
    }

}

void sbus_frame_parse(remoter_t *remoter, uint8_t *buf)
{
    if ((buf[0] != SBUS_HEAD) || (buf[24] != SBUS_END))
    {
        static uint32_t error_count = 0;
        error_count++;
        return;
    }

    remoter->online = (buf[23] != 0x0C);

    remoter->rc.ch[0] = ((int16_t)buf[ 1] >> 0  | ((int16_t)buf[ 2] << 8 )) & 0x07FF;
    remoter->rc.ch[1] = ((int16_t)buf[ 2] >> 3  | ((int16_t)buf[ 3] << 5 )) & 0x07FF;
    remoter->rc.ch[2] = ((int16_t)buf[ 3] >> 6  | ((int16_t)buf[ 4] << 2 )  | (int16_t)buf[ 5] << 10 ) & 0x07FF;
    remoter->rc.ch[3] = ((int16_t)buf[ 5] >> 1  | ((int16_t)buf[ 6] << 7 )) & 0x07FF;
    remoter->rc.ch[4] = ((int16_t)buf[ 6] >> 4  | ((int16_t)buf[ 7] << 4 )) & 0x07FF;
    remoter->rc.ch[5] = ((int16_t)buf[ 7] >> 7  | ((int16_t)buf[ 8] << 1 )  | (int16_t)buf[ 9] << 9 ) & 0x07FF;
    remoter->rc.ch[6] = ((int16_t)buf[ 9] >> 2  | ((int16_t)buf[10] << 6 )) & 0x07FF;
    remoter->rc.ch[7] = ((int16_t)buf[10] >> 5  | ((int16_t)buf[11] << 3 )) & 0x07FF;
    remoter->rc.ch[8] = ((int16_t)buf[12] >> 0  | ((int16_t)buf[13] << 8 )) & 0x07FF;
    remoter->rc.ch[9] = ((int16_t)buf[13] >> 3  | ((int16_t)buf[14] << 5 )) & 0x07FF;
    remoter->rc.ch[10] = ((int16_t)buf[14] >> 6  | ((int16_t)buf[15] << 2 )  | (int16_t)buf[16] << 10 ) & 0x07FF;
    remoter->rc.ch[11] = ((int16_t)buf[16] >> 1  | ((int16_t)buf[17] << 7 )) & 0x07FF;
}

void DBus_SendChannelData(remoter_t *remoter, int channel)
{
    uart_msg_t uart_msg;
    static char tx_buff[64];  
    int len = sprintf(tx_buff, "Channel %d: %d\r\n", channel, remoter->rc.ch[channel]);

    uart_msg.huart = &huart1;
    uart_msg.pBuffer = (uint8_t *)tx_buff;
    uart_msg.Len = len;

    uart_tx_send_IT(&uart_msg);  
    // HAL_UART_Transmit(&huart1, (uint8_t *)tx_buff, len, 100);
}
