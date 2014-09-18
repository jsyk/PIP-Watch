#include "leds.h"
#include "main.h"
#include "rtclock.h"
#include "utils.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#define LED_PERIOD_MS               1

#define LEDS_ON_TIMEOUT             (10 * 1000)
#define LEDS_STAY_OFF_TM            (10 * 1000)
#define LEDS_REMEMBER_ON_TM         (500)


/* GLOBAL VARIABLE
 * current LEDs state
 */
struct led_state_t leds_st[3];

/* queue is filled when there is a change in leds_st */
QueueHandle_t toLEDsQueue = NULL;

/* the last tick-time any LED was update */
TickType_t leds_last_upd_tm = 0;

/* current max timout in ticks when LEDs stay ON without change, and then they go OFF */
TickType_t leds_timeout = (LEDS_ON_TIMEOUT / portTICK_PERIOD_MS);



/* Init LEDs internal variables */
void LEDs_Init(void)
{
    toLEDsQueue = xQueueCreate(16, sizeof(int));
    leds_timeout = (LEDS_ON_TIMEOUT / portTICK_PERIOD_MS);

    LEDs_AllOff();
}


void LEDsTask(void *pvParameters)
{
    int iled = 0;
    int ibit = 0;

    do {
        uint8_t imask = (1 << ibit);

        if (leds_st[iled].ri & imask) {
            /* RED is ON */
            GPIO_ResetBits(LEDRK_Port, LEDRK_Pin);      /* activate RED cathode */
        } else {
            GPIO_SetBits(LEDRK_Port, LEDRK_Pin);            /* deactivate RED cathode */
        }

        if (leds_st[iled].gi & imask) {
            /* GREEN is ON */
            GPIO_ResetBits(LEDGK_Port, LEDGK_Pin);      /* activate GREEN cathode */
        } else {
            GPIO_SetBits(LEDGK_Port, LEDGK_Pin);            /* deactivate GREEN cathode */
        }

        if (leds_st[iled].bi & imask) {
            /* BLUE is ON */
            GPIO_ResetBits(LEDBK_Port, LEDBK_Pin);      /* activate BLUE cathode */
        } else {
            GPIO_SetBits(LEDBK_Port, LEDBK_Pin);            /* deactivate BLUE cathode */
        }


        /* activate anode of the correct LED (iled) */
        if (iled == 0) {
            GPIO_SetBits(LED1A_Port, LED1A_Pin);
        } else if (iled == 1) {
            GPIO_SetBits(LED2A_Port, LED2A_Pin);
        } else {
            GPIO_SetBits(LED3A_Port, LED3A_Pin);
        }

        /* let the LEDs shine! */
        vTaskDelay(( ( TickType_t ) LED_PERIOD_MS / portTICK_PERIOD_MS ));

        /* deactivate all anodes */
        GPIO_ResetBits(LED1A_Port, LED1A_Pin);
        GPIO_ResetBits(LED2A_Port, LED2A_Pin);
        GPIO_ResetBits(LED3A_Port, LED3A_Pin);

        if (++iled > 2) {
            iled = 0;
            ibit = (ibit + 1) % 8;
        }


        int iled_chng;
        if (xQueueReceive(toLEDsQueue, &iled_chng, 0) == pdTRUE) {
            /* LED state changed! */
            /* update the time of LEDs last change */
            leds_last_upd_tm = xTaskGetTickCount();
            /* reset to normal timeout */
            leds_timeout = (LEDS_ON_TIMEOUT / portTICK_PERIOD_MS);
        } else {
            /* LEDs not changed. For how long did we received nothing? */
            TickType_t diff_nochange_tm = (xTaskGetTickCount() - leds_last_upd_tm);

            if (diff_nochange_tm > leds_timeout) {
                /* LEDs off */
                GPIO_SetBits(LEDRK_Port, LEDRK_Pin);            /* deactivate RED cathode */
                GPIO_SetBits(LEDGK_Port, LEDGK_Pin);            /* deactivate GREEN cathode */
                GPIO_SetBits(LEDBK_Port, LEDBK_Pin);            /* deactivate BLUE cathode */

                /* wait until new LED state is received */

                if (xQueueReceive(toLEDsQueue, &iled_chng, (LEDS_STAY_OFF_TM / portTICK_PERIOD_MS)) == pdTRUE) {
                    /* LED state changed! */
                    /* reset timeout */
                    leds_timeout = (LEDS_ON_TIMEOUT / portTICK_PERIOD_MS);
                } else {
                    /* no change; Light-up for just a short time to remember the last state. */
                    leds_timeout = (LEDS_REMEMBER_ON_TM / portTICK_PERIOD_MS);
                }
                
                leds_last_upd_tm = xTaskGetTickCount();
            }
        }

    } while (1);
}

void LEDs_AllOff()
{
    for (int i = 0; i < 3; ++i) {
        leds_st[i].ri = 0;
        leds_st[i].gi = 0;
        leds_st[i].bi = 0;
    }
}

void LEDs_Set(int iled, uint8_t ir, uint8_t ig, uint8_t ib)
{
    if (iled >= 0 && iled <= 2) {
        leds_st[iled].ri = ir;
        leds_st[iled].gi = ig;
        leds_st[iled].bi = ib;
    }

    xQueueSend(toLEDsQueue, &iled, 0);
}
