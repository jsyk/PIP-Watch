#ifndef GUI_H
#define GUI_H

#include "gui_window.h"
#include "FreeRTOS.h"
#include "queue.h"


#define CFACE_CENTER_X      136
#define CFACE_CENTER_Y      36
#define CFACE_RADIUS        34


#define TERM_BUFLINES       8
#define TERM_VISLINES       6


/* for g_evnt in struct guievent: */
#define GUI_E_NONE              0           /* invalid */
#define GUI_E_PRINTSTR          1           /* print string to display */
#define GUI_E_BATT              2           /* battery state updated - see global variables */
#define GUI_E_BUTTON            3           /* button pressed or released */
#define GUI_E_CLOCK             4           /* clock updated - see global variable */
#define GUI_E_NEWSMS            5           /* new sms */

/* GUI Event message structure sent over the toGuiQueue */
struct guievent {
    /* mandatory: event type */
    int evnt;
    /* optional: ptr to buffer */
    void *buf;
    /* optional: int argument */
    int kpar;
};


/* ------------------------------------------------------------------ */

/* queue of struct guievent for the GUI task */
extern QueueHandle_t toGuiQueue;


/* ------------------------------------------------------------------ */
/* interface  functions */

/* RTOS task: GUI thread */
void GuiDrawTask(void *pvParameters);

void printstr(const char *buf);
void printstrn(const char *buf, int cnt);


#endif
