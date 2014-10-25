#include "buttons.h"
#include "main.h"
#include "rtclock.h"
#include "utils.h"
#include "stm32f10x_it.h"
#include "task.h"
#include "queue.h"
#include "leds.h"
#include "motor.h"
#include "gui.h"
#include <string.h>


/* current button state */
struct button_state_t btnsts[3];

/* queue of buttons that have changed their state */
QueueHandle_t btnEventQueue = NULL;



/* Button Press or Release */
void EXTI9_5_IRQHandler(void)
{
    long xHigherPriorityTaskWoken = pdFALSE;

    TickType_t ctick = xTaskGetTickCountFromISR();
    
    /* get current button state */
    int btn[3];
    btn[0] = (GPIO_ReadInputDataBit(BTN0_Port, BTN0_Pin) == Bit_SET) ? 0 : 1;
    btn[1] = (GPIO_ReadInputDataBit(BTN1_Port, BTN1_Pin) == Bit_SET) ? 0 : 1;
    btn[2] = (GPIO_ReadInputDataBit(BTN2_Port, BTN2_Pin) == Bit_SET) ? 0 : 1;

    for (int i = 0; i < 3; ++i) {
        if ((ctick - btnsts[i].tick) > (BUTTON_DEAD_TM/portTICK_PERIOD_MS)) {
            /* the button's dead time is over */
            if (btn[i] != btnsts[i].st) {
                /* change button state */
                btnsts[i].st = btn[i];
                btnsts[i].tick = ctick;

                if (btnEventQueue) {
                    if (xQueueSendFromISR(btnEventQueue, &i, &xHigherPriorityTaskWoken) == pdTRUE) {
                        // ok; will alloc new buffer
                    } else {
                        // fail; ignore, keep buffer
                    }
                }
            }
        }
    }

    /* clear IRQ penging bits */
    // EXTI_ClearITPendingBit(EXTI_Line5 | EXTI_Line6 | EXTI_Line7 | EXTI_Line8 | EXTI_Line9);
    EXTI_ClearITPendingBit(EXTI->PR);

    /* signal end-of-irq and possible reschedule point */
    portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
}


void ButtonsTask(void *pvParameters)
{
    /* queue of button presses received in irq handler */
    btnEventQueue = xQueueCreate(16, sizeof(int));


    while (1) {
        int btn;
        if (xQueueReceive(btnEventQueue, &btn, portMAX_DELAY) == pdFALSE)
            continue;

        LEDs_Set(LED2, 
            (btnsts[BTN0].st) ? LED_INTENS_100 : LED_INTENS_0, 
            (btnsts[BTN1].st) ? LED_INTENS_100 : LED_INTENS_0, 
            (btnsts[BTN2].st) ? LED_INTENS_100 : LED_INTENS_0);

        struct guievent gevnt;
        gevnt.evnt = GUI_E_BUTTON;
        gevnt.buf = NULL;
        gevnt.kpar = btn;

        if (btnsts[btn].st) {
            gevnt.kpar |= BTN_PRESSED;
            Motor_Pulse(MOTOR_DUR_SHORT);
        } else{
            gevnt.kpar |= BTN_RELEASED;
        }
        
        xQueueSend(toGuiQueue, &gevnt, portMAX_DELAY);
    }
}
