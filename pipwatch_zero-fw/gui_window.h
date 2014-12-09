#ifndef GUI_WINDOW_H
#define GUI_WINDOW_H

#include <u8g.h>

/* Location on screen */
struct GuiPoint {
    int x, y;
};

struct GuiWindow;

/* Callback function type to draw the window win onto device pu8g.
 * abspos is absolute position of the window. */
typedef int (*gui_draw_window_fn_t)(u8g_t *u8g, struct GuiWindow *win,
                struct GuiPoint abspos);

/* Callback function type tp react for button events.
 * btnev is a combination of BTNx and BTN_{DOWN, UP, SHORTCLICK, LONGCLICK}.
 * Set updscreen to nonzero if redraw is needed.
 * Return nonzero if the event has been procesed and should not be propagated more. */
typedef int (*gui_btnevent_fn_t)(struct GuiWindow *win, int btnev, int *updscreen);

/* GUI Window base class.
 */
struct GuiWindow {
    /* parent window */
    struct GuiWindow *parent;
    /* offset relative to parent's origin */
    struct GuiPoint offs;       // TODO This assumes absolute position; maybe put to a layout container?
    /* width and height in pixels */
    struct GuiPoint size;

    /* callback function: draw the window on screen */
    gui_draw_window_fn_t draw_window_fn;
    /* callback function: button event */
    gui_btnevent_fn_t btnevent_fn;
};



#endif
