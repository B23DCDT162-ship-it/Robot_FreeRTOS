#include "stm32f10x.h"
#include "ultrasonic.h"

#define TRIG GPIO_Pin_4
#define ECHO GPIO_Pin_5
#define PORT GPIOA

void Delay_us(uint32_t us)
{
    while(us--)
    {
        for(volatile int i = 0; i < 8; i++);
    }
}

void Ultrasonic_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    /* TRIG PA4: OUTPUT */
    GPIO_InitStructure.GPIO_Pin = TRIG;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(PORT, &GPIO_InitStructure);

    /* ECHO PA5: INPUT */
    GPIO_InitStructure.GPIO_Pin = ECHO;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_Init(PORT, &GPIO_InitStructure);

    GPIO_ResetBits(PORT, TRIG);
}

uint32_t Get_Distance(void)
{
    uint32_t timeout;
    uint32_t time = 0;

    /* Phat xung TRIG */
    GPIO_ResetBits(PORT, TRIG);
    Delay_us(2);

    GPIO_SetBits(PORT, TRIG);
    Delay_us(10);

    GPIO_ResetBits(PORT, TRIG);

    /* Cho ECHO len muc 1 */
    timeout = 30000;

    while(GPIO_ReadInputDataBit(PORT, ECHO) == 0)
    {
        if(timeout == 0)
            return 0;

        timeout--;
    }

    /* Do thoi gian ECHO o muc 1 */
    timeout = 30000;

    while(GPIO_ReadInputDataBit(PORT, ECHO) == 1)
    {
        time++;
        Delay_us(1);

        if(timeout == 0)
            return 0;

        timeout--;
    }

    return time / 58;
}