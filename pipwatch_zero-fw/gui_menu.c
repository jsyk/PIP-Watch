#include "gui_menu.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>


/* allocate new menu gui element */
struct guimenu *gui_menu_alloc(int n_items)
{
    struct guimenu *m = pvPortMalloc(sizeof(struct guimenu));
    if (m) {
        memset(m, 0, sizeof(struct guimenu));
        textlines_init(&m->items, n_items);
        m->win.draw_window_fn = gui_menu_draw_cb;
    }
    return m;
}


/* drawing callback for menu */
int gui_menu_draw_cb(u8g_t *u8g, struct guiwindow *win,
                struct guipoint abspos)
{
    struct guimenu *menu = (struct guimenu *)win;

    int y0 = menu->selected*10;

    u8g_SetDefaultBackgroundColor(u8g);
    u8g_DrawBox(u8g, abspos.x, y0, abspos.x+10, abspos.y+70);

    // u8g_SetDefaultMidColor(u8g);
    u8g_SetDefaultForegroundColor(u8g);
    u8g_DrawTriangle(u8g, abspos.x, y0,
            abspos.x+10, y0+5,
            abspos.x, y0+10);

    
    return 0;
}
