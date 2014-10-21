#include "btm.h"
#include "main.h"
#include "rtclock.h"
#include "battery.h"
#include "utils.h"
#include "leds.h"
#include "motor.h"
#include "gui.h"

#include "jsmn.h"
#include <string.h>
#include <ctype.h>

/* Scheduler includes. */
// #include "FreeRTOS.h"
#include "task.h"
// #include "queue.h"
#include "STM32_USART.h"

/* Library includes. */
#include "stm32f10x_it.h"


static int btm_rx_stream(long lPort);
static int btm_rx_seqnum(long lPort, int *seqnum);
static int btm_rx_new_msg(long lPort);
static int btm_rx_codetext(long lPort, char *buf, int buflen);
static int btm_rx_escapecode(long lPort, char *cch);
static int btm_rx_ack_msg(long lPort, int *seqnum);
static int btm_rx_status(long lPort);
static int btm_rx_statustext(long lPort, char *buf, int buflen);

static int do_decode_status(const char *buf);
static int do_rx_new_msg(char *buf);


int btm_state = BTMST_NonConfigured;



/* Remove all characters already received in USART queue */
void usartDrainInput(long lPort)
{
    char ch;
    while (xSerialGetChar(lPort, &ch, 0) == pdPASS)
        ;
}


/* Receive all characters until the buffer is full or there are none more
 * within the timeout */
int usartGetString(long lPort, char *buf, int buflen, TickType_t xBlockTime)
{
    char ch;            /* current character */
    int cnt = 0;        /* number of chars received */

    while (cnt < (buflen-1)) {
        if (xSerialGetChar(lPort, &ch, xBlockTime) != pdPASS) {
            /* no more chars in the buffer and we waited a timeout -> bail out */
            break;
        }

        /* save the character into buffer */
        buf[cnt++] = ch;
    }

    /* null-terminate, if possible */
    if (cnt < buflen) {
        buf[cnt] = 0;
    }

    /* print the received string to display */
    printstr(buf);

    return cnt;
}

/* Receive characters until the buffer is full or end of line or there are none more
 * within the timeout. */
int usartGetLine(long lPort, char *buf, int buflen, TickType_t xBlockTime)
{
    char ch;            /* current character */
    int cnt = 0;        /* number of chars received */
    int expect_newline = 0;

    while (cnt < (buflen-1)) {
        if (xSerialPeekChar(lPort, &ch, xBlockTime) != pdPASS) {
            /* no more chars in the buffer and we waited a timeout -> bail out */
            break;
        }

        if (expect_newline && ch != '\n') {
            /* we were expecting \n now, but it has not come -> leave ch in the queue and exit */
            break;
        }

        /* save the character into buffer */
        buf[cnt++] = ch;

        /* if it is \r, then expect \n as the next character */
        if (ch == '\r') {
            expect_newline = 1;
        }

        /* physically remove the character we've peeked from the queue */
        xSerialGetChar(lPort, &ch, xBlockTime);

        if (ch == '\n') {
            /* end of line -> end of str, for now */
            break;
        }
    }

    /* null-terminate, if possible */
    if (cnt < buflen) {
        buf[cnt] = 0;
    }

    /* print the received string to display */
    // printstr(buf);

    return cnt;
}


/* Expecting the 'OK' response from modem. Wait for it and do anything
 * possible to decode it. */
int btmExpectOK()
{
    const int buflen = 64;
    char xbuf[buflen];
    int len;

    len = usartGetString(comBTM, xbuf, buflen, ( TickType_t ) 200 / portTICK_PERIOD_MS);

    char *buf = xbuf;

    if (len < 4) {
        goto err;
    }

    if ((buf[0] == 'A' && buf[1] == 'T') || (buf[0] == '\r')) {
        // discard echo text
        while (!(buf[0] == '\r' && buf[1] == '\n')) {
            if (buf[0] == 0) {
                goto err;
            }
            ++buf;
        }
        // skip \r\n
        buf += 2;
    }

    if (buf[0] != 'O') {
        goto err;
    }
    if (buf[1] != 'K') {
        goto err;
    }
    if (buf[2] != '\r') {
        goto err;
    }
    if (buf[3] != '\n') {
        goto err;
    }

    return 0;

err:
#if 0
    zbuf = pvPortMalloc(sizeof(char) * 64);
    strncpy(zbuf, xbuf, len);
    for (int i = 0; i < 63; ++i) {
        if (zbuf[i] == '\r' || zbuf[i] == '\n') {
            zbuf[i] = ' ';
        }
    }
    zbuf[63] = 0;
    xQueueSend(toDisplayStrQueue, &zbuf, 0);
#endif
    return 1;
}

void setBtmState(int new_btmst)
{
    btm_state = new_btmst;

    switch (btm_state) {
        case BTMST_NonConfigured:
            LEDs_Set(LED0, LED_INTENS_100, LED_INTENS_0, LED_INTENS_0);
            break;

        case BTMST_Configured:
            LEDs_Set(LED0, LED_INTENS_100, LED_INTENS_100, LED_INTENS_0);
            break;

        case BTMST_Listening:
            LEDs_Set(LED0, LED_INTENS_0, LED_INTENS_100, LED_INTENS_0);
            break;

        case BTMST_Connected:
            LEDs_Set(LED0, LED_INTENS_0, LED_INTENS_0, LED_INTENS_100);
            break;

        default: break;
    }
}


void btmInitModem()
{
    /* Initialise COM0, which is USART1 according to the STM32 libraries. */
    lCOMPortInit( comBTM, mainBAUD_RATE );

    setBtmState(BTMST_NonConfigured);

    /* Reset BTM */
    #if 0
    GPIO_ResetBits(BTM_Reset_Port, BTM_Reset_Pin);
    vTaskDelay( ( TickType_t ) 10 / portTICK_PERIOD_MS );
    GPIO_SetBits(BTM_Reset_Port, BTM_Reset_Pin);
    #endif

    // do { } while (1);

    // const char *atEscape = "^^^";
    const char *atEscapeChar = "^";
    const char *atEOL = "\r";
    const char *atTest = "AT\r";
    
    // after-reset condition: give the BT module some time to init itself.
    vTaskDelay( ( TickType_t ) 1000 / portTICK_PERIOD_MS );

    /* Reset */
    const char *atReset = "ATZ\r";
    lSerialPutString( comBTM, atReset, strlen(atReset) );
    if (btmExpectOK()) {
        // failed; Try getting into command mode by sending the escape sequence:
        do {
            // Before the escape sequence there must be silence for guard time
            vTaskDelay( ( TickType_t ) 120 / portTICK_PERIOD_MS );
            
            lSerialPutString( comBTM, atEscapeChar, strlen(atEscapeChar) );
            vTaskDelay( ( TickType_t ) 120 / portTICK_PERIOD_MS );
            lSerialPutString( comBTM, atEscapeChar, strlen(atEscapeChar) );
            vTaskDelay( ( TickType_t ) 120 / portTICK_PERIOD_MS );
            lSerialPutString( comBTM, atEscapeChar, strlen(atEscapeChar) );
            
            // After the escape sequence there must be silence for guard time
            vTaskDelay( ( TickType_t ) 120 / portTICK_PERIOD_MS );

            // Send end of line
            lSerialPutString( comBTM, atEOL, strlen(atEOL) );
            // wait a little bit
            vTaskDelay( ( TickType_t ) 100 / portTICK_PERIOD_MS );
            // empty input buffer
            usartDrainInput(comBTM);            /* this drains possible 'ERROR 05' status */
            
            // Send plain AT
            lSerialPutString( comBTM, atTest, strlen(atTest) );
            
            // expect "OK\r\n"
        } while (btmExpectOK());
    }


    setBtmState(BTMST_Configured);

#if 0    
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2 /*| GPIO_Pin_1*/;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init( GPIOA, &GPIO_InitStruct );
#endif
#if 0
    /* Set mode */
    const char *atModeF1 = "AT&F1\r";
    lSerialPutString( comBTM, atModeF1, strlen(atModeF1) );
    if (btmExpectOK()) {
        // failed
        assert_failed(__FILE__, __LINE__);
    }
#endif

    /* Set Friendly name */
    const char *atSetDeviceName = "AT+BTF=\"PIP-Watch\"\r";
    lSerialPutString( comBTM, atSetDeviceName, strlen(atSetDeviceName) );
    if (btmExpectOK()) {
        // failed
        vTaskDelay( portMAX_DELAY );
        assert_failed(__FILE__, __LINE__);
    }

#if 0
    /* Open and make unit detectable */
    const char *atEnaRadio2 = "AT+BTO\r";
    lSerialPutString( comBTM, atEnaRadio2, strlen(atEnaRadio2) );
    if (btmExpectOK()) {
        // failed
        assert_failed(__FILE__, __LINE__);
    }
#endif

    /* Set passkey */
    const char *atSetPin = "AT+BTK=\"1984\"\r";
    lSerialPutString( comBTM, atSetPin, strlen(atSetPin) );
    if (btmExpectOK()) {
        // failed
        assert_failed(__FILE__, __LINE__);
    }

#if 1
    /* Set mode F1: Minimal power consumption, 9600Bd; mode F1= 0:4,4,48,80(50ms)
        S508 = 2500 = page scan interval [ms]
        S509 = 11 = page scan window [ms]
    */
    const char *atSetMinPower = "AT&F1\r";          
    // const char *atSetMinPower = "AT&F0\r";          // Medium power consumption, 9600Bd
    lSerialPutString( comBTM, atSetMinPower, strlen(atSetMinPower) );
    if (btmExpectOK()) {
        // failed
        assert_failed(__FILE__, __LINE__);
    }

#if 1
    /* Set shorter Page scan interval: 1200ms, lower latency when opening the connection, higher
     * probability of success, higher avg power.  */
    // const char *atSniffStatus = "ATI13\r";
    // const char *atSniffMaxInterval = "ATS564=60\r";
    const char *atPgScanInterv = "ATS508=1200\r";
    lSerialPutString( comBTM, atPgScanInterv, strlen(atPgScanInterv) );
    if (btmExpectOK()) {
        // failed
        assert_failed(__FILE__, __LINE__);
    }
#endif
#endif

#if 0
    /* Make modem Discoverable and connectable */
    const char *atEnaRadio = "AT+BTP\r";
    lSerialPutString( comBTM, atEnaRadio, strlen(atEnaRadio) );
    if (btmExpectOK()) {
        // failed
        assert_failed(__FILE__, __LINE__);
    }

    /* auto-save pairing info */
    const char *atSavePairing = "ATS538=1\r";
    lSerialPutString( comBTM, atSavePairing, strlen(atSavePairing) );
    if (btmExpectOK()) {
        // failed
        assert_failed(__FILE__, __LINE__);
    }
    
#else
    /* Make modem Connectable (Enable page scans only) */
    const char *atEnaRadio = "AT+BTG\r";
    // const char *atEnaRadio = "AT+BTG10BF48CD0778\r";         /* no effect on power consumption */
    lSerialPutString( comBTM, atEnaRadio, strlen(atEnaRadio) );
    if (btmExpectOK()) {
        // failed
        assert_failed(__FILE__, __LINE__);
    }
#endif

#if 0
    /* Query Sniff state */
    // const char *atSniffStatus = "ATI13\r";           // mode F1= 0:4,4,48,80
    const char *atSniffStatus = "ATS509?\r";
    lSerialPutString( comBTM, atSniffStatus, strlen(atSniffStatus) );
    /* btmExpectOK() WILL fail here! */
    if (btmExpectOK()) {
        // failed
        assert_failed(__FILE__, __LINE__);
    }
#endif

#if 1
    /* Disable local echo */
    const char *atDisableEcho = "ATE0\r";
    lSerialPutString( comBTM, atDisableEcho, strlen(atDisableEcho) );
    if (btmExpectOK()) {
        // failed
        vTaskDelay( portMAX_DELAY );
        assert_failed(__FILE__, __LINE__);
    }
#endif

    setBtmState(BTMST_Listening);
}

#define MSGBUFSIZE      64

/* Described at the top of this file. */
void BluetoothModemTask( void *pvParameters )
{
    /* Just to avoid compiler warnings. */
    ( void ) pvParameters;

    /* modem setup  */
    btmInitModem();

    while (1) {
        btm_rx_stream(comBTM);
    }

#if 0
    char *buf = NULL;

    for( ;; )
    {
        if (!buf) {
            buf = pvPortMalloc(sizeof(char) * MSGBUFSIZE);
        }

        /* Block and wait for a line */
        usartGetLine(comBTM, buf, MSGBUFSIZE, portMAX_DELAY);

        if ((strcmp(buf, "\r") == 0) || (strcmp(buf, "\n") == 0) || (strcmp(buf, "\r\n") == 0)) {
            /* discard empty line */
            continue;
        }

        if (strncmp(buf, "RING ", 5) == 0) {
            /* Incomming connection -> send an ack to accept */
            const char *atAcceptRing = "ATA\r";
            lSerialPutString( comBTM, atAcceptRing, strlen(atAcceptRing) );
        
            setBtmState(BTMST_Connected);
            Motor_Pulse(MOTOR_DUR_LONG);
            
            /* clear buffer */
            buf[0] = '\0';
            continue;
        }

        if (strncmp(buf, "CONNECT ", 8) == 0) {
            /* Connected to a remote. */
            /* no need to do anything else now */
            
            /* clear buffer */
            buf[0] = '\0';
            continue;
        }

        if (strncmp(buf, "NO CARRIER", 10) == 0) {
            /* Disconnect */
            setBtmState(BTMST_Listening);
            /* clear buffer */
            buf[0] = '\0';
            continue;
        }

        for (int i = 0; i < ((int)strlen(buf))-4; ++i) {
            if (buf[i] == '*') {
                /* set time: *<hours><minutes> */
                int hours = (buf[i+1]-'0')*10 + (buf[i+2]-'0');
                int minutes = (buf[i+3]-'0')*10 + (buf[i+4]-'0');
                hours %= 24;
                minutes %= 60;
                current_rtime.sec = 0;
                current_rtime.hour = hours;
                current_rtime.min = minutes;
                break;
            }
        }

        struct guievent gevnt;
        gevnt.evnt = GUI_E_PRINTSTR;
        gevnt.buf = buf;
        gevnt.kpar = 0;

        if (xQueueSend(toGuiQueue, &gevnt, 0) == pdTRUE) {
            // ok; will alloc new buffer
            buf = NULL;
            Motor_Pulse(MOTOR_DUR_MEDIUM);
        } else {
            // fail; ignore, keep buffer
        }

        // motor demo
        // GPIO_SetBits(GPIOB, 1 << 13);
        // vTaskDelay( ( TickType_t ) 300 / portTICK_PERIOD_MS );
        // GPIO_ResetBits(GPIOB, 1 << 13);
    }
#endif
}


/**
    <btm_rx_stream> :=
        . <new_msg> | <msg_ack> | <msg_nack> | <btm_status>
 */
static int btm_rx_stream(long lPort)
{
    char ch = 0;
    int seqnum = 0;

    // xSerialPeekChar(lPort, &ch, portMAX_DELAY);
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
            return 1;
            break;
    }
}

/**
    <seqnum> :=
        . <digit> <digit>
 */
static int btm_rx_seqnum(long lPort, int *seqnum)
{
    char ch = 0;
    int cnt = 0;
    *seqnum = 0;

    do {
        // xSerialPeekChar(lPort, &ch, portMAX_DELAY);
        xSerialGetChar(lPort, &ch, portMAX_DELAY);

        if (ch == '\r') {
            /* receive status message */
            btm_rx_status(lPort);
            continue;
        }

        if (!isdigit((unsigned char)ch)) {
            /* error: not a digit */
            return 1;
        }

        *seqnum = *seqnum * 10 + (ch - '0');
        ++cnt;
    } while (cnt < 2);

    return 0;
}


/**
    <new_msg> :=
        '*' . <seqnum> <code_text> <lf>
 */
static int btm_rx_new_msg(long lPort)
{
    int seqnum;

    if (btm_rx_seqnum(lPort, &seqnum) != 0) {
        return 1;
    }

    const int buflen = 2048;
    char *buf = pvPortMalloc(sizeof(char)*buflen);

    if (!buf) {
        /* no memory */
        return 1;
    }

    if (btm_rx_codetext(lPort, buf, buflen) != 0) {
        return 1;
    }

    /* give the message to upper layer; the buffer is consumed */
    do_rx_new_msg(buf);

    return 0;
}

/**
    <code_text> := non-visible chars below space, backslash char, are coded
        using escape \<2-hex-num>
 */
static int btm_rx_codetext(long lPort, char *buf, int buflen)
{
    char ch = 0;
    int cnt = 0;

    do {
        if (cnt >= buflen-1) {
            return 1;
        }

        xSerialGetChar(lPort, &ch, portMAX_DELAY);

        switch (ch) {
            case '\r':  /* modem status msg */
                btm_rx_status(lPort);
                break;

            case '\n':  /* end of codetext */
                buf[cnt++] = ch;
                buf[cnt++] = 0;
                return 0;
                break;

            case '\\':  /* escape char */
                btm_rx_escapecode(lPort, &ch);
                buf[cnt++] = ch;
                break;

            default:
                if (ch >= 0 && ch < 32) {
                    /* this char should have been escaped! */
                    return 1;
                }

                buf[cnt++] = ch;
                break;
        }
    } while (1);
}

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

/**
    btm_rx_ack_msg :=
        '+' . <seqnum> <lf> |
        '-' . <seqnum> <lf>
 */
static int btm_rx_ack_msg(long lPort, int *seqnum)
{
    char ch = 0;

    if (btm_rx_seqnum(lPort, seqnum) != 0)
        return 1;

    do {
        xSerialGetChar(lPort, &ch, portMAX_DELAY);

        switch (ch) {
            case '\r':
                /* receive status message */
                btm_rx_status(lPort);
                break;
            
            case '\n':
                return 0;
            
            default:
                /* error */
                return 1;
        }
    } while (1);

}

/**
    btm_rx_statustext :=
        . <string text, no escaping> <cr> <lf>
 */
static int btm_rx_statustext(long lPort, char *buf, int buflen)
{
    char ch;
    int cnt = 0;

    do {
        if (cnt >= buflen-1) {
            buf[cnt] = 0;
            return 1;
        }

        xSerialGetChar(lPort, &ch, portMAX_DELAY);

        switch (ch) {
            case '\r':  /* modem status msg */
                /* ignore */
                break;

            case '\n':  /* end of codetext */
                buf[cnt++] = ch;
                buf[cnt++] = 0;
                return 0;
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
static int btm_rx_status(long lPort)
{
    char ch;

    xSerialGetChar(lPort, &ch, portMAX_DELAY);
    if (ch != '\n') {
        /* expected \n */
        return 1;
    }

    const int buflen = 256;
    char *buf = pvPortMalloc(sizeof(char) * buflen);

    if (!buf) {
        /* malloc failed */
        return 1;
    }
    
    if (btm_rx_statustext(lPort, buf, buflen)) {
        /* error reading status message from modem */
        return 1;
    }

    /* act on the status message */
    int res = do_decode_status(buf);
    vPortFree(buf);
    return res;
}

/* Decode and act on status message received from the modem. 
 * The status text is normally ended by <lf> and \0.

RING <btaddr><lf>
PIN? <btaddr><lf>
PAIR <n> <btaddr><lf>
PAIR 0 <btaddr> <MM><lf>
RX<string><lf>
CONNECT <btaddr><lf>
NO CARRIER<lf>

 */
static int do_decode_status(const char *buf)
{
    if (strncmp(buf, "RING ", 5) == 0) {
        /* Incomming connection -> send an ack to accept */
        const char *atAcceptRing = "ATA\r";
        lSerialPutString( comBTM, atAcceptRing, strlen(atAcceptRing) );
    
        setBtmState(BTMST_Connected);
        Motor_Pulse(MOTOR_DUR_LONG);
        
        return 0;
    }

    if (strncmp(buf, "CONNECT ", 8) == 0) {
        /* Connected to a remote. */
        /* no need to do anything else now */
        return 0;
    }

    if (strncmp(buf, "NO CARRIER", 10) == 0) {
        /* Disconnect */
        setBtmState(BTMST_Listening);
        return 0;
    }

    return 1;
}


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


/* NOTE: the buf is consumed here! */
static int do_rx_new_msg(char *buf)
{
    struct guievent gevnt;
    jsmn_parser parser;
    jsmntok_t *tokens = NULL;
    int maxtok = 64;
    // int result = 0;

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

                prs_ptok_time(buf, &tok[1], tok[0].size, &current_rtime);

                gevnt.evnt = GUI_E_PRINTSTR;
                gevnt.buf = pvPortMalloc(16);
                strcpy(gevnt.buf, "time-ok");
                gevnt.kpar = 0;
                xQueueSend(toGuiQueue, &gevnt, 0);

                tok += 2;
                continue;
            }

            if (match_tok(buf, &tok[0], tokend, "msgtext", JSMN_STRING)
                    && match_tok(buf, &tok[1], tokend, NULL, JSMN_STRING)) {
                printstrn(buf+tok[1].start, tok[1].end - tok[1].start);
                tok += 2;
                continue;
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
