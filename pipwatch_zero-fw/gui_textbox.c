#include "gui_textbox.h"

#include "FreeRTOS.h"
#include "task.h"

#include <string.h>
#include <ctype.h>

static int prs_echar(struct guitextbox *tbox, struct textlines_iterator *txtit, int *ch);
static int prs_esccode(struct guitextbox *tbox, struct textlines_iterator *txtit, int *ch);
static int prs_token(struct guitextbox *tbox, struct textlines_iterator *txtit, char *strtoken, int maxlen);
static int prs_tokname(struct guitextbox *tbox, struct textlines_iterator *txtit, char *strtoken, int maxlen);



/* initialize textline iterator to the beginning */
void textlines_iterator_init(struct textlines_iterator *it, char **textlines, int nlines)
{
    memset(it, 0, sizeof(struct textlines_iterator));
    it->textlines = textlines;
    it->nlines = nlines;
    /* find the first character */
    for (it->k = 0; it->k < it->nlines; it->k++) {
        if (it->textlines[it->k] != NULL) {
            it->m = 0;
            /* found a character! */
            return;
        }
    }
}

/* return the current character, or -1 if at the end */
int textlines_iterator_peekc(struct textlines_iterator *it)
{
    if (it->textlines == NULL || it->k >= it->nlines || it->k < 0)
        return -1;
    return (unsigned char)it->textlines[it->k][it->m];
}

/* advance to the next character; return -1 if at the end */
int textlines_iterator_next(struct textlines_iterator *it)
{
    if (it->textlines == NULL || it->k >= it->nlines || it->k < 0)
        return -1;

    if (it->textlines[it->k] != NULL) {
        if (it->textlines[it->k][it->m] != '\0') {
            /* not at the end of string, so just advance */
            it->m++;
            return 0;
        }
    }

    /* else at the end of string, skip to next line. */
    it->m = 0;
    it->k++;

    for ( ; it->k < it->nlines; it->k++) {
        if (it->textlines[it->k] != NULL) {
            return 0;
        }
    }

    return 1;
}


void stringlist_scroll_add(struct guitextbox *tbox, char *str)
{
    if (tbox->textlines) {
        if (tbox->textlines[0]) {
            vPortFree(tbox->textlines[0]);
            tbox->textlines[0] = NULL;
        }
        for (int i = 1; i < tbox->nlines; ++i) {
            tbox->textlines[i-1] = tbox->textlines[i];
        }
        tbox->textlines[tbox->nlines-1] = str;
    }
}


/* ---------------------------------------------------------------------- */

/* allocate new textbox gui element */
struct guitextbox *gui_textbox_alloc(int nlines)
{
    struct guitextbox *tbox = pvPortMalloc(sizeof(struct guitextbox));
    if (tbox != NULL) {
        memset(tbox, 0, sizeof(struct guitextbox));
        tbox->win.draw_window_fn = gui_textbox_draw_cb;
        
        if (nlines > 0) {
            tbox->nlines = nlines;
            tbox->textlines = pvPortMalloc(sizeof(char*) * nlines);
        }
    }
    return tbox;
}

/* drawing callback for textbox */
int gui_textbox_draw_cb(u8g_t *u8g, struct guiwindow *win,
                struct guipoint abspos)
{
    struct guitextbox *tbox = (struct guitextbox *)win;

    /* textlines reader at the beginning */
    struct textlines_iterator txtit;
    textlines_iterator_init(&txtit, tbox->textlines, tbox->nlines);

    char *stok = pvPortMalloc(sizeof(char) * 32);
    int px = abspos.x;
    int py = abspos.y + 10;

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
                } else if (strncmp(stok, "</b", 3) == 0) {
                    /* dec bold style */
                } 
            }
        } else if (c == 0 || c == '\n') {
            /* '\0' or '\n' are new lines */
            /* TBD decode special handling of lf */
            px = abspos.x;
            py += 10;       // TBD font height
            c = 0;
        }

        if (c != 0) {
            /* print character using the current style */
            if ((px < abspos.x + tbox->win.size.x) 
                    && (py-10 < abspos.y + tbox->win.size.y)) {
                u8g_DrawGlyph(u8g,  px, py, c);
            }
            /* advance x position by glyph width */
            px += u8g_GetGlyphDeltaX(u8g, c);
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
static int prs_echar(struct guitextbox *tbox, struct textlines_iterator *txtit, int *ch)
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
static int prs_esccode(struct guitextbox *tbox, struct textlines_iterator *txtit, int *ch)
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
static int prs_token(struct guitextbox *tbox, struct textlines_iterator *txtit,
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
static int prs_tokname(struct guitextbox *tbox, struct textlines_iterator *txtit,
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
