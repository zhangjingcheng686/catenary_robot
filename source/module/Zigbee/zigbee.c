#include "zigbee.h"
#include "uart_api.h"
#include <math.h>
#include <stddef.h>
#include <string.h>

float Remote_TargetAngles[ZIGBEE_TARGET_COUNT] = {0.0f};
uint8_t Remote_Data_Valid = 0;
uint32_t Zigbee_Last_Update_Time = 0;

static volatile uint32_t zigbee_target_sequence = 0;
static uint32_t zigbee_taken_sequence = 0;

static uart_rx_t zigbee_rx_msg;
static uart_msg_t zigbee_uart_msg;
static uint8_t zigbee_rx_buff[100]; 
static uint8_t zigbee_frame_buffer[sizeof(Zigbee_Frame_t)];
static uint32_t zigbee_frame_size = 0;

static uint8_t Zigbee_FrameIsValid(const Zigbee_Frame_t *frame)
{
    if (frame->header != ZIGBEE_HEADER || frame->tail != ZIGBEE_TAIL) {
        return 0;
    }

    uint8_t checksum = 0;
    const uint8_t *bytes = (const uint8_t *)frame;
    for (size_t i = 0; i < offsetof(Zigbee_Frame_t, checksum); i++) {
        checksum += bytes[i];
    }
    if (checksum != frame->checksum) {
        return 0;
    }

    for (uint32_t i = 0; i < ZIGBEE_TARGET_COUNT; i++) {
        if (!isfinite(frame->angles[i])) {
            return 0;
        }
    }

    return 1;
}

void Zigbee_Rx_Hook(uint8_t *pData, uint32_t size)
{
    for (uint32_t i = 0; i < size; i++) {
        zigbee_frame_buffer[zigbee_frame_size++] = pData[i];
        if (zigbee_frame_size < sizeof(Zigbee_Frame_t)) {
            continue;
        }

        Zigbee_Frame_t frame;
        memcpy(&frame, zigbee_frame_buffer, sizeof(frame));
        if (Zigbee_FrameIsValid(&frame)) {
            /* 奇数表示正在更新，偶数表示七路目标与时间戳已全部写完。 */
            zigbee_target_sequence++;
            __DMB();
            memcpy(Remote_TargetAngles, frame.angles, sizeof(Remote_TargetAngles));
            Zigbee_Last_Update_Time = HAL_GetTick();
            Remote_Data_Valid = 1;
            __DMB();
            zigbee_target_sequence++;
            zigbee_frame_size = 0;
        } else {
            /* 丢掉一个字节后继续查找，缓存保留到下一次串口回调。 */
            memmove(zigbee_frame_buffer, zigbee_frame_buffer + 1,
                    sizeof(zigbee_frame_buffer) - 1U);
            zigbee_frame_size--;
        }
    }
}

uint8_t Zigbee_TakeTarget(float angles[ZIGBEE_TARGET_COUNT], uint32_t *receive_time_ms)
{
    uint32_t sequence = zigbee_target_sequence;
    if (sequence == zigbee_taken_sequence || (sequence & 1U) != 0U) {
        return 0;
    }

    __DMB();
    memcpy(angles, Remote_TargetAngles, sizeof(Remote_TargetAngles));
    *receive_time_ms = Zigbee_Last_Update_Time;
    __DMB();

    /* 复制期间若被串口中断更新，本轮跳过，下一轮重新取完整目标。 */
    if (sequence != zigbee_target_sequence) {
        return 0;
    }

    zigbee_taken_sequence = sequence;
    return 1;
}

void Zigbee_Rx_Init(UART_HandleTypeDef *huart)
{
    zigbee_uart_msg.huart = huart;
    zigbee_uart_msg.pBuffer = zigbee_rx_buff;
    zigbee_uart_msg.Len = sizeof(zigbee_rx_buff);

    zigbee_rx_msg.rx_msg = &zigbee_uart_msg;
    zigbee_rx_msg.count = 0;
    zigbee_rx_msg.time = 0;

    uart_rx_init(&zigbee_rx_msg);
    uart_rx_hook_reg(&zigbee_rx_msg, Zigbee_Rx_Hook);
}
