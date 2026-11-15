
#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"

#include "board.h"
#include "bsp_timer.h"
#include "motor.h"
#include "ultrasonic.h"
#include "encoder.h"

/* ================= CAU HINH ================= */

#define MOTOR_SPEED             40U
#define TURN_SPEED              40U

#define OBSTACLE_DISTANCE       10U

#define STOP_TIME               300U
#define BACKWARD_TIME           300U

#define TURN_STEP_TIME          500U
#define TURN_CHECK_TIME         100U

#define CHECK_INTERVAL          50U

#define MAX_TURN_STEPS          15U

/* Phat hien ket */
#define STUCK_COUNT_LIMIT       10U

/* So lan quay khi bi ket */
#define STUCK_TURN_STEPS        5U

/* Bluetooth */
#define BT_BAUDRATE             9600U

/* ================= CHE DO ROBOT ================= */

typedef enum
{
    MODE_AUTO = 0,
    MODE_MANUAL
} RobotMode_t;

volatile RobotMode_t robot_mode = MODE_AUTO;
volatile char bluetooth_command = 'S';

/* ================= KHAI BAO ================= */

void LED_Task(void *pvParameters);
void Robot_Task(void *pvParameters);

void Bluetooth_Init(void);
uint8_t Is_Obstacle(uint32_t distance);

void Manual_Control(void);

uint8_t Wait_Check_Mode(uint32_t time_ms);

uint8_t Turn_Left_Scan(void);
uint8_t Avoid_Obstacle(void);
uint8_t Recover_From_Stuck(void);

/* ================= MAIN ================= */

int main(void)
{
    SystemInit();
    SystemCoreClockUpdate();

    Board_Init();
    BSP_Timer_Init();

    Motor_Init();
    Ultrasonic_Init();
    Encoder_Init();

    Encoder_Reset();

    Bluetooth_Init();

    Motor_Stop();

    xTaskCreate(
        LED_Task,
        "LED",
        128,
        NULL,
        1,
        NULL
    );

    xTaskCreate(
        Robot_Task,
        "Robot",
        256,
        NULL,
        3,
        NULL
    );

    vTaskStartScheduler();

    while (1)
    {
    }
}

/* ================= BLUETOOTH USART2 ================= */

void Bluetooth_Init(void)
{
    GPIO_InitTypeDef gpio;
    USART_InitTypeDef usart;
    NVIC_InitTypeDef nvic;

    RCC_APB2PeriphClockCmd(
        RCC_APB2Periph_GPIOA,
        ENABLE
    );

    RCC_APB1PeriphClockCmd(
        RCC_APB1Periph_USART2,
        ENABLE
    );

    /* PA2 - USART2_TX */
    gpio.GPIO_Pin = GPIO_Pin_2;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &gpio);

    /* PA3 - USART2_RX */
    gpio.GPIO_Pin = GPIO_Pin_3;
    gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &gpio);

    /* USART2 */
    USART_StructInit(&usart);

    usart.USART_BaudRate = BT_BAUDRATE;
    usart.USART_WordLength = USART_WordLength_8b;
    usart.USART_StopBits = USART_StopBits_1;
    usart.USART_Parity = USART_Parity_No;
    usart.USART_HardwareFlowControl =
        USART_HardwareFlowControl_None;
    usart.USART_Mode =
        USART_Mode_Rx | USART_Mode_Tx;

    USART_Init(USART2, &usart);

    /* Cho phep ngat nhan */
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);

    /* Cau hinh NVIC */
    nvic.NVIC_IRQChannel = USART2_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = 5;
    nvic.NVIC_IRQChannelSubPriority = 0;
    nvic.NVIC_IRQChannelCmd = ENABLE;

    NVIC_Init(&nvic);

    USART_Cmd(USART2, ENABLE);
}

/* ================= NGAT NHAN BLUETOOTH ================= */

void USART2_IRQHandler(void)
{
    char rx;

    if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
    {
        rx = (char)USART_ReceiveData(USART2);

        /* Chuyen chu thuong thanh chu hoa */
        if (rx >= 'a' && rx <= 'z')
        {
            rx = rx - 'a' + 'A';
        }

        switch (rx)
        {
            case 'A':
                bluetooth_command = 'S';
                robot_mode = MODE_AUTO;
                break;

            case 'M':
                bluetooth_command = 'S';
                robot_mode = MODE_MANUAL;
                break;

            case 'F':
            case 'B':
            case 'L':
            case 'R':
            case 'S':

                if (robot_mode == MODE_MANUAL)
                {
                    bluetooth_command = rx;
                }

                break;

            default:
                break;
        }
    }
}

/* ================= KIEM TRA VAT CAN ================= */

uint8_t Is_Obstacle(uint32_t distance)
{
    if (distance > 0U && distance <= OBSTACLE_DISTANCE)
    {
        return 1U;
    }

    return 0U;
}

/* ================= KIEM TRA CHE DO TRONG KHI CHO ================= */

uint8_t Wait_Check_Mode(uint32_t time_ms)
{
    uint32_t elapsed = 0U;

    while (elapsed < time_ms)
    {
        if (robot_mode != MODE_AUTO)
        {
            Motor_Stop();
            return 0U;
        }

        vTaskDelay(pdMS_TO_TICKS(10));

        elapsed += 10U;
    }

    if (robot_mode != MODE_AUTO)
    {
        Motor_Stop();
        return 0U;
    }

    return 1U;
}

/* ================= LED TASK ================= */

void LED_Task(void *pvParameters)
{
    (void)pvParameters;

    while (1)
    {
        Board_LED_Toggle();

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

/* ================= DIEU KHIEN THU CONG ================= */

void Manual_Control(void)
{
    switch (bluetooth_command)
    {
        case 'F':
            Motor_Set(MOTOR_FORWARD, MOTOR_SPEED);
            break;

        case 'B':
            Motor_Set(MOTOR_BACKWARD, MOTOR_SPEED);
            break;

        case 'L':
            Motor_Set(MOTOR_TURN_LEFT, TURN_SPEED);
            break;

        case 'R':
            Motor_Set(MOTOR_TURN_RIGHT, TURN_SPEED);
            break;

        case 'S':
        default:
            Motor_Stop();
            break;
    }
}

/* ================= QUAY TRAI TUNG DOAN ================= */

uint8_t Turn_Left_Scan(void)
{
    uint8_t i;

    for (i = 0U; i < MAX_TURN_STEPS; i++)
    {
        if (robot_mode != MODE_AUTO)
        {
            Motor_Stop();
            return 0U;
        }

        Motor_Set(MOTOR_TURN_LEFT, TURN_SPEED);

        if (!Wait_Check_Mode(TURN_STEP_TIME))
        {
            return 0U;
        }

        Motor_Stop();

        if (!Wait_Check_Mode(TURN_CHECK_TIME))
        {
            return 0U;
        }

        if (Is_Obstacle(Get_Distance()) == 0U)
        {
            break;
        }
    }

    Motor_Stop();

    if (!Wait_Check_Mode(150U))
    {
        return 0U;
    }

    return 1U;
}

/* ================= XU LY VAT CAN ================= */

uint8_t Avoid_Obstacle(void)
{
    /* 1. DUNG */
    Motor_Stop();

    if (!Wait_Check_Mode(STOP_TIME))
    {
        return 0U;
    }

    /* 2. LUI */
    Motor_Set(MOTOR_BACKWARD, MOTOR_SPEED);

    if (!Wait_Check_Mode(BACKWARD_TIME))
    {
        return 0U;
    }

    /* 3. DUNG SAU KHI LUI */
    Motor_Stop();

    if (!Wait_Check_Mode(200U))
    {
        return 0U;
    }

    /* 4. QUAY TRAI DE QUET */
    return Turn_Left_Scan();
}

/* ================= XU LY KHI BI KET ================= */

uint8_t Recover_From_Stuck(void)
{
    uint8_t i;

    /* 1. DUNG */
    Motor_Stop();

    if (!Wait_Check_Mode(STOP_TIME))
    {
        return 0U;
    }

    /* 2. LUI */
    Motor_Set(MOTOR_BACKWARD, MOTOR_SPEED);

    if (!Wait_Check_Mode(BACKWARD_TIME))
    {
        return 0U;
    }

    /* 3. DUNG SAU KHI LUI */
    Motor_Stop();

    if (!Wait_Check_Mode(200U))
    {
        return 0U;
    }

    /* 4. QUAY TRAI 5 DOAN */
    for (i = 0U; i < STUCK_TURN_STEPS; i++)
    {
        Motor_Set(MOTOR_TURN_LEFT, TURN_SPEED);

        if (!Wait_Check_Mode(TURN_STEP_TIME))
        {
            return 0U;
        }

        Motor_Stop();

        if (!Wait_Check_Mode(TURN_CHECK_TIME))
        {
            return 0U;
        }
    }

    /* 5. DUNG SAU KHI QUAY */
    Motor_Stop();

    if (!Wait_Check_Mode(150U))
    {
        return 0U;
    }

    return 1U;
}

/* ================= ROBOT TASK ================= */

void Robot_Task(void *pvParameters)
{
    uint32_t distance;

    int16_t left_previous;
    int16_t right_previous;

    int16_t left_current;
    int16_t right_current;

    uint8_t stuck_count = 0U;

    (void)pvParameters;

    left_previous = Encoder_GetLeft();
    right_previous = Encoder_GetRight();

    while (1)
    {
        /* ================= CHE DO THU CONG ================= */

        if (robot_mode == MODE_MANUAL)
        {
            Manual_Control();

            vTaskDelay(pdMS_TO_TICKS(20));

            continue;
        }

        /* ================= DOC CAM BIEN ================= */

        distance = Get_Distance();

        /* Neu vua chuyen sang thu cong */
        if (robot_mode != MODE_AUTO)
        {
            Motor_Stop();
            continue;
        }

        /* ================= VAT CAN ================= */

        if (Is_Obstacle(distance))
        {
            stuck_count = 0U;

            Motor_Stop();

            Avoid_Obstacle();

            /* CAP NHAT LAI ENCODER */
            left_previous = Encoder_GetLeft();
            right_previous = Encoder_GetRight();

            continue;
        }

        /* ================= TIEN ================= */

        Motor_Set(MOTOR_FORWARD, MOTOR_SPEED);

        vTaskDelay(pdMS_TO_TICKS(CHECK_INTERVAL));

        if (robot_mode != MODE_AUTO)
        {
            Motor_Stop();
            continue;
        }

        /* DOC ENCODER */
        left_current = Encoder_GetLeft();
        right_current = Encoder_GetRight();

        /* ================= KIEM TRA KET ================= */

        if ((left_current == left_previous) &&
            (right_current == right_previous))
        {
            if (stuck_count < STUCK_COUNT_LIMIT)
            {
                stuck_count++;
            }
        }
        else
        {
            stuck_count = 0U;
        }

        /* CAP NHAT ENCODER */
        left_previous = left_current;
        right_previous = right_current;

        /* ================= NEU BI KET ================= */

        if (stuck_count >= STUCK_COUNT_LIMIT)
        {
            stuck_count = 0U;

            Recover_From_Stuck();

            /* CAP NHAT LAI ENCODER */
            left_previous = Encoder_GetLeft();
            right_previous = Encoder_GetRight();
        }
    }
}

/* ================= FREE RTOS HOOK ================= */

void vApplicationStackOverflowHook(
    TaskHandle_t xTask,
    char *pcTaskName
)
{
    (void)xTask;
    (void)pcTaskName;

    Motor_Stop();

    while (1)
    {
    }
}

void vApplicationMallocFailedHook(void)
{
    Motor_Stop();

    while (1)
    {
    }
}
