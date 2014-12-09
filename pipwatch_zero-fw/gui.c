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
#include "gui_textbox.h"
#include "gui_menu.h"


/* --------------------------------------------------------------------------------- */
/* Global variables */

u8g_t u8g;


/* queue of struct guievent for the GUI task */
QueueHandle_t toGuiQueue = NULL;


/* --------------------------------------------------------------------------------- */



static void draw(struct GuiClockface *clkface, struct GuiTextbox *term_tbox, struct GuiBattery *batt)
{
#define TXT_OFFS_Y      10
// #define TXT_LINESPC_Y   15
#define TXT_LINESPC_Y   10
    u8g_SetDefaultForegroundColor(&u8g);
    // u8g_SetFont(&u8g, u8g_font_helvR12);
    u8g_SetFont(&u8g, u8g_font_helvR08);

    struct GuiPoint apos;
    apos.x = 0;
    apos.y = TXT_OFFS_Y;
    term_tbox->win.draw_window_fn(&u8g, &term_tbox->win, apos);


    /* clock face */
    if (clkface) {
        /* draw round clock face */
        apos.x = CFACE_CENTER_X - CFACE_RADIUS;
        apos.y = CFACE_CENTER_Y - CFACE_RADIUS;
        clkface->win.draw_window_fn(&u8g, &clkface->win, apos);
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

    // draw_battery(vbat_percent, 0, 0);
    apos.x = 0;
    apos.y = 0;
    batt->vbat_measured = vbat_measured;
    batt->percent = vbat_percent;
    batt->batt_state = batt_state;
    batt->win.draw_window_fn(&u8g, &batt->win, apos);


#if 1
    /* print temperature */
    char s[8];
    int k = itostr(s, 8, temp_celsius);
    s[k] = 'o'; //0xB0;
    s[k+1] = 'C';
    s[k+2] = '\0';
    u8g_SetFont(&u8g, u8g_font_helvR08);
    u8g_DrawStr(&u8g,  70, 8, s);
#endif
}


void GuiDrawTask(void *pvParameters)
{
    u8g_InitComFn(&u8g, &u8g_dev_ssd1606_172x72_hw_spi, u8g_com_null_fn);
    u8g_SetDefaultForegroundColor(&u8g);

    struct GuiTextbox *term_tbox = gui_textbox_alloc(TERM_VISLINES);
    term_tbox->win.size.x = WIDTH;
    term_tbox->win.size.y = HEIGHT - 10;
    term_tbox->markup = 1;          /* expands <b>... */
    term_tbox->escexpand = 1;       /* expands "\\n" */
    term_tbox->wraplines = 1;       /* wraps long lines, otherwise they are trimmed */
    textlines_scroll_add(&term_tbox->txt, newstrn("<b>PIP-Watch ZERO</b>\n  fw1.0\\n  hw02", -1));
    textlines_scroll_add(&term_tbox->txt, newstrn("Lorem ipsum dolor re ipsum dolor alea dolor ypsum rea lorema.", -1));

    struct GuiClockface *clkface = gui_clockface_alloc();
    clkface->win.size.x = 2*CFACE_RADIUS;
    clkface->win.size.y = 2*CFACE_RADIUS;
    clkface->center_x = CFACE_RADIUS; //CFACE_CENTER_X;
    clkface->center_y = CFACE_RADIUS; //CFACE_CENTER_Y;
    clkface->radius = CFACE_RADIUS;

    struct GuiBattery *batt = gui_battery_alloc();
    batt->win.size.x = DEFAULT_BATTERY_WIDTH;
    batt->win.size.y = DEFAULT_BATTERY_HEIGHT;

    struct GuiMenu *menu = gui_menu_alloc(6);
    menu->win.size.x = 10;
    menu->win.size.y = 72;

    struct GuiSMS *sms = gui_sms_alloc();
    sms->txtbox->win.size.x = WIDTH;
    sms->txtbox->win.size.y = HEIGHT - 10;

    struct smstext *txt = sms_alloc();
    txt->sender_name = newstrn("Prvni Odesilatel", -1);
    txt->text = newstrn("Prvni textovka, lorem ipsum dolor sit", -1);
    gui_sms_add(sms, txt);
    
    txt = sms_alloc();
    txt->sender_name = newstrn("Druhy Odesilatel", -1);
    txt->text = newstrn("Druha textovka, lorem ipsum dolor sit", -1);
    gui_sms_add(sms, txt);


    struct guievent gevnt;
    int need_disp_refresh = 1;
    int show_clkface = 1;

    for (;;) {
        clkface->hours = current_rtime.hour;
        clkface->minutes = current_rtime.min;

        if (need_disp_refresh) {
            /* refresh display - picture loop */
            u8g_FirstPage(&u8g);
            do {
                draw((show_clkface ? clkface : NULL), term_tbox, batt);
                epd_updrange_x1 = 0;
                epd_updrange_x2 = WIDTH - 1;

#if 0
                struct guipoint abspos;
                abspos.x = WIDTH - 51;
                abspos.y = 0;
                menu->win.draw_window_fn(&u8g, &menu->win, abspos);

                epd_updrange_x1 = abspos.x;
                epd_updrange_x2 = abspos.x + 10;
#endif
            } while ( u8g_NextPage(&u8g) );

            need_disp_refresh = 0;
        }


        TickType_t maxwait = portMAX_DELAY;

        while (xQueueReceive(toGuiQueue, &gevnt, maxwait) == pdTRUE) {
            /* GUI event recevied */

            if ((gevnt.evnt == GUI_E_PRINTSTR) && gevnt.buf) {
                textlines_scroll_add(&term_tbox->txt, gevnt.buf);
                gevnt.buf = NULL;
                ++need_disp_refresh;
            }

            if ((gevnt.evnt == GUI_E_BATT) || (gevnt.evnt == GUI_E_CLOCK)) {
                ++need_disp_refresh;
            }

            if (gevnt.evnt == GUI_E_BUTTON) {
                if ((gevnt.kpar & BTN_DOWN) && ((gevnt.kpar & BTNx_M) == BTN1)) {
                    /* pressed the middle button */
                    show_clkface = !show_clkface;
                    ++need_disp_refresh;

                    if (--menu->selected < 0) menu->selected = 0;
                }

                if ((gevnt.kpar & BTN_DOWN) && ((gevnt.kpar & BTNx_M) == BTN2)) {
                    /* pressed the lower button */
                    if (++menu->selected > 6) menu->selected = 6;
                    ++need_disp_refresh;
                }                
            }

            if (gevnt.evnt == GUI_E_NEWSMS) {
                struct smstext *sms = gevnt.buf;
                char *s = newstrn("<b>New SMS:</b> ", 15+strlen(sms->sender_phone));
                if (sms->sender_phone != NULL) {
                    strcat(s, sms->sender_phone);
                }
                textlines_scroll_add(&term_tbox->txt, s);
                if (sms->text != NULL) {
                    char *txt = newstrn(sms->text, 35);
                    if (strlen(txt) > 32) {
                        strtrimn(txt, 3);
                        strcat(txt, "...");
                    }
                    textlines_scroll_add(&term_tbox->txt, txt);
                }

                sms_free(sms);
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
