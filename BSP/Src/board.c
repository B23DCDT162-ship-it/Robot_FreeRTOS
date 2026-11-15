
#include "board.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"

void Board_Init(void)
{
    GPIO_InitTypeDef gpio;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

    gpio.GPIO_Pin = GPIO_Pin_13;
    gpio.GPIO_Mode = GPIO_Mode_Out_PP;
    gpio.GPIO_Speed = GPIO_Speed_2MHz;

    GPIO_Init(GPIOC, &gpio);

    Board_LED_Off();
}

void Board_LED_On(void)
{
    GPIO_ResetBits(GPIOC, GPIO_Pin_13);
}

void Board_LED_Off(void)
{
    GPIO_SetBits(GPIOC, GPIO_Pin_13);
}

void Board_LED_Toggle(void)
{
    GPIOC->ODR ^= GPIO_Pin_13;
}
