#ifndef UARTALL
#define UARTALL

#include "usart.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
// #include "arm_math.h"
// #include "arm_const_structs.h"

void uart_save_buffer_size(UART_HandleTypeDef *huart, uint16_t size);
#endif // !UARTALL
