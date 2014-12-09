#include "gui_menu.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>


/* allocate new menu gui element */
struct GuiMenu *gui_menu_alloc(int n_items)
{
    struct GuiMenu *m = pvPortMalloc(sizeof(struct GuiMenu));
    if (m) {
        memset(m, 0, sizeof(struct GuiMenu));
        textlines_init(&m->items, n_items);
        m->win.draw_window_fn = gui_menu_draw_cb;
    }
    return m;
}


static const char *menutext[] = {
    "Lorem",
    "Ipsum",
    "Dolor",
    "Sit",
    "Amet",
    "Consecteur",
    "Adipiscing"
};

/* drawing callback for menu */
int gui_menu_draw_cb(u8g_t *u8g, struct GuiWindow *win,
                struct GuiPoint abspos)
{
    struct GuiMenu *menu = (struct GuiMenu *)win;

    int y0 = menu->selected*10;

    u8g_SetDefaultBackgroundColor(u8g);
    u8g_DrawBox(u8g, abspos.x, y0, abspos.x+10, abspos.y+70);

    // u8g_SetDefaultMidColor(u8g);
    u8g_SetDefaultForegroundColor(u8g);
    u8g_DrawTriangle(u8g, abspos.x, y0,
            abspos.x+10, y0+5,
            abspos.x, y0+10);

    u8g_SetDefaultForegroundColor(u8g);

    for (int i = 0; i < 7; ++i) {
        y0 = (i+1)*10;
        const char *s = menutext[i];

        if (i == menu->selected) {
            u8g_SetFont(u8g, u8g_font_helvB08);
        } else {
            u8g_SetFont(u8g, u8g_font_helvR08);
        }
        u8g_DrawStr(u8g,  abspos.x+12, y0, s);
    }

    return 0;
}
