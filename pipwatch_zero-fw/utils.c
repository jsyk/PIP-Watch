#include "utils.h"
// #include "u8g.h"
#include <string.h>

#include "FreeRTOS.h"


/* Integer to decimal string conversion.
 * Returns the number of characters written to buf, not counting the final \0.
 * Terminates the string with \0 if there is a space in the buffer.
 */
int itostr(char *buf, int buflen, int x)
{
    int cnt = 0;
    
    if (buflen <= 0) { return cnt; }

    if (x < 0) {
        buf[0] = '-';
        ++buf;
        --buflen;
        ++cnt;
        x = -x;
    }

    if (buflen <= 0) { return cnt; }

    char *beg = buf;

    do {
        char digit = (x % 10) + '0';
        buf[0] = digit;
        ++buf;
        ++cnt;
        --buflen;
        if (buflen <= 0) { return cnt; }
        x = x / 10;
    } while (x > 0);

    if (buflen > 0) {
        buf[0] = '\0';
    }
    
    --buf;

    while (beg < buf) {
        char ch = *beg;
        *beg = *buf;
        *buf = ch;
        ++beg;
        --buf;
    }

    return cnt;
}

/* Integer to decimal string conversion, right-justify to the buffer end, no \0 insertion,
 * fill unused chars with fillch.
 * Returns the number of digits written.
 */
int itostr_rjust(char *buf, int buflen, int x, char fillch)
{
    int cnt = 0;
    
    if (buflen <= 0) { return cnt; }

    if (x < 0) {
        buf[0] = '-';
        ++buf;
        --buflen;
        ++cnt;
        x = -x;
    }

    if (buflen <= 0) { return cnt; }

    char *beg = buf;
    buf = beg + buflen - 1;

    do {
        char digit = (x % 10) + '0';
        buf[0] = digit;
        --buf;
        ++cnt;
        --buflen;
        if (buflen <= 0) { return cnt; }
        x = x / 10;
    } while (x > 0);

    /* fill the rest */
    while (buf >= beg) {
        buf[0] = fillch;
        --buf;
    }

    return cnt;
}

/* Allocate new string and copy from buf up to n characters,
 * alloc 1 more for '\0'
 */
char *newstrn(const char *buf, int n)
{
    if (n <= 0) {
        n = strlen(buf);
    }
    char *s = pvPortMalloc((n + 1) * sizeof(char));
    if (s != NULL) {
        if (buf != NULL) {
            strncpy(s, buf, n);
            s[n] = '\0';
        } else {
            s[0] = '\0';
        }
    }
    return s;
}

/* Trim string by up to n characters from the end. */
char *strtrimn(char *buf, int n)
{
    if (buf != NULL) {
        int len = strlen(buf);
        if (n > len) {
            n = len;
        }
        buf[len - n] = '\0';
    }
    return buf;
}


/* initialize textline iterator to the beginning */
void textlines_iterator_init(struct TextLines_iterator *it, const struct TextLines *txt)
{
    memset(it, 0, sizeof(struct TextLines_iterator));
    it->txt = txt;
    /* find the first character */
    for (it->k = 0; it->k < it->txt->nlines; it->k++) {
        if (it->txt->textlines[it->k] != NULL) {
            it->m = 0;
            /* found a character! */
            return;
        }
    }
}

/* return the current character, or -1 if at the end */
int textlines_iterator_peekc(struct TextLines_iterator *it)
{
    const struct TextLines *txt = it->txt;
    if (txt->textlines == NULL || it->k >= txt->nlines || it->k < 0)
        return -1;
    return (unsigned char)txt->textlines[it->k][it->m];
}

/* advance to the next character; return -1 if at the end */
int textlines_iterator_next(struct TextLines_iterator *it)
{
    const struct TextLines *txt = it->txt;

    if (txt->textlines == NULL || it->k >= txt->nlines || it->k < 0)
        return -1;

    if (txt->textlines[it->k] != NULL) {
        if (txt->textlines[it->k][it->m] != '\0') {
            /* not at the end of string, so just advance */
            it->m++;
            return 0;
        }
    }

    /* else at the end of string, skip to next line. */
    it->m = 0;
    it->k++;

    for ( ; it->k < txt->nlines; it->k++) {
        if (txt->textlines[it->k] != NULL) {
            return 0;
        }
    }

    return 1;
}

/* initialize textlines */
void textlines_init(struct TextLines *txt, int nlines)
{
    memset(txt, 0, sizeof(struct TextLines));
    if (nlines > 0) {
        txt->textlines = pvPortMalloc(sizeof(char*) * nlines);
        memset(txt->textlines, 0, sizeof(char*) * nlines);
        txt->nlines = nlines;
    }
}


/* Append a new line to the textline, scrolling the textlines up if needed
 * The total number of lines does not exceed nlines. */
void textlines_scroll_add(struct TextLines *tbox, char *str)
{
    if (tbox->textlines) {
        /* find the last line from the back that is NULL */
        int k;
        for (k = tbox->nlines-1 ; k >= 0; k--) {
            if (tbox->textlines[k] != NULL) {
                break;
            }
        }
        if (k < 0) {
            /* stringlist is empty, so put str in the first line */
            k = 0;
        } else if (k == tbox->nlines-1) {
            /* stringlist is full, must scroll up */
            if (tbox->textlines[0]) {
                vPortFree(tbox->textlines[0]);
                tbox->textlines[0] = NULL;
            }
            for (int i = 1; i < tbox->nlines; ++i) {
                tbox->textlines[i-1] = tbox->textlines[i];
            }
        } else {
            /* found last string at position k */
            k++;
        }
        tbox->textlines[k] = str;
    }
}
