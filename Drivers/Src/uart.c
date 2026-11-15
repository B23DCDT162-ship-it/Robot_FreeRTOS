
#include "uart.h"
#include "bsp_uart.h"

void UART_Init(uint32_t baudrate)
{
    BSP_UART2_Init(baudrate);
}

void UART_SendChar(char data)
{
    BSP_UART2_SendChar(data);
}

void UART_SendString(const char *str)
{
    BSP_UART2_SendString(str);
}

uint8_t UART_ReadChar(char *data)
{
    return BSP_UART2_ReadChar(data);
}
