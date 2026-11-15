
#include "bsp_uart.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_usart.h"

void BSP_UART2_Init(uint32_t baudrate)
{
    GPIO_InitTypeDef gpio;
    USART_InitTypeDef usart;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    /* PA2 - USART2_TX */
    gpio.GPIO_Pin = GPIO_Pin_2;
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpio);

    /* PA3 - USART2_RX */
    gpio.GPIO_Pin = GPIO_Pin_3;
    gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &gpio);

    USART_StructInit(&usart);

    usart.USART_BaudRate = baudrate;
    usart.USART_WordLength = USART_WordLength_8b;
    usart.USART_StopBits = USART_StopBits_1;
    usart.USART_Parity = USART_Parity_No;
    usart.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;

    USART_Init(USART2, &usart);
    USART_Cmd(USART2, ENABLE);
}

void BSP_UART2_SendChar(char data)
{
    USART_SendData(USART2, (uint16_t)data);

    while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
}

void BSP_UART2_SendString(const char *str)
{
    while (*str)
    {
        BSP_UART2_SendChar(*str++);
    }
}

uint8_t BSP_UART2_ReadChar(char *data)
{
    if (USART_GetFlagStatus(USART2, USART_FLAG_RXNE) != RESET)
    {
        *data = (char)USART_ReceiveData(USART2);
        return 1;
    }

    return 0;
}
