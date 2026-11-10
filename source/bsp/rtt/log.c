
#ifndef LOG_H
#define LOG_H
#include "SEGGER_RTT.h"
#if defined(USE_HAL_DRIVER)
// extern float DWT_GetTimeline_ms(void);
extern uint64_t DWT_GetTimeline_us(void);
#define SYS_MS (DWT_GetTimeline_us() / 1000)
#define SYS_US DWT_GetTimeline_us()
#else
#define SYS_MS 0 // 如果不是使用HAL库 则使用自定义的时间源
#define SYS_US 0
#endif

#define USE_LOG_DEBUG 1
#define PRINT_TIMESTAMP 1

typedef enum
{
    LOG_TERMINAL0,
    LOG_TERMINAL1,
    LOG_TERMINAL2,
    LOG_TERMINAL3,
    LOG_TERMINAL4,
    LOG_TERMINAL5,
} LogTerminal_e;

typedef enum
{
    LOG_ARR_BYTE1,
    LOG_ARR_BYTE2,
    LOG_ARR_BYTE4,
    LOG_ARR_FLOAT,
    LOG_ARR_INT,
} LogArrayType_e;

#if USE_LOG_DEBUG
#if PRINT_TIMESTAMP
#define LOG_PROTO(type, color, format, ...)                                                                                        \
    SEGGER_RTT_printf(0, "[%02d:%03d:%03d] %s%s" format "%s\r\n", (SYS_MS / 1000) % 60, SYS_MS % 1000, SYS_US % 1000, color, type, \
                      ##__VA_ARGS__, RTT_CTRL_RESET);
// #define LOG_PROTO(type, color, format, ...)                                                                                                
        //     SEGGER_RTT_printf(0, "[%02d:%02d:%02d:%03d:%03d] %s%s" format "%s\r\n", (SYS_MS / (60 * 60 * 1000)) % 24, (SYS_MS / (60 * 1000)) % 60, 
        //                       (SYS_MS / 1000) % 60, SYS_MS % 1000, SYS_US % 1000, color, type, ##__VA_ARGS__, RTT_CTRL_RESET);
#else
#define LOG_PROTO(type, color, format, ...) SEGGER_RTT_printf(0, "%s%s" format "%s\r\n", color, type, ##__VA_ARGS__, RTT_CTRL_RESET);
#endif


#else
#define LOG_INFO(format, ...)
#define LOG_DEBUG(format, ...)
#define LOG_WARN(format, ...)
#define LOG_ERROR(format, ...)
#define LOG_CLEAR()
#define LOG_ARRAY(pArr, len, logArrayType, terminal)
#define LOG_INIT()
#endif

#endif
