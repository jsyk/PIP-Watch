#ifndef RTCLOCK_H
#define RTCLOCK_H

#include <u8g.h>

typedef struct {
    short sec;
    short min;
    short hour;
} rtclock_t;

extern rtclock_t current_rtime;


void draw_clock_face(unsigned int hours, unsigned int minutes, int center_x, int center_y, int radius, u8g_t *u8g);

void RTC_IRQHandler(void);

#endif
