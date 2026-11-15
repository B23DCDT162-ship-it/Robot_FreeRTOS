
#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_tim.h"

#include "motor.h"

#define PWM_PERIOD 999U

/* Dieu khien ben trai: ENB = PA9, IN3 = PB14, IN4 = PB15 */
void Motor_SetLeft(uint8_t forward, uint8_t duty)
{
    if (forward)
    {
        GPIO_ResetBits(GPIOB, GPIO_Pin_14);
        GPIO_SetBits(GPIOB, GPIO_Pin_15);
    }
    else
    {
        GPIO_SetBits(GPIOB, GPIO_Pin_14);
        GPIO_ResetBits(GPIOB, GPIO_Pin_15);
    }

    TIM_SetCompare2(TIM1, (uint16_t)(duty * 10U));
}

/* Dieu khien ben phai: ENA = PA8, IN1 = PB12, IN2 = PB13 */
void Motor_SetRight(uint8_t forward, uint8_t duty)
{
    if (forward)
    {
        GPIO_SetBits(GPIOB, GPIO_Pin_12);
        GPIO_ResetBits(GPIOB, GPIO_Pin_13);
    }
    else
    {
        GPIO_ResetBits(GPIOB, GPIO_Pin_12);
        GPIO_SetBits(GPIOB, GPIO_Pin_13);
    }

    TIM_SetCompare1(TIM1, (uint16_t)(duty * 10U));
}

/* Khoi tao motor */
void Motor_Init(void)
{
    GPIO_InitTypeDef gpio;
    TIM_TimeBaseInitTypeDef tim;
    TIM_OCInitTypeDef oc;

    RCC_APB2PeriphClockCmd(
        RCC_APB2Periph_GPIOA |
        RCC_APB2Periph_GPIOB |
        RCC_APB2Periph_AFIO |
        RCC_APB2Periph_TIM1,
        ENABLE
    );

    /* PA8 = ENA, PA9 = ENB */
    gpio.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpio);

    /* PB12, PB13, PB14, PB15 */
    gpio.GPIO_Pin = GPIO_Pin_12 |
                    GPIO_Pin_13 |
                    GPIO_Pin_14 |
                    GPIO_Pin_15;
    gpio.GPIO_Mode = GPIO_Mode_Out_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &gpio);

    GPIO_ResetBits(
        GPIOB,
        GPIO_Pin_12 |
        GPIO_Pin_13 |
        GPIO_Pin_14 |
        GPIO_Pin_15
    );

    /* TIMER 1 PWM */
    tim.TIM_Prescaler = 71;
    tim.TIM_CounterMode = TIM_CounterMode_Up;
    tim.TIM_Period = PWM_PERIOD;
    tim.TIM_ClockDivision = TIM_CKD_DIV1;
    tim.TIM_RepetitionCounter = 0;

    TIM_TimeBaseInit(TIM1, &tim);

    TIM_OCStructInit(&oc);
    oc.TIM_OCMode = TIM_OCMode_PWM1;
    oc.TIM_OutputState = TIM_OutputState_Enable;
    oc.TIM_OCPolarity = TIM_OCPolarity_High;
    oc.TIM_Pulse = 0;

    TIM_OC1Init(TIM1, &oc);
    TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);

    TIM_OC2Init(TIM1, &oc);
    TIM_OC2PreloadConfig(TIM1, TIM_OCPreload_Enable);

    TIM_ARRPreloadConfig(TIM1, ENABLE);

    TIM_Cmd(TIM1, ENABLE);
    TIM_CtrlPWMOutputs(TIM1, ENABLE);

    Motor_Stop();
}

/* Dung motor */
void Motor_Stop(void)
{
    TIM_SetCompare1(TIM1, 0);
    TIM_SetCompare2(TIM1, 0);

    GPIO_ResetBits(
        GPIOB,
        GPIO_Pin_12 |
        GPIO_Pin_13 |
        GPIO_Pin_14 |
        GPIO_Pin_15
    );
}

/* Dieu khien robot */
void Motor_Set(MotorCommand_t command, uint8_t duty_percent)
{
    if (duty_percent > 100U)
    {
        duty_percent = 100U;
    }

    switch (command)
    {
        case MOTOR_FORWARD:
            Motor_SetRight(1U, duty_percent);
            Motor_SetLeft(1U, (uint8_t)(duty_percent * 50U / 100U));
            break;

        case MOTOR_BACKWARD:
            Motor_SetLeft(0U, duty_percent);
            Motor_SetRight(0U, duty_percent);
            break;

        case MOTOR_TURN_LEFT:
            /* Banh trai lui, banh phai tien */
            Motor_SetLeft(0U, duty_percent);
            Motor_SetRight(1U, duty_percent);
            break;

        case MOTOR_TURN_RIGHT:

    /* Banh trai tien */
    Motor_SetLeft(1U, duty_percent);

    /* Banh phai lui cham hon */
    Motor_SetRight(0U, (uint8_t)(duty_percent * 70U / 100U));

    break;

        case MOTOR_STOP:
        default:
            Motor_Stop();
            break;
    }
}
