
#ifndef MOTOR_H
#define MOTOR_H

#include "stm32f10x.h"

typedef enum
{
    MOTOR_STOP = 0,
    MOTOR_FORWARD,
    MOTOR_BACKWARD,
    MOTOR_TURN_LEFT,
    MOTOR_TURN_RIGHT
} MotorCommand_t;

void Motor_Init(void);
void Motor_Stop(void);
void Motor_Set(MotorCommand_t command, uint8_t duty_percent);

#endif
