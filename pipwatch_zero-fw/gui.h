#ifndef GUI_H
#define GUI_H

#include <u8g.h>
#include "FreeRTOS.h"
#include "queue.h"

/* battery symbol graphical dimensions */
#define BATTERY_WIDTH       20
#define BATTERY_HEIGHT      8


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


struct guipoint {
    int x, y;
};

struct guiwindow;

/* Callback to draw the window win onto device pu8g.
 * abspos is absolute position of the window. */
typedef int (*gui_draw_window_fn_t)(u8g_t *pu8g, struct guiwindow *win,
                struct guipoint abspos);


/* GUI Window base class.
 */
struct guiwindow {
    /* parent window */
    struct guiwindow *parent;
    /* offset relative to parent's origin */
    struct guipoint offs;       // TODO This assumes absolute position; maybe put to a layout container?
    /* width and height in pixels */
    struct guipoint size;

    gui_draw_window_fn_t draw_window_fn;
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
