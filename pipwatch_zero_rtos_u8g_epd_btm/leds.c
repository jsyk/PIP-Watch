#include "leds.h"
#include "main.h"
#include "rtclock.h"
#include "utils.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#define LED_PERIOD_MS       1


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
}
