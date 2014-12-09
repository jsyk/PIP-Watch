#include "gui_textbox.h"

#include "FreeRTOS.h"
#include "task.h"

#include <string.h>
#include <ctype.h>

static int prs_echar(struct GuiTextbox *tbox, struct TextLines_iterator *txtit, int *ch);
static int prs_esccode(struct GuiTextbox *tbox, struct TextLines_iterator *txtit, int *ch);
static int prs_token(struct GuiTextbox *tbox, struct TextLines_iterator *txtit, char *strtoken, int maxlen);
static int prs_tokname(struct GuiTextbox *tbox, struct TextLines_iterator *txtit, char *strtoken, int maxlen);


/* init default font style */
void fontstyle_init(struct fontstyle *fs)
{
    memset(fs, 0, sizeof(struct fontstyle));
    fs->font_size = 8;
    fs->font_family = FF_HELV;
    fs->extra_lnspc = 2;
    fs->color = COLOR_FOREGR;
}


void fontstyle_activate(struct fontstyle *fs, u8g_t *u8g)
{
    const u8g_fntpgm_uint8_t *font = u8g_font_helvR08;

    if (fs->bold) {
        if (fs->font_family == FF_HELV) {
            if (fs->font_size == 8)
                font = u8g_font_helvB08;
        }
    } else {
        if (fs->font_family == FF_HELV) {
            if (fs->font_size == 8)
                font = u8g_font_helvR08;
        }

    }

    u8g_SetFont(u8g, font);
}


/* ---------------------------------------------------------------------- */

/* allocate new textbox gui element */
struct GuiTextbox *gui_textbox_alloc(int nlines)
{
    struct GuiTextbox *tbox = pvPortMalloc(sizeof(struct GuiTextbox));
    if (tbox != NULL) {
        memset(tbox, 0, sizeof(struct GuiTextbox));
        tbox->win.draw_window_fn = gui_textbox_draw_cb;
        textlines_init(&tbox->txt, nlines);
        fontstyle_init(&tbox->ini_fontstyle);
    }
    return tbox;
}

/* drawing callback for textbox */
int gui_textbox_draw_cb(u8g_t *u8g, struct GuiWindow *win,
                struct GuiPoint abspos)
{
    struct GuiTextbox *tbox = (struct GuiTextbox *)win;

    /* textlines reader at the beginning */
    struct TextLines_iterator txtit;
    textlines_iterator_init(&txtit, &tbox->txt);

    char *stok = pvPortMalloc(sizeof(char) * 32);
    struct fontstyle fs = tbox->ini_fontstyle;
    int px = abspos.x;
    int py = abspos.y + fs.font_size + fs.extra_lnspc;
    int fs_changed = 1;

    do {
        int c = prs_token(tbox, &txtit, stok, 32);
        if (c < 0) {
            /* end of iterator */
            break;
        }

        c = stok[0];

        if (c == '<') {
            if (stok[1] != '<') {
                c = 0;

                if (strncmp(stok, "<b", 2) == 0) {
                    /* inc bold style */
                    fs.bold++;
                    fs_changed++;
                } else if (strncmp(stok, "</b", 3) == 0) {
                    /* dec bold style */
                    if (--fs.bold < 0) {
                        fs.bold = 0;
                    }
                    fs_changed++;
                } 
            }
        } else if (c == 0 || c == '\n') {
            /* '\0' or '\n' are new lines */
            /* TBD decode special handling of lf */
            px = abspos.x;
            py += fs.font_size + fs.extra_lnspc;
            c = 0;
        }

        if (c != 0) {
            /* print character using the current style */
            if (fs_changed) {
                fontstyle_activate(&fs, u8g);
                fs_changed = 0;
            }

            /* check if visible */
            if ((px < abspos.x + tbox->win.size.x) 
                    && (py-(fs.font_size+fs.extra_lnspc) < abspos.y + tbox->win.size.y)) {
                /* draw glyph */
                u8g_DrawGlyph(u8g,  px, py, c);
            }
            /* advance x position by glyph width */
            px += u8g_GetGlyphDeltaX(u8g, c);
            if (tbox->wraplines && px > abspos.x + tbox->win.size.x) {
                px = abspos.x;
                py += fs.font_size + fs.extra_lnspc;
            }
        }

    } while (textlines_iterator_peekc(&txtit) != -1);

    vPortFree(stok);
    return 0;
}


/*
    <echar> ::=
        . normal char except '\'
        | '\' <esccode>
*/
static int prs_echar(struct GuiTextbox *tbox, struct TextLines_iterator *txtit, int *ch)
{
    *ch = textlines_iterator_peekc(txtit);
    textlines_iterator_next(txtit);

    switch (*ch) {
        case '\\':
            if (tbox->escexpand) {
                return prs_esccode(tbox, txtit, ch);
            }
            /* else fallthrough */
        default:
            return 0;
    }
}

/*
    <esccode> ::=
        . 'n' | '\' | '"' | {'u' <unicode_seq>}
*/
static int prs_esccode(struct GuiTextbox *tbox, struct TextLines_iterator *txtit, int *ch)
{
    *ch = textlines_iterator_peekc(txtit);
    textlines_iterator_next(txtit);

    switch (*ch) {
        case 'n':
            *ch = '\n';
            return 0;
        
        case 'u':
            /* hexcode */
            // TODO
            return 0;
        
        case '\\':
        case '"':
        default:
            /* no transform, just the character itself (ignore the backslash) */
            return 0;
    }
}

/*
    <token> ::=
        {'<' '<'}
        | '<' ['/'] <tokname>
        | <echar\{'<'}>
*/
static int prs_token(struct GuiTextbox *tbox, struct TextLines_iterator *txtit,
        char *strtoken, int maxlen)
{
    int ch;
    prs_echar(tbox, txtit, &ch);
    strtoken[0] = ch;
    if (ch < 0)
        return -1;

    if ((ch != '<') || (!tbox->markup)) {
        /* just the character is the token */
        strtoken[1] = '\0';
        return 0;
    }

    prs_echar(tbox, txtit, &ch);
    strtoken[1] = ch;
    if (ch < 0)
        return -1;

    switch (ch) {
        case '>':
            /* empty token? fallthrough */
        case '<':
            /* escaped < */
            strtoken[2] = '\0';
            return 0;
        case '/':
            /* end of token token; fallthrough */
        default:
            /* start of token token */
            return prs_tokname(tbox, txtit, &strtoken[2], maxlen-2);
    }
}

/*
    <tokname> ::=
        { <echar\{'<','>'}> * } '>'
*/
static int prs_tokname(struct GuiTextbox *tbox, struct TextLines_iterator *txtit,
        char *strtoken, int maxlen)
{
    int ch;
    do {
        prs_echar(tbox, txtit, &ch);
        if (ch < 0)
            return -1;

        switch (ch) {
            case '<':
                /* error  */
                *strtoken++ = '\0';
                return 1;
            case '>':
                /* end of token */
                *strtoken++ = ch;
                *strtoken++ = '\0';
                return 0;
            default:
                *strtoken++ = tolower(ch);
                break;
        }

    } while (--maxlen > 0);
    *strtoken = '\0';
    return 1;
}
