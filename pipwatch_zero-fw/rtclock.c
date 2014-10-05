#include "rtclock.h"
#include "main.h"

/* Scheduler includes. */
// #include "FreeRTOS.h"
#include "task.h"
// #include "queue.h"

/* Library includes. */
#include "stm32f10x_it.h"



/* Global variables */

rtclock_t current_rtime;

// extern QueueHandle_t toDisplayStrQueue;


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

void draw_clock_face(unsigned int hours, unsigned int minutes, 
    int center_x, int center_y, int radius, u8g_t *u8g)
{
    /* face */
    u8g_DrawCircle(u8g, center_x, center_y, radius, U8G_DRAW_ALL);
    u8g_DrawCircle(u8g, center_x, center_y, radius+1, U8G_DRAW_ALL);

    /* hour markers */
    for (int h = 0; h < 12; ++h) {
        u8g_DrawCircle(u8g, 
            sintab60[absrot60(h*5+15)] + center_x, 
            sintab60[h*5] + center_y, 2, U8G_DRAW_ALL);
    }

    hours = (hours % 12);
    minutes = minutes % 60;

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

        char *buf = NULL;
        xQueueSendFromISR(toDisplayStrQueue, &buf, &xHigherPriorityTaskWoken);
    }

    /* clear RTC irq pending bit of the 'seconds' irq */
    RTC_ClearITPendingBit(RTC_IT_SEC);

    /* signal end-of-irq and possible reschedule point */
    portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
}
