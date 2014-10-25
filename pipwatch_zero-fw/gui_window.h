#ifndef GUI_WINDOW_H
#define GUI_WINDOW_H

#include <u8g.h>

struct guipoint {
    int x, y;
};

struct guiwindow;

/* Callback to draw the window win onto device pu8g.
 * abspos is absolute position of the window. */
typedef int (*gui_draw_window_fn_t)(u8g_t *u8g, struct guiwindow *win,
                struct guipoint abspos);


/* GUI Window base class.
 */
struct guiwindow {
    /* parent window */
    struct guiwindow *parent;
    /* offset relative to parent's origin */
    struct guipoint offs;       // TODO This assumes absolute position; maybe put to a layout container?
    /* width and height in pixels */
    struct guipoint size;

    gui_draw_window_fn_t draw_window_fn;
};



#endif
