#include "gui.h"
/* Standard includes. */
#include <string.h>
/* Scheduler includes. */
#include "task.h"


#include "sms.h"
#include "epd.h"
#include "rtclock.h"
#include "battery.h"
#include "buttons.h"
#include "utils.h"

/* --------------------------------------------------------------------------------- */
/* Global variables */

u8g_t u8g;

/* terminal text: array of pointers to strings (text lines) */
char *term_text[TERM_BUFLINES];


/* queue of struct guievent for the GUI task */
QueueHandle_t toGuiQueue = NULL;


/* --------------------------------------------------------------------------------- */


static void draw_battery(int percent, int px, int py)
{
    if (percent < 0) { percent = 0; }
    if (percent > 100) { percent = 100; }

    u8g_SetDefaultMidColor(&u8g);
    u8g_DrawBox(&u8g, px, py, percent / (100/BATTERY_WIDTH), BATTERY_HEIGHT);

    u8g_SetDefaultForegroundColor(&u8g);
    u8g_DrawFrame(&u8g, px, py, BATTERY_WIDTH, BATTERY_HEIGHT);
    u8g_DrawFrame(&u8g, px+BATTERY_WIDTH, py+2, 2, 4);

    u8g_SetFont(&u8g, u8g_font_helvR08);
    char s[16];
#if 0
    /* print percents */
    int k = itostr(s, 16, vbat_percent);
    s[k] = '%';
    s[k+1] = '\0';
#else
    /* print voltage */
    int vbat100 = vbat_measured / 10;
    if ((vbat_measured % 10) >= 5) { vbat100 += 1; }
    int k = itostr(s, 16, vbat100);
    memmove(s+2, s+1, k-1);
    s[1] = '.';
    s[k+1] = 'V';
    s[k+2] = batt_state;
    s[k+3] = '\0';
#endif
    u8g_DrawStr(&u8g,  px+BATTERY_WIDTH+4, py+BATTERY_HEIGHT, s);
}

static void draw(int show_clkface)
{
#define TXT_OFFS_Y      10
// #define TXT_LINESPC_Y   15
#define TXT_LINESPC_Y   10
    u8g_SetDefaultForegroundColor(&u8g);
    // u8g_SetFont(&u8g, u8g_font_helvR12);
    u8g_SetFont(&u8g, u8g_font_helvR08);

    int k = 1;
    for (int i = TERM_BUFLINES-TERM_VISLINES; i < TERM_BUFLINES; ++i) {
        if (term_text[i]) {
            u8g_DrawStr(&u8g,  0, TXT_OFFS_Y+TXT_LINESPC_Y*k, term_text[i]);
        }
        ++k;
    }


    /* clock face */
#define CFACE_CENTER_X      136
#define CFACE_CENTER_Y      36
#define CFACE_RADIUS        34
    if (show_clkface) {
        /* draw round clock face */
        draw_clock_face(current_rtime.hour, current_rtime.min,
            CFACE_CENTER_X, CFACE_CENTER_Y, CFACE_RADIUS, &u8g);
    } else {
        /* print clock as text in status bar */
        // u8g_SetFont(&u8g, u8g_font_helvR08);
        u8g_SetFont(&u8g, u8g_font_helvB08);
        char s[12];
        itostr_rjust(s, 2, current_rtime.hour, '0');
        s[2] = ':';
        itostr_rjust(s+3, 2, current_rtime.min, '0');
        s[5] = '\0';
        u8g_DrawStr(&u8g,  130, 8, s);
    }

    draw_battery(vbat_percent, 0, 0);

#if 1
    /* print temperature */
    char s[8];
    k = itostr(s, 8, temp_celsius);
    s[k] = 'o'; //0xB0;
    s[k+1] = 'C';
    s[k+2] = '\0';
    u8g_SetFont(&u8g, u8g_font_helvR08);
    u8g_DrawStr(&u8g,  70, 8, s);
#endif
}

void term_add_line(char *str)
{
    /* scroll terminal text by one line up */
    if (term_text[0]) {
        vPortFree(term_text[0]);
        term_text[0] = NULL;
    }
    for (int i = 1; i < TERM_BUFLINES; ++i) {
        term_text[i-1] = term_text[i];
    }
    term_text[TERM_BUFLINES-1] = str;
}

void GuiDrawTask(void *pvParameters)
{
    u8g_InitComFn(&u8g, &u8g_dev_ssd1606_172x72_hw_spi, u8g_com_null_fn);
    u8g_SetDefaultForegroundColor(&u8g);

    for (int i = 0; i < TERM_BUFLINES; ++i) {
        term_text[i] = NULL;
    }

    struct guievent gevnt;
    int need_disp_refresh = 1;
    int show_clkface = 1;

    for (;;) {
        if (need_disp_refresh) {
            /* refresh display - picture loop */
            u8g_FirstPage(&u8g);
            do {
                draw(show_clkface);
            } while ( u8g_NextPage(&u8g) );

            need_disp_refresh = 0;
        }


        TickType_t maxwait = portMAX_DELAY;

        while (xQueueReceive(toGuiQueue, &gevnt, maxwait) == pdTRUE) {
            if ((gevnt.evnt == GUI_E_PRINTSTR) && gevnt.buf) {
#if 0
                for (int i = 0; i < 3; ++i) {
                    strncpy(hello_text[i], hello_text[i+1], 32);
                }
                strncpy(hello_text[3], gevnt.buf, 32);
#endif
                term_add_line(gevnt.buf);
                gevnt.buf = NULL;
                ++need_disp_refresh;
            }

            if ((gevnt.evnt == GUI_E_BATT) || (gevnt.evnt == GUI_E_CLOCK)) {
                ++need_disp_refresh;
            }

            if (gevnt.evnt == GUI_E_BUTTON) {
                if ((gevnt.kpar & BTN_PRESSED) && ((gevnt.kpar & BTNx_M) == BTN1)) {
                    /* pressed the middle button */
                    show_clkface = !show_clkface;
                    ++need_disp_refresh;
                }
            }

            if (gevnt.evnt == GUI_E_NEWSMS) {
                struct smstext *sms = gevnt.buf;
                char *s = newstrn("New SMS: ", 15+strlen(sms->sender_phone));
                if (sms->sender_phone != NULL) {
                    strcat(s, sms->sender_phone);
                }
                term_add_line(s);
                if (sms->text != NULL) {
                    term_add_line(sms->text);       // TODO: wrap
                }

                vPortFree(sms->sender_phone);
                vPortFree(sms);

                ++need_disp_refresh;
            }

            /* just in case - try to get more from the queue, otherwise go to redraw phase */
            maxwait = 1;
        }

    }  
}


/*
 * Print string buffer to the display.
 */
void printstrn(const char *buf, int cnt)
{
    char *zbuf = pvPortMalloc(sizeof(char) * (cnt+1));
    strncpy(zbuf, buf, cnt+1);
    for (int i = 0; i < cnt; ++i) {
        if (zbuf[i] < 32 && zbuf[i] != 0) {
            zbuf[i] = ' ';
        }
    }
    zbuf[cnt] = 0;

    struct guievent gevnt;
    gevnt.evnt = GUI_E_PRINTSTR;
    gevnt.buf = zbuf;
    gevnt.kpar = 0;
    xQueueSend(toGuiQueue, &gevnt, 0);
}

void printstr(const char *buf)
{
    int cnt = strlen(buf);
    printstrn(buf, cnt);
}

void clearterm(void)
{
    for (int i = 0; i < TERM_BUFLINES; ++i) {
        if (term_text[i]) {
            vPortFree(term_text[i]);
        }
        term_text[i] = NULL;
    }
}

int screentextsplit(const char *buf, int buflen, int pixwidth,
                    unsigned int *lnlens, int cnt)
{
    const char *bufend = buf + buflen;
    
    int minus_w = u8g_GetGlyph(&u8g, '-');
    int k = 0;
    int lnw = 0;

    while ((buf < bufend) && (k < cnt)) {
        char c = *buf;      // current character

        if (c == 0) {
            /* end of string */
            break;
        } else if (c == '\n') {
            /* newline is special */
            lnlens[k++] = lnw;
            lnw = 0;
        } else if (c < 32) {
            /* replace control chars by space */
            c = ' ';
        }

        int w = u8g_GetGlyph(&u8g, c);
    }
}
