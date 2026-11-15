#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "board.h"
#include "app_tasks.h"
#include "FreeRTOS.h"
#include "task.h"

int main(void)
{
    SystemInit();
    SystemCoreClockUpdate();

    Board_Init();

    if (AppTasks_Create() != pdPASS) {
        Board_SetStatusLed(1);
        for (;;) { }
    }

    vTaskStartScheduler();

    /* Scheduler returns only if there is insufficient heap. */
    Board_SetStatusLed(1);
    for (;;) { }
}
