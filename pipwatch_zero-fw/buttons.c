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
    btn[0] = (GPIO_ReadInputDataBit(HW_BTN0_Port, HW_BTN0_Pin) == Bit_SET) ? 0 : 1;
    btn[1] = (GPIO_ReadInputDataBit(HW_BTN1_Port, HW_BTN1_Pin) == Bit_SET) ? 0 : 1;
    btn[2] = (GPIO_ReadInputDataBit(HW_BTN2_Port, HW_BTN2_Pin) == Bit_SET) ? 0 : 1;

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


/* OS task handling buttons events */
void ButtonsTask(void *pvParameters)
{
    /* clear state */
    memset(btnsts, 0, 3*sizeof(struct button_state_t));

    /* queue of button presses received in the IRQ handler */
    btnEventQueue = xQueueCreate(16, sizeof(int));

    /* index of the button that changed state, from IRQ handler */
    int ibtn;
    /* max waiting time for the next button irq event */
    int maxwaittm = portMAX_DELAY;

    while (1) {
        int motpulse = 0;
        
        /* receive event from IRQ handler */
        if (xQueueReceive(btnEventQueue, &ibtn, maxwaittm) == pdTRUE) {
            /* Hardware button event registered.
             * create GUI event  */
            struct guievent gevnt;
            gevnt.evnt = GUI_E_BUTTON;
            gevnt.buf = NULL;
            gevnt.kpar = ibtn;

            if (btnsts[ibtn].st) {
                /* the button has been pressed */
                gevnt.kpar |= BTN_DOWN;
                /* record the press-down time */
                btnsts[ibtn].longclick_sent_tick = btnsts[ibtn].kdwn_tick = btnsts[ibtn].tick;
                /* tacktile feedback */
                motpulse = MOTOR_DUR_SHORT;
            } else{
                /* the button has been released */
                gevnt.kpar |= BTN_UP;
                /* was it only a short click? */
                int clickdur = btnsts[ibtn].tick - btnsts[ibtn].kdwn_tick;
                if (clickdur <= (BUTTON_SHORTCLICK_TM/portTICK_PERIOD_MS)) {
                    /* yes */
                    gevnt.kpar |= BTN_SHORTCLICK;
                }
            }
            
            /* send event to GUI task */
            xQueueSend(toGuiQueue, &gevnt, portMAX_DELAY);
        } else {
            /* time-out waiting for hardware button event */
        }

        /* current tick time */
        const TickType_t ctick = xTaskGetTickCount();

        maxwaittm = portMAX_DELAY;

        for (ibtn = 0; ibtn < 3; ++ibtn) {
            if (btnsts[ibtn].st) {
                /* the button is pressed; for how long? */
                int clickdur = ctick - btnsts[ibtn].kdwn_tick;
                if (clickdur > (BUTTON_SHORTCLICK_TM/portTICK_PERIOD_MS)) {
                    /* the button is currently pressed for more than a short click duration;
                     * send BTN_LONGCLICK every BUTTON_LONGCLICK_REPE_TM */
                    int last = ctick - btnsts[ibtn].longclick_sent_tick;
                    if (last > (BUTTON_LONGCLICK_REPE_TM/portTICK_PERIOD_MS)) {
                        /* send new BTN_LONGCLICK event */
                        struct guievent gevnt;
                        gevnt.evnt = GUI_E_BUTTON;
                        gevnt.buf = NULL;
                        gevnt.kpar = ibtn | BTN_LONGCLICK;
                        xQueueSend(toGuiQueue, &gevnt, portMAX_DELAY);
                        /* calculate the next long-click time */
                        maxwaittm = uimin(maxwaittm, (BUTTON_LONGCLICK_REPE_TM/portTICK_PERIOD_MS));
                        /* update the time */
                        btnsts[ibtn].longclick_sent_tick = ctick;
                        /* tacktile feedback */
                        motpulse = MOTOR_DUR_MEDIUM;
                    } else {
                        /* the button is pressed for long, but it is not yet a time to send
                         * the long-click event */
                        maxwaittm = uimin(maxwaittm, 
                            (BUTTON_LONGCLICK_REPE_TM/portTICK_PERIOD_MS) - last);
                    }
                } else {
                    /* the button is pressed, but only for a short time */
                    maxwaittm = uimin(maxwaittm, 
                        (BUTTON_SHORTCLICK_TM/portTICK_PERIOD_MS) - clickdur);
                }
            }
        }


        LEDs_Set(LED2, 
            (btnsts[BTN0].st) ? LED_INTENS_100 : LED_INTENS_0, 
            (btnsts[BTN1].st) ? LED_INTENS_100 : LED_INTENS_0, 
            (btnsts[BTN2].st) ? LED_INTENS_100 : LED_INTENS_0);

        if (motpulse > 0) {
            Motor_Pulse(motpulse);
        }
    }
}
