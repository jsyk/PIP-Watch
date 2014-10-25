#ifndef RTCLOCK_H
#define RTCLOCK_H

#include "gui_window.h"


struct guiclockface {
    /* the window base class; MUST BE THE FIRST ELEMENT */
    struct guiwindow win;

    /* the time to show */
    unsigned int hours;
    unsigned int minutes;

    /* clock-face position and radius */
    int center_x; 
    int center_y;
    int radius;
};

/* callback to draw the clock face */
int gui_draw_clock_face_cb(u8g_t *u8g, struct guiwindow *win,
                struct guipoint abspos);

/* allocate new guiclockface */
struct guiclockface *gui_clockface_alloc(void);


/* ----------------------------------------------------------------- */

typedef struct {
    short sec;
    short min;
    short hour;
} rtclock_t;

extern rtclock_t current_rtime;


void RTC_IRQHandler(void);

#endif
