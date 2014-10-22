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
