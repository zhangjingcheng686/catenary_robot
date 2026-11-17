#include "DBusSys.h"
#include "usart.h"
#include "string.h"
#include "stdlib.h"
#include <stdio.h>
#include "uart_api.h"

remoter_t remoter;
uart_rx_t DBus_msg;
uart_msg_t Dbus_rx_msg;
uint8_t Dbus_rx_buff[DBUS_BUFF_SIZE];

typedef struct {
    uint8_t data[DBUS_BUFF_SIZE];
    uint32_t size;
    uint32_t receive_time_ms;
} SbusRxFrame_t;

static SbusRxFrame_t sbus_rx_frame;
static volatile uint32_t sbus_frame_sequence;
static uint32_t sbus_taken_sequence;

static void dbus_set_offline(void)
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

static void dbus_receive_frame(uint8_t *data, uint32_t size)
{
    uint32_t copy_size = size;
    if (copy_size > sizeof(sbus_rx_frame.data)) {
        copy_size = sizeof(sbus_rx_frame.data);
    }

    /* 一起发布字节、实际长度和时间戳，奇数表示正在写入。 */
    sbus_frame_sequence++;
    __DMB();
    memcpy(sbus_rx_frame.data, data, copy_size);
    sbus_rx_frame.size = size;
    sbus_rx_frame.receive_time_ms = HAL_GetTick();
    __DMB();
    sbus_frame_sequence++;
}

static bool dbus_take_frame(SbusRxFrame_t *frame)
{
    uint32_t sequence = sbus_frame_sequence;
    if (sequence == sbus_taken_sequence || (sequence & 1U) != 0U) {
        return false;
    }

    __DMB();
    *frame = sbus_rx_frame;
    __DMB();
    if (sequence != sbus_frame_sequence) {
        return false;
    }

    sbus_taken_sequence = sequence;
    return true;
}

void DBus_Init(void)
{
    remoter = (remoter_t){0};
    dbus_set_offline();
    sbus_rx_frame = (SbusRxFrame_t){0};
    sbus_frame_sequence = 0;
    sbus_taken_sequence = 0;
    DBus_msg.count = 0;
    DBus_msg.time = 0;

    DBus_msg.rx_msg = &Dbus_rx_msg;
    DBus_msg.rx_msg->pBuffer = Dbus_rx_buff;
    DBus_msg.rx_msg->huart = &huart5;         
    DBus_msg.rx_msg->Len = sizeof(Dbus_rx_buff);
    uart_rx_hook_reg(&DBus_msg, dbus_receive_frame);
    uart_rx_init(&DBus_msg);
}

void DBus_Refresh(void)
{
    DBus_msg.time = (DBus_msg.time >= 0xffff) ? 0xffff : (DBus_msg.time + 1);

    SbusRxFrame_t frame;
    if (dbus_take_frame(&frame)) {
        remoter_t decoded = {0};
        SbusFrameResult_t result = sbus_frame_parse(&decoded, frame.data, frame.size);

        if (result == SBUS_FRAME_VALID &&
            HAL_GetTick() - frame.receive_time_ms <= DBUS_TIMEOUT_MS) {
            remoter.rc = decoded.rc;
            remoter.last_update_time = frame.receive_time_ms;
            remoter.online = 1;
            DBus_msg.time = 0;
        } else if (result == SBUS_FRAME_FAILSAFE) {
            dbus_set_offline();
        }
    }

    /* 每轮检查超时，坏帧与丢帧标志都不能刷新最近正常帧的时间。 */
    if (HAL_GetTick() - remoter.last_update_time > DBUS_TIMEOUT_MS) {
        dbus_set_offline();
    }
}

SbusFrameResult_t sbus_frame_parse(remoter_t *remoter, const uint8_t *buf, uint32_t size)
{
    if (remoter == NULL || buf == NULL || size != DBUS_BUFF_SIZE) {
        return SBUS_FRAME_INVALID;
    }

    if ((buf[0] != SBUS_HEAD) || (buf[24] != SBUS_END))
    {
        return SBUS_FRAME_INVALID;
    }

    if ((buf[23] & SBUS_FLAG_FAILSAFE) != 0U) {
        return SBUS_FRAME_FAILSAFE;
    }
    if ((buf[23] & SBUS_FLAG_FRAME_LOST) != 0U) {
        return SBUS_FRAME_INVALID;
    }

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
    return SBUS_FRAME_VALID;
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
