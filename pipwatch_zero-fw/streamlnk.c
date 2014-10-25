#include "streamlnk.h"
#include "btm.h"
#include "applnk.h"
#include "utils.h"

#include "FreeRTOS.h"
// #include "queue.h"
#include "task.h"

#include "STM32_USART.h"

#include <string.h>
#include <ctype.h>


#define EAT_HONOR_CR	0
#define EAT_ALSO_CR		1



static streamlnkresult btm_rx_msg(long lPort);
static streamlnkresult btm_rx_seqnum(long lPort, int *seqnum);
static streamlnkresult btm_rx_new_msg(long lPort);
static streamlnkresult btm_rx_codetext(long lPort, char *buf, int buflen);
// static int btm_rx_escapecode(long lPort, char *cch);
static streamlnkresult btm_rx_ack_msg(long lPort, int *seqnum);
static streamlnkresult btm_rx_status(long lPort);
static streamlnkresult btm_rx_statustext(long lPort, char *buf, int buflen);
static void btm_eat_till_eom(long lPort, int eat_cr);

static void btm_send_ack(long lPort, char acktp, int seqnum);


/**
	<btm_rx_stream> :=
	    { <btm_rx_msg> }*
	    ON ERROR:
	        a) eat till <lf>
	        b) restart parser
 */
int btm_rx_stream(long lPort)
{
    do {
    	streamlnkresult result = btm_rx_msg(lPort);

    	switch (result) {
    		case SLNK_OK:
    		case SLNK_RESET:
    			break;

    		case SLNK_OOM:
    		case SLNK_PARSE_ERROR:
    			/* eat till <lf> */
    			btm_eat_till_eom(lPort, EAT_HONOR_CR);
    			break;
    	}
    } while (1);
}


/* Eat all chars until <lf>.
 * If eat_cr=1, then eat also <cr>, otherwise act on <cr>
 * as usual - as a modem unsolicitated message.
 */
static void btm_eat_till_eom(long lPort, int eat_cr)
{
	int ch = 0;

	do {
		xSerialGetChar(lPort, &ch, portMAX_DELAY);

		switch (ch) {
			case '\n':
				/* end of line */
				return;

			case '\r':
				if (eat_cr == EAT_HONOR_CR) {
					streamlnkresult result = btm_rx_status(lPort);
					if (result == SLNK_RESET) {
						/* state reset, no need to continue eating */
						return;
					}
				}
				break;

			default:
				break;
		}
	} while (1);
}


/**
    <btm_rx_msg> :=
        . {<new_msg> | <msg_ack> | <msg_nack> | <btm_status>}
 */
streamlnkresult btm_rx_msg(long lPort)
{
    int ch = 0;
    int seqnum = 0;

    xSerialGetChar(lPort, &ch, portMAX_DELAY);

    switch (ch) {
        case '*':   /* start of new message */
            return btm_rx_new_msg(lPort);
            break;

        case '+':   /* ack of a msg */
            return btm_rx_ack_msg(lPort, &seqnum);
            break;

        case '-':   /* nack of a msg */
            return btm_rx_ack_msg(lPort, &seqnum);
            break;

        case '\r':  /* modem message */
            return btm_rx_status(lPort);
            break;

        default:    /* error */
            return SLNK_PARSE_ERROR;
            break;
    }
}


/**
    <seqnum> :=
        . <digit> <digit>
 */
static streamlnkresult btm_rx_seqnum(long lPort, int *seqnum)
{
    int ch = 0;
    int cnt = 0;
    *seqnum = 0;

    do {
        // xSerialPeekChar(lPort, &ch, portMAX_DELAY);
        xSerialGetChar(lPort, &ch, portMAX_DELAY);

        if (ch == '\r') {
            /* receive status message */
            streamlnkresult result = btm_rx_status(lPort);
            if (result != SLNK_OK) {
            	return result;
            }
            continue;
        }

        if (!isdigit(ch)) {
            /* error: not a digit */
            return SLNK_PARSE_ERROR;
        }

        *seqnum = *seqnum * 10 + (ch - '0');
        ++cnt;
    } while (cnt < 2);

    return SLNK_OK;
}


/**
    <new_msg> :=
        '*' . <seqnum> <code_text> <lf>
 */
static streamlnkresult btm_rx_new_msg(long lPort)
{
    int seqnum = 0;
    streamlnkresult result = SLNK_OK;

    if ((result = btm_rx_seqnum(lPort, &seqnum)) != SLNK_OK) {
        return result;       /* stream parsing error */
    }

    const int buflen = 2048;
    char *buf = pvPortMalloc(sizeof(char)*buflen);

    if (!buf) {
        /* no memory, send nack */
        btm_send_ack(lPort, '-', seqnum);
        return SLNK_OOM;       /* stream parsing error */
    }

    if ((result = btm_rx_codetext(lPort, buf, buflen)) != SLNK_OK) {
        /* code text error */
        btm_send_ack(lPort, '-', seqnum);
        return result;       /* stream parsing error */
    }

    /* give the message to upper layer; the buffer is consumed in any case! */
    if (applnk_rx_new_msg(buf) != 0) {
        /* message decoding error (higher layer) */
        btm_send_ack(lPort, '-', seqnum);
    } else {
        /* ok received and processed, send ack */
        btm_send_ack(lPort, '+', seqnum);
    }

    return SLNK_OK;           /* stream parsing ok */
}

/**
<code_text> := non-visible chars below space  must be code using escape secquence.
    Their presence here is either an error, or a token <cr>, <lf>.
    <cr> = modem message
    <lf> = end of code_text
    Escape sequences will be expanded in application (presentation) layer, not in the stream layer!
 */
static streamlnkresult btm_rx_codetext(long lPort, char *buf, int buflen)
{
    int ch = 0;
    int cnt = 0;

    do {
        if (cnt >= buflen-1) {
            return SLNK_OOM;
        }

        xSerialGetChar(lPort, &ch, portMAX_DELAY);

        switch (ch) {
            case '\r': { /* modem status msg */
                streamlnkresult result = btm_rx_status(lPort);
                if (result != SLNK_OK) {
                	return result;
                }
                break;
            }

            case '\n':  /* end of codetext */
                buf[cnt++] = ch;
                buf[cnt++] = 0;
                return SLNK_OK;
                break;
#if 0
            case '\\':  /* escape char */
                btm_rx_escapecode(lPort, &ch);
                buf[cnt++] = ch;
                break;
#endif
            default:
                if (ch >= 0 && ch < 32) {
                    /* this char should have been escaped! */
                    return SLNK_PARSE_ERROR;
                }

                buf[cnt++] = ch;
                break;
        }
    } while (1);
}

#if 0
/**
    escapecode := 
        '\' . <hex-char> <hex-char>
 */
static int btm_rx_escapecode(long lPort, char *cch)
{
    char ch = 0;
    *cch = 0;
    int cnt = 0;

    do {
        xSerialGetChar(lPort, &ch, portMAX_DELAY);

        if (ch == '\r') {
            /* receive status message */
            btm_rx_status(lPort);
            continue;
        }

        if (!isxdigit((unsigned char)ch)) {
            return 1;
        }

        int x = 0;
        if (isdigit((unsigned char)ch)) {
            x = ch - '0';
        } else if (ch >= 'a' && ch <= 'f') {
            x = ch - 'a' + 10;
        } else if (ch >= 'A' && ch <= 'F') {
            x = ch - 'A' + 10;
        }

        *cch = *cch * 16 + x;
        ++cnt;
    } while (cnt < 2);

    return 0;
}
#endif

/**
    btm_rx_ack_msg :=
        '+' . <seqnum> <lf> |
        '-' . <seqnum> <lf>
 */
static streamlnkresult btm_rx_ack_msg(long lPort, int *seqnum)
{
    int ch = 0;
    streamlnkresult result;

    if ((result = btm_rx_seqnum(lPort, seqnum)) != SLNK_OK) {
        return result;
    }

    do {
        xSerialGetChar(lPort, &ch, portMAX_DELAY);

        switch (ch) {
            case '\r':
                /* receive status message */
                result = btm_rx_status(lPort);
                if (result != SLNK_OK) {
                	return result;
                }
                break;
            
            case '\n':
                return SLNK_OK;
            
            default:
                /* error */
                return SLNK_PARSE_ERROR;
        }
    } while (1);

}

/**
    btm_rx_statustext :=
        . <string text, no escaping> <cr> <lf>
 */
static streamlnkresult btm_rx_statustext(long lPort, char *buf, int buflen)
{
    int ch;
    int cnt = 0;

    do {
        if (cnt >= buflen-1) {
            buf[cnt] = 0;
            return SLNK_OOM;
        }

        xSerialGetChar(lPort, &ch, portMAX_DELAY);

        switch (ch) {
            case '\r':  /* modem status msg */
                /* ignore */
                break;

            case '\n':  /* end of codetext */
                buf[cnt++] = ch;
                buf[cnt++] = 0;
                return SLNK_OK;
                break;

            default:
                buf[cnt++] = ch;
                break;
        }
    } while (1);
}

/**
    <btm_status> :=
        <cr> . <lf> <status_text> <cr> <lf>
 */
static streamlnkresult btm_rx_status(long lPort)
{
    int ch;
    streamlnkresult result;

    xSerialGetChar(lPort, &ch, portMAX_DELAY);
    if (ch != '\n') {
        /* expected \n */
        return SLNK_PARSE_ERROR;
    }

    const int buflen = 256;
    char *buf = pvPortMalloc(sizeof(char) * buflen);

    if (!buf) {
        /* malloc failed */
        /* eat till end of message <lf>, including <cr> */
        btm_eat_till_eom(lPort, EAT_ALSO_CR);
        return SLNK_RESET;
    }
    
    if ((result = btm_rx_statustext(lPort, buf, buflen)) != SLNK_OK) {
        /* error reading status message from modem */
        if (result == SLNK_OOM) {
	        /* eat till end of message <lf>, including <cr> */
	        btm_eat_till_eom(lPort, EAT_ALSO_CR);
			result = SLNK_RESET;        	
        }
        return result;
    }

    /* act on the status message */
    if (do_decode_btm_status(buf) == 0) {
        result = SLNK_OK;
    } else {
        result = SLNK_RESET;
    }
    vPortFree(buf);
    return result;
}

/**
    Write ack/nack to btm link.
 */
static void btm_send_ack(long lPort, char acktp, int seqnum)
{
    char buf[4];
    itostr_rjust(buf, 2, seqnum % 100, '0');
    buf[2] = '\0';

    xSerialPutChar(lPort, acktp, ( 5 / portTICK_PERIOD_MS ));
    lSerialPutString(lPort, buf, 2);
    xSerialPutChar(lPort, '\n', ( 5 / portTICK_PERIOD_MS ));
}
