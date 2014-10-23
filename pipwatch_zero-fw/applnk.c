#include "applnk.h"
#include "rtclock.h"
#include "sms.h"
#include "gui.h"
#include "utils.h"
#include "jsmn.h"

#include "FreeRTOS.h"
#include "task.h"

#include <string.h>
#include <ctype.h>

static void prs_ptok_time(const char *buf, const jsmntok_t *tok, int tcount, rtclock_t *tm);
static int match_tok(const char *buf, const jsmntok_t *tok, const jsmntok_t *tokend, const char *tokname, int jsmntp);
static int prs_otok_sms(const char *buf, const jsmntok_t *tok, int tcount, struct smstext *sms);



/* parse primitive token "time" */
static void prs_ptok_time(const char *buf, const jsmntok_t *tok, int tcount, rtclock_t *tm)
{
    int i = tok[0].start - 1;
    int hours = 0;
    if (i+2 <= tok[0].end) {
        hours = (buf[i+1]-'0')*10 + (buf[i+2]-'0');
    }
    int minutes = 0;
    if (i+4 <= tok[0].end) {
        minutes = (buf[i+3]-'0')*10 + (buf[i+4]-'0');
    }
    int seconds = 0;
    if (i+6 <= tok[0].end) {
        seconds = (buf[i+5]-'0')*10 + (buf[i+6]-'0');
    }

    hours %= 24;
    minutes %= 60;
    seconds %= 60;

    tm->hour = hours;
    tm->min = minutes;
    tm->sec = seconds;
}


/* Matches tokname and jsmntp agains the current tok. Also checks that the tok is not beyond toklast.
 * Return 0 on false, 1 on match.
 */
static int match_tok(const char *buf, const jsmntok_t *tok, const jsmntok_t *tokend, const char *tokname, int jsmntp)
{
    if (tok >= tokend) {
        /* at the end - fail */
        return 0;
    }
    if (tokname != NULL) {
        if (tok->end - tok->start != strlen(tokname))
            /* name len not match */
            return 0;
        if (strncmp(buf+tok->start, tokname, strlen(tokname)) != 0)
            /* name match fail */
            return 0;
    }
    if (jsmntp >= 0) {
        if (tok->type != jsmntp)
            /* token type match fail */
            return 0;
    }
    return 1;       /* match found */
}


static int prs_otok_sms(const char *buf, const jsmntok_t *tok, int tcount, struct smstext *sms)
{
    const jsmntok_t *ptok = &tok[1];
    const jsmntok_t *tokend = tok + 1 + tcount;

    sms->tm_recv.sec = sms->tm_recv.min = sms->tm_recv.hour = 0;
    sms->sender_name = NULL;
    sms->sender_phone = NULL;
    sms->text = NULL;

    while (ptok < tokend) {
        if (match_tok(buf, &ptok[0], tokend, "time", JSMN_STRING)
                && match_tok(buf, &ptok[1], tokend, NULL, JSMN_STRING)) {
            /* decode time from string and place in tm_recv in sms */
            prs_ptok_time(buf, &ptok[1], ptok[0].size, &sms->tm_recv);
            /* two tokens consumed */
            ptok += 2;
            continue;
        }

        if (match_tok(buf, &ptok[0], tokend, "sendername", JSMN_STRING)
                && match_tok(buf, &ptok[1], tokend, NULL, JSMN_STRING)) {
            /* alloc new string and copy sender name to sms struct */
            sms->sender_name = newstrn(buf+ptok[1].start, ptok[1].end-ptok[1].start);
            /* two tokens consumed */
            ptok += 2;
            continue;
        }

        if (match_tok(buf, &ptok[0], tokend, "senderphone", JSMN_STRING)
                && match_tok(buf, &ptok[1], tokend, NULL, JSMN_STRING)) {
            /* alloc new string and copy sender name to sms struct */
            sms->sender_phone = newstrn(buf+ptok[1].start, ptok[1].end-ptok[1].start);
            /* two tokens consumed */
            ptok += 2;
            continue;
        }

        if (match_tok(buf, &ptok[0], tokend, "text", JSMN_STRING)
                && match_tok(buf, &ptok[1], tokend, NULL, JSMN_STRING)) {
            /* alloc new string and copy text to sms struct */
            sms->text = newstrn(buf+ptok[1].start, ptok[1].end-ptok[1].start);
            /* two tokens consumed */
            ptok += 2;
            continue;
        }

        break;
    }

    return 0;
}

/* NOTE: the buf is consumed here! */
int applnk_rx_new_msg(char *buf)
{
    jsmn_parser parser;
    jsmntok_t *tokens = NULL;
    int maxtok = 64;

    jsmn_init(&parser);

    tokens = pvPortMalloc(sizeof(jsmntok_t) * maxtok);
    if (tokens == NULL) {
        /* out of memory */
        printstr("do_rx_new_msg: tokens malloc");
        return 1;
    }

    int tokcnt = jsmn_parse(&parser, buf, strlen(buf), tokens, maxtok);

    if (tokcnt < 0) {
        /* parse error */
        printstr("do_rx_new_msg: parse error");
        vPortFree(tokens);
        return 1;
    }

    jsmntok_t *tok = tokens;
    const jsmntok_t *tokend = tokens+tokcnt;

    if (match_tok(buf, tok, tokend, NULL, JSMN_OBJECT)) {
        tok += 1;

        while (tok < tokend) {
            if (match_tok(buf, &tok[0], tokend, "time", JSMN_STRING)
                    && match_tok(buf, &tok[1], tokend, NULL, JSMN_STRING)) {
                /* decode time and set it to the current clock time */
                prs_ptok_time(buf, &tok[1], tok[0].size, &current_rtime);
                /* visual feedback, also time updates immediately */
                printstr("time-ok");
                /* two tokens consumed */
                tok += 2;
                continue;
            }

            if (match_tok(buf, &tok[0], tokend, "msgtext", JSMN_STRING)
                    && match_tok(buf, &tok[1], tokend, NULL, JSMN_STRING)) {
                /* print text verbatim */
                printstrn(buf+tok[1].start, tok[1].end - tok[1].start);
                /* two tokens consumed */
                tok += 2;
                continue;
            }

            if (match_tok(buf, &tok[0], tokend, "sms", JSMN_STRING)
                    && match_tok(buf, &tok[1], tokend, NULL, JSMN_OBJECT)) {
                /* parse sms */
                struct smstext *sms = pvPortMalloc(sizeof(struct smstext));
                if (sms == NULL) {
                    vPortFree(tokens);
                    vPortFree(buf);
                    printstr("do_rx_new_msg: sms oom");
                    return 1;
                }
                prs_otok_sms(buf, &tok[1], tok[1].size, sms);
                /* send sms to gui */
                struct guievent gevnt;
                gevnt.evnt = GUI_E_NEWSMS;
                gevnt.buf = sms;
                gevnt.kpar = 0;
                xQueueSend(toGuiQueue, &gevnt, 0);
                /* two tokens plus all subtokens of the second one consumed */
                tok += 2 + tok[1].size;
            }

            break;
        }

    }

    vPortFree(tokens);

    if (tok < tokend) {
        /* not all tokens recognized; print the buf string... */
        printstr(buf);
    }

    vPortFree(buf);

#if 0
    gevnt.evnt = GUI_E_PRINTSTR;
    gevnt.buf = buf;
    gevnt.kpar = 0;

    if (xQueueSend(toGuiQueue, &gevnt, 0) == pdTRUE) {
        // ok; will alloc new buffer
        buf = NULL;
        Motor_Pulse(MOTOR_DUR_MEDIUM);
    } else {
        // fail; ignore, keep buffer
        vPortFree(buf);
    }
#endif

    return 0;
}
