
#ifndef ENCODER_H
#define ENCODER_H

#include "stm32f10x.h"

void Encoder_Init(void);

int16_t Encoder_GetLeft(void);
int16_t Encoder_GetRight(void);

void Encoder_Reset(void);

#endif
