
#include "bsp_encoder.h"

#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_tim.h"

void BSP_Encoder_Init(void)
{
    GPIO_InitTypeDef gpio;
    TIM_TimeBaseInitTypeDef tim;

    /* Enable GPIOA clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    /* Enable TIM2 and TIM3 clocks */
    RCC_APB1PeriphClockCmd(
        RCC_APB1Periph_TIM2 |
        RCC_APB1Periph_TIM3,
        ENABLE
    );

    /* Left encoder: PA0 = TIM2_CH1, PA1 = TIM2_CH2 */
    gpio.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    gpio.GPIO_Mode = GPIO_Mode_IPU;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpio);

    /* Right encoder: PA6 = TIM3_CH1, PA7 = TIM3_CH2 */
    gpio.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_Init(GPIOA, &gpio);

    /* Timer configuration */
    tim.TIM_Prescaler = 0;
    tim.TIM_CounterMode = TIM_CounterMode_Up;
    tim.TIM_Period = 0xFFFF;
    tim.TIM_ClockDivision = TIM_CKD_DIV1;
    tim.TIM_RepetitionCounter = 0;

    TIM_TimeBaseInit(TIM2, &tim);
    TIM_TimeBaseInit(TIM3, &tim);

    /* TIM2 encoder mode */
    TIM_EncoderInterfaceConfig(
        TIM2,
        TIM_EncoderMode_TI12,
        TIM_ICPolarity_Rising,
        TIM_ICPolarity_Rising
    );

    /* TIM3 encoder mode */
    TIM_EncoderInterfaceConfig(
        TIM3,
        TIM_EncoderMode_TI12,
        TIM_ICPolarity_Rising,
        TIM_ICPolarity_Rising
    );

    /* Input filter: IC1F and IC2F = 6 */
    TIM2->CCMR1 &= ~((0xFU << 4) | (0xFU << 12));
    TIM2->CCMR1 |=  ((6U << 4) | (6U << 12));

    TIM3->CCMR1 &= ~((0xFU << 4) | (0xFU << 12));
    TIM3->CCMR1 |=  ((6U << 4) | (6U << 12));

    /* Reset counters */
    TIM_SetCounter(TIM2, 0);
    TIM_SetCounter(TIM3, 0);

    /* Enable timers */
    TIM_Cmd(TIM2, ENABLE);
    TIM_Cmd(TIM3, ENABLE);
}

/* Read left encoder */
int16_t BSP_Encoder_GetLeft(void)
{
    return (int16_t)TIM_GetCounter(TIM2);
}

/* Read right encoder */
int16_t BSP_Encoder_GetRight(void)
{
    return (int16_t)TIM_GetCounter(TIM3);
}

/* Reset left encoder */
void BSP_Encoder_ResetLeft(void)
{
    TIM_SetCounter(TIM2, 0);
}

/* Reset right encoder */
void BSP_Encoder_ResetRight(void)
{
    TIM_SetCounter(TIM3, 0);
}
