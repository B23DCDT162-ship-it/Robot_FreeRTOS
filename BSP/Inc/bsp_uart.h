
#ifndef BSP_UART_H
#define BSP_UART_H

#include "stm32f10x.h"

void BSP_UART2_Init(uint32_t baudrate);
void BSP_UART2_SendChar(char data);
void BSP_UART2_SendString(const char *str);
uint8_t BSP_UART2_ReadChar(char *data);

#endif
