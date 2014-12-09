#ifndef BUTTONS_H
#define BUTTONS_H

#include "FreeRTOS.h"

/* hardware button state */
struct button_state_t {
	/* STATE WRITTEN IN IRQ HANDLER */
	/* current button state: 0 (up, released) or 1 (down, pressed) */
	int st;
	/* the cpu-tick time when the current state (up or down) was recorded */
	TickType_t tick;

	/* STATE UPDATED IN OS TASK */
	/* the cpu-tick time when the button was last pressed;
	 * used for detecting short and long button clicks in ButtonTask */
	TickType_t kdwn_tick;
	/* the cpu-tick time when the event BTN_LONGCLICK was last sent */
	TickType_t longclick_sent_tick;
};


/* buttons identification */
#define BTN0		0
#define BTN1		1
#define BTN2		2

/* mask for BTNx */
#define BTNx_M      0xFF

/* button event */
#define BTN_DOWN    		0x100
#define BTN_UP    			0x200
#define BTN_SHORTCLICK		0x400
#define BTN_LONGCLICK		0x800


/* hardware ports and pins */
#define HW_BTN0_Port	GPIOB
#define HW_BTN1_Port	GPIOB
#define HW_BTN2_Port	GPIOB

#define HW_BTN0_Pin	GPIO_Pin_5
#define HW_BTN1_Pin	GPIO_Pin_6
#define HW_BTN2_Pin	GPIO_Pin_7


/* dead time in milliseconds - supress hardware glitches */
#define BUTTON_DEAD_TM				10

/* max short click time in miliseconds */
#define BUTTON_SHORTCLICK_TM		1000
/* repetition time of BTN_LONGCLICK events once long-click is recognized */
#define BUTTON_LONGCLICK_REPE_TM	800


/* current button state */
extern struct button_state_t btnsts[3];


/* IRQ handler for button state change */
void EXTI9_5_IRQHandler(void);

/* OS task handling buttons events */
void ButtonsTask(void *pvParameters);


#endif
