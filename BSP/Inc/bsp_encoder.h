
#ifndef BSP_ENCODER_H
#define BSP_ENCODER_H

#include "stm32f10x.h"

void BSP_Encoder_Init(void);

int16_t BSP_Encoder_GetLeft(void);
int16_t BSP_Encoder_GetRight(void);

void BSP_Encoder_ResetLeft(void);
void BSP_Encoder_ResetRight(void);

#endif
