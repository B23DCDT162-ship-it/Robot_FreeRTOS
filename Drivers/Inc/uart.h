
#ifndef UART_H
#define UART_H

#include "stm32f10x.h"

void UART_Init(uint32_t baudrate);
void UART_SendChar(char data);
void UART_SendString(const char *str);
uint8_t UART_ReadChar(char *data);

#endif
