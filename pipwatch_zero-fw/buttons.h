#ifndef BUTTONS_H
#define BUTTONS_H

#include "FreeRTOS.h"


struct button_state_t {
	/* current button state: 0 (released) or 1 (pressed) */
	int st;
	/* the cpu-tick when the current state was put down */
	TickType_t tick;
};


/* buttons identification */
#define BTN0		0
#define BTN1		1
#define BTN2		2

#define BTNx_M      0xFF

/* button event */
#define BTN_PRESSED     0x100
#define BTN_RELEASED    0x200

/* hardware ports and pins */
#define BTN0_Port	GPIOB
#define BTN1_Port	GPIOB
#define BTN2_Port	GPIOB

#define BTN0_Pin	GPIO_Pin_5
#define BTN1_Pin	GPIO_Pin_6
#define BTN2_Pin	GPIO_Pin_7


/* dead time in milliseconds */
#define BUTTON_DEAD_TM		20


/* current button state */
extern struct button_state_t btnsts[3];


void EXTI9_5_IRQHandler(void);


void ButtonsTask(void *pvParameters);


#endif
