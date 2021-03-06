#ifndef GUI_MENU_H
#define GUI_MENU_H

#include "gui_window.h"
#include "gui_textbox.h"
#include "utils.h"


/* GUI Menu */
struct GuiMenu {
    /* the window base class; MUST BE THE FIRST ELEMENT */
    struct GuiWindow win;
    /* menu items */
    struct TextLines items;
    /* scroll offset */
    int scrolloffs;
    /* selected item index; -1 if none */
    int selected;
    /* menu font */
    struct fontstyle font;
    /* */
    // int 
};

/* allocate new menu gui element */
struct GuiMenu *gui_menu_alloc(int n_items);


/* drawing callback for menu */
int gui_menu_draw_cb(u8g_t *u8g, struct GuiWindow *win,
                struct GuiPoint abspos);

#endif
