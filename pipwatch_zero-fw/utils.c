#include "utils.h"


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
