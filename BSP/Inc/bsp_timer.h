
#ifndef BSP_TIMER_H
#define BSP_TIMER_H

#include "stm32f10x.h"

void BSP_Timer_Init(void);
void BSP_DelayUs(uint16_t us);
uint16_t BSP_Timer_GetCounter(void);

#endif
