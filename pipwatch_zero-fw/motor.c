#include "motor.h"
#include "stm32f10x_it.h"
#include "task.h"
#include "queue.h"


/* queue */
QueueHandle_t toMotorQueue = NULL;

void MotorTask(void *pvParameters)
{
    /* queue of button presses received in irq handler */
    toMotorQueue = xQueueCreate(16, sizeof(int));

    while (1) {
        int motrun = 0;
        if (xQueueReceive(toMotorQueue, &motrun, portMAX_DELAY) == pdTRUE) {

            if (motrun > MOTOR_DUR_LONG)
                motrun = MOTOR_DUR_LONG;
            if (motrun < 0)
                motrun = 0;

            GPIO_SetBits(MOTOR_Port, MOTOR_Pin);
            vTaskDelay(( ( TickType_t ) motrun ));
            GPIO_ResetBits(MOTOR_Port, MOTOR_Pin);
        }
    }
}


void Motor_Pulse(int dur)
{
    if (toMotorQueue) {
        xQueueSend(toMotorQueue, &dur, 0);
    }
}
