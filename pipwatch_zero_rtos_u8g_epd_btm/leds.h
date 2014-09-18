#ifndef LEDS_H
#define LEDS_H

#include <stdint.h>
#include "stm32f10x_gpio.h"

/* cathodes */
#define LEDRK_Port      GPIOC
#define LEDRK_Pin       GPIO_Pin_10

#define LEDGK_Port      GPIOC
#define LEDGK_Pin       GPIO_Pin_11

#define LEDBK_Port      GPIOC
#define LEDBK_Pin       GPIO_Pin_12

/* anodes */
#define LED1A_Port      GPIOD
#define LED1A_Pin       GPIO_Pin_2

#define LED2A_Port      GPIOB
#define LED2A_Pin       GPIO_Pin_8

#define LED3A_Port      GPIOB
#define LED3A_Pin       GPIO_Pin_9



#define LED_INTENS_0        0x00        /* 0000_0000 */
#define LED_INTENS_25       0x11        /* 0001_0001 */
#define LED_INTENS_50       0x44        /* 0101_0101 */
#define LED_INTENS_75       0x77        /* 0111_0111 */
#define LED_INTENS_100      0xFF        /* 1111_1111 */


struct led_state_t {
    /* colour intensity using LED_INTENS_* */
    uint8_t ri, gi, bi;
};


#define     LED0        0
#define     LED1        1
#define     LED2        2


void LEDs_Init(void);
void LEDsTask(void *pvParameters);
void LEDs_AllOff();

void LEDs_Set(int iled, uint8_t ir, uint8_t ig, uint8_t ib);

#endif
