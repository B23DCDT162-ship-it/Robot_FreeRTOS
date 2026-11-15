
#include "encoder.h"
#include "bsp_encoder.h"

void Encoder_Init(void)
{
    BSP_Encoder_Init();
}

int16_t Encoder_GetLeft(void)
{
    return BSP_Encoder_GetLeft();
}

int16_t Encoder_GetRight(void)
{
    return BSP_Encoder_GetRight();
}

void Encoder_Reset(void)
{
    BSP_Encoder_ResetLeft();
    BSP_Encoder_ResetRight();
}
