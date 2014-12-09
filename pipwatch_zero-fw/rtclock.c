#include "rtclock.h"
#include "main.h"
#include "gui.h"

/* Scheduler includes. */
// #include "FreeRTOS.h"
#include "task.h"
// #include "queue.h"
#include <string.h>

/* Library includes. */
#include "stm32f10x_it.h"



/* Global variables */

rtclock_t current_rtime;



#define SINTAB_MULT         34
#define SINTAB_QCNT         15


const short sintab60[60] = {
    // round(sin(2*pi/60 * 0:59) * 34)
    0,   4,   7,  11,  14,  17,  20,  23,  25,  28,  29,  31,  32,  33,  34,  34,  34,  33,  32,
    31,  29,  28,  25,  23,  20,  17,  14,  11,   7,   4,   0,  -4,  -7, -11, -14, -17, -20, -23,
    -25, -28, -29, -31, -32, -33, -34, -34, -34, -33, -32, -31, -29, -28, -25, -23, -20, -17, -14,
    -11,  -7,  -4
};


int absrot60(int angle)
{
    while (angle < 0) { angle += 60; }
    while (angle >= 60) { angle -= 60; }
    return angle;
}

/* allocate new guiclockface */
struct GuiClockface *gui_clockface_alloc(void)
{
    struct GuiClockface *f =  pvPortMalloc(sizeof(struct GuiClockface));
    if (f) {
        memset(f, 0, sizeof(struct GuiClockface));
        f->win.draw_window_fn = gui_draw_clock_face_cb;
    }
    return f;
}

/* callback to draw the clock face */
int gui_draw_clock_face_cb(u8g_t *u8g, struct GuiWindow *win,
                struct GuiPoint abspos)
{
    struct GuiClockface *f = (struct GuiClockface *)win;

    int center_x = f->center_x + abspos.x;
    int center_y = f->center_y + abspos.y;

    /* clear background to white */
    u8g_SetDefaultBackgroundColor(u8g);
    u8g_DrawBox(u8g, center_x-f->radius, center_y-f->radius, 2*f->radius, 2*f->radius);
    u8g_SetDefaultForegroundColor(u8g);

    /* face */
    u8g_DrawCircle(u8g, center_x, center_y, f->radius, U8G_DRAW_ALL);
    u8g_DrawCircle(u8g, center_x, center_y, f->radius+1, U8G_DRAW_ALL);

    /* hour markers */
    for (int h = 0; h < 12; ++h) {
        u8g_DrawCircle(u8g, 
            sintab60[absrot60(h*5+15)] + center_x, 
            sintab60[h*5] + center_y, 2, U8G_DRAW_ALL);
    }

    int hours = (f->hours % 12);
    int minutes = f->minutes % 60;

    /* hours hand */
    int angle = absrot60(-(short)hours*5 - (short)minutes/12 + 15);
    int x2 = sintab60[absrot60(angle+15)]/2 + center_x;   // cos, x-axis is natural direction
    int y2 = (-sintab60[angle]/2) + center_y;         // sin, y-axis is inverted

    u8g_DrawLine(u8g, center_x, center_y, x2, y2);
    u8g_DrawLine(u8g, center_x+1, center_y, x2+1, y2);
    u8g_DrawLine(u8g, center_x, center_y+1, x2, y2+1);
    u8g_DrawLine(u8g, center_x-1, center_y, x2-1, y2);
    u8g_DrawLine(u8g, center_x, center_y-1, x2, y2-1);

    /* minutes hand */
    angle = absrot60(-(short)minutes + 15);
    x2 = sintab60[absrot60(angle+15)]*30/34 + center_x;   // cos, x-axis is natural direction
    y2 = (-sintab60[angle])*30/34 + center_y;         // sin, y-axis is inverted

    u8g_DrawLine(u8g, center_x, center_y, x2, y2);

    return 0;
}

void RTC_IRQHandler(void)
{
    long xHigherPriorityTaskWoken = pdFALSE;

    current_rtime.sec += 1;
    if (current_rtime.sec >= 60) {
        current_rtime.sec -= 60;
        current_rtime.min += 1;
        if (current_rtime.min >= 60) {
            current_rtime.min -= 60;
            current_rtime.hour += 1;
            if (current_rtime.hour >= 24) {
                current_rtime.hour -= 24;
            }
        }

        struct guievent gevnt;
        gevnt.evnt = GUI_E_CLOCK;
        gevnt.buf = NULL;
        gevnt.kpar = 0;
        xQueueSendFromISR(toGuiQueue, &gevnt, &xHigherPriorityTaskWoken);
    }

    /* clear RTC irq pending bit of the 'seconds' irq */
    RTC_ClearITPendingBit(RTC_IT_SEC);

    /* signal end-of-irq and possible reschedule point */
    portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
}
