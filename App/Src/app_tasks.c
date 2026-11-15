#include "app_tasks.h"
#include "app_config.h"
#include "board.h"
#include "motor.h"
#include "ultrasonic.h"
#include "encoder.h"
#include "uart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

typedef struct {
    uint16_t distance_cm;
    uint8_t valid;
} SensorMessage_t;

static QueueHandle_t sensor_queue;
static QueueHandle_t command_queue;

static void SensorTask(void *argument)
{
    SensorMessage_t msg;
    (void)argument;

    for (;;) {
        msg.distance_cm = Ultrasonic_ReadCm();
        msg.valid = (msg.distance_cm != 0xFFFFU) ? 1U : 0U;
        (void)xQueueOverwrite(sensor_queue, &msg);
        vTaskDelay(pdMS_TO_TICKS(APP_SENSOR_PERIOD_MS));
    }
}

static void CommandTask(void *argument)
{
    uint8_t ch;
    MotorCommand_t cmd = MOTOR_STOP;
    QueueHandle_t rx = Uart_GetRxQueue();
    (void)argument;

    for (;;) {
        if (xQueueReceive(rx, &ch, portMAX_DELAY) == pdPASS) {
            switch (ch) {
            case 'F': case 'f': cmd = MOTOR_FORWARD; break;
            case 'B': case 'b': cmd = MOTOR_BACKWARD; break;
            case 'L': case 'l': cmd = MOTOR_TURN_LEFT; break;
            case 'R': case 'r': cmd = MOTOR_TURN_RIGHT; break;
            case 'S': case 's': cmd = MOTOR_STOP; break;
            default: continue;
            }
            (void)xQueueOverwrite(command_queue, &cmd);
        }
    }
}

static void MotorControlTask(void *argument)
{
    SensorMessage_t sensor = {0xFFFFU, 0U};
    MotorCommand_t command = MOTOR_STOP;
    MotorCommand_t requested = MOTOR_STOP;
    TickType_t last_wake = xTaskGetTickCount();
    (void)argument;

    for (;;) {
        (void)xQueuePeek(sensor_queue, &sensor, 0);
        (void)xQueuePeek(command_queue, &requested, 0);

        /* Fail-safe: invalid or too-close range means stop. */
        if ((sensor.valid == 0U) ||
            (sensor.distance_cm < APP_OBSTACLE_STOP_CM)) {
            command = MOTOR_STOP;
        } else {
            command = requested;
        }

        Motor_Set(command, APP_DEFAULT_DUTY_PERCENT);
        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(APP_CONTROL_PERIOD_MS));
    }
}

static void MonitorTask(void *argument)
{
    (void)argument;
    for (;;) {
        Board_ToggleStatusLed();
        (void)Encoder_GetLeftDelta();
        (void)Encoder_GetRightDelta();
        vTaskDelay(pdMS_TO_TICKS(APP_MONITOR_PERIOD_MS));
    }
}

BaseType_t AppTasks_Create(void)
{
    SensorMessage_t initial_sensor = {0xFFFFU, 0U};
    MotorCommand_t initial_command = MOTOR_STOP;

    Motor_Init();
    Ultrasonic_Init();
    Encoder_Init();

    sensor_queue = xQueueCreate(1, sizeof(SensorMessage_t));
    command_queue = xQueueCreate(1, sizeof(MotorCommand_t));
    if ((sensor_queue == 0) || (command_queue == 0)) return pdFAIL;

    (void)xQueueOverwrite(sensor_queue, &initial_sensor);
    (void)xQueueOverwrite(command_queue, &initial_command);

    Uart_Init();
    if (Uart_GetRxQueue() == 0) return pdFAIL;

    if (xTaskCreate(SensorTask, "Sensor", 160, 0, 3, 0) != pdPASS) return pdFAIL;
    if (xTaskCreate(CommandTask, "Command", 160, 0, 2, 0) != pdPASS) return pdFAIL;
    if (xTaskCreate(MotorControlTask, "MotorCtrl", 160, 0, 4, 0) != pdPASS) return pdFAIL;
    if (xTaskCreate(MonitorTask, "Monitor", 128, 0, 1, 0) != pdPASS) return pdFAIL;

    return pdPASS;
}

/* Required by FreeRTOSConfig.h. Keep handlers minimal and safe. */
void vApplicationMallocFailedHook(void)
{
    taskDISABLE_INTERRUPTS();
    for (;;) { }
}

void vApplicationStackOverflowHook(TaskHandle_t task, char *task_name)
{
    (void)task;
    (void)task_name;
    taskDISABLE_INTERRUPTS();
    for (;;) { }
}
