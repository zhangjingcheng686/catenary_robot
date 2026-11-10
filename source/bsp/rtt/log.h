#ifndef LOG_H
#define LOG_H

#include "SEGGER_RTT.h"

// 日志级别定义
#define LOG_LEVEL_NOLOG 0 // 无日志输出
#define LOG_LEVEL_ERROR 1 // 只输出错误
#define LOG_LEVEL_WARN 2  // 输出警告和错误
#define LOG_LEVEL_INFO 3  // 输出信息、警告和错误
#define LOG_LEVEL_DEBUG 4 // 输出调试信息、信息、警告和错误

// 设置当前日志级别
#define LOG_LEVEL LOG_LEVEL_DEBUG

// 常用文本颜色控制宏
#define LOG_RESET RTT_CTRL_RESET
#define LOG_RED RTT_CTRL_TEXT_RED
#define LOG_GREEN RTT_CTRL_TEXT_GREEN
#define LOG_YELLOW RTT_CTRL_TEXT_YELLOW
#define LOG_BLUE RTT_CTRL_TEXT_BLUE
#define LOG_MAGENTA RTT_CTRL_TEXT_MAGENTA
#define LOG_CYAN RTT_CTRL_TEXT_CYAN
#define LOG_WHITE RTT_CTRL_TEXT_WHITE

// 日志输出宏定义
#if LOG_LEVEL >= LOG_LEVEL_DEBUG
#define LOG(fmt, ...) SEGGER_RTT_printf(0, LOG_RESET "[DEBUG]: " fmt "\r\n", ##__VA_ARGS__)
#else
#define LOG(fmt, ...)
#endif

#if LOG_LEVEL >= LOG_LEVEL_INFO
#define LOG_INFO(fmt, ...) SEGGER_RTT_printf(0, LOG_GREEN "[INFO]: " fmt "\r\n", ##__VA_ARGS__)
#else
#define LOG_INFO(fmt, ...)
#endif

#if LOG_LEVEL >= LOG_LEVEL_WARN
#define LOG_WARN(fmt, ...) SEGGER_RTT_printf(0, LOG_YELLOW "[WARN]: " fmt "\r\n", ##__VA_ARGS__)
#else
#define LOG_WARN(fmt, ...)
#endif

#if LOG_LEVEL >= LOG_LEVEL_ERROR
#define LOG_ERROR(fmt, ...) SEGGER_RTT_printf(0, LOG_RED "[ERROR]: " fmt "\r\n", ##__VA_ARGS__)
#else
#define LOG_ERROR(fmt, ...)
#endif

#define LOG_CLEAR() SEGGER_RTT_WriteString(0, "\r\n" RTT_CTRL_CLEAR)                                    // 清屏

// 打印数组
#define LOG_ARRAY(pArr, len, logArrayType, terminal)                    \
    do                                                                  \
    {                                                                   \
        SEGGER_RTT_SetTerminal(terminal);                               \
        for (int i = 0; i < len; i++)                                   \
        {                                                               \
            switch (logArrayType)                                       \
            {                                                           \
            case LOG_ARR_BYTE1:                                         \
                SEGGER_RTT_printf(0, "%02x ", *(uint8_t *)(pArr + i));  \
                break;                                                  \
            case LOG_ARR_BYTE2:                                         \
                SEGGER_RTT_printf(0, "%04x ", *(uint16_t *)(pArr + i)); \
                break;                                                  \
            case LOG_ARR_BYTE4:                                         \
                SEGGER_RTT_printf(0, "%08x ", *(uint32_t *)(pArr + i)); \
                break;                                                  \
            case LOG_ARR_FLOAT:                                         \
                SEGGER_RTT_printf(0, "%f ", *(float *)(pArr + i));      \
                break;                                                  \
            case LOG_ARR_INT:                                           \
                SEGGER_RTT_printf(0, "%d ", *(int *)(pArr + i));        \
                break;                                                  \
            default:                                                    \
                break;                                                  \
            }                                                           \
        }                                                               \
        SEGGER_RTT_printf(0, "\r\n");                                   \
        SEGGER_RTT_SetTerminal(0);                                      \
    } while (0)

// 初始化
#define LOG_INIT()               \
    do                           \
    {                            \
        SEGGER_RTT_Init();       \
        DWT_Init(480);           \
        LOG_CLEAR();             \
        LOG_INFO("RTT Inited!"); \
    } while (0)


#endif // LOG_H
