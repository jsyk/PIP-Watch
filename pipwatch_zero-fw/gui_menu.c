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
    }
    return m;
}


/* drawing callback for menu */
int gui_menu_draw_cb(u8g_t *u8g, struct guiwindow *win,
                struct guipoint abspos)
{
    struct guimenu *menu = (struct guimenu *)win;

    

    
    return 0;
}
