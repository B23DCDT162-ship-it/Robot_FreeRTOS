
#include "bsp_timer.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_tim.h"

void BSP_Timer_Init(void)
{
    TIM_TimeBaseInitTypeDef tim;
    RCC_ClocksTypeDef clocks;
    uint32_t timer_clock;
    uint16_t prescaler;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

    RCC_GetClocksFreq(&clocks);

    timer_clock = clocks.PCLK1_Frequency;

    if ((RCC->CFGR & RCC_CFGR_PPRE1) != 0)
    {
        timer_clock *= 2U;
    }

    prescaler = (uint16_t)(timer_clock / 1000000U - 1U);

    tim.TIM_Prescaler = prescaler;
    tim.TIM_CounterMode = TIM_CounterMode_Up;
    tim.TIM_Period = 0xFFFF;
    tim.TIM_ClockDivision = TIM_CKD_DIV1;
    tim.TIM_RepetitionCounter = 0;

    TIM_TimeBaseInit(TIM4, &tim);

    TIM_SetCounter(TIM4, 0);
    TIM_Cmd(TIM4, ENABLE);
}

void BSP_DelayUs(uint16_t us)
{
    uint16_t start;

    start = TIM_GetCounter(TIM4);

    while ((uint16_t)(TIM_GetCounter(TIM4) - start) < us);
}

uint16_t BSP_Timer_GetCounter(void)
{
    return TIM_GetCounter(TIM4);
}
