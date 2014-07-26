#include "btm.h"
#include "main.h"
#include "rtclock.h"
#include "battery.h"

#include <string.h>

/* Scheduler includes. */
// #include "FreeRTOS.h"
#include "task.h"
// #include "queue.h"
#include "STM32_USART.h"

/* Library includes. */
#include "stm32f10x_it.h"


void usartDrainInput(long lPort)
{
    char ch;
    while (xSerialGetChar(lPort, &ch, 0) == pdPASS)
        ;
}

int usartGetString(long lPort, char *buf, int buflen, TickType_t xBlockTime)
{
    char ch;
    int cnt = 0;
    while ((cnt < buflen) && (xSerialGetChar(lPort, &ch, xBlockTime) == pdPASS)) {
        buf[cnt++] = ch;
    }
    /* null-terminate, if possible */
    if (cnt < buflen) {
        buf[cnt] = 0;
    }
    return cnt;
}

int btmExpectOK()
{
    const int buflen = 64;
    char xbuf[buflen];
    int len;

    len = usartGetString(comBTM, xbuf, buflen, ( TickType_t ) 10 / portTICK_PERIOD_MS);

    char *buf = xbuf;

    if (len < 4)
        return 1;

    if ((buf[0] == 'A' && buf[1] == 'T') || (buf[0] == '\r')) {
        // discard echo text
        while (!(buf[0] == '\r' && buf[1] == '\n')) {
            if (buf[0] == 0)
                return 5;
            ++buf;
        }
        // skip \r\n
        buf += 2;
    }

    if (buf[0] != 'O')
        return 2;
    if (buf[1] != 'K')
        return 3;
    if (buf[2] != '\r')
        return 4;
    if (buf[3] != '\n')
        return 4;

    return 0;
}


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


/* Described at the top of this file. */
void BluetoothModemTask( void *pvParameters )
{
    char cChar;

    /* Just to avoid compiler warnings. */
    ( void ) pvParameters;


    /* Initialise COM0, which is USART1 according to the STM32 libraries. */
    lCOMPortInit( comBTM, mainBAUD_RATE );

    // do { } while (1);

    const char *atEscape = "///";
    const char *atEOL = "\r";
    const char *atTest = "AT\r";
    
    // after-reset condition: give the BT module some time to init itself.
    vTaskDelay( ( TickType_t ) 1000 / portTICK_PERIOD_MS );

    do {
        #if 1
        // Before the escape sequence there must be silence for 1s
        vTaskDelay( ( TickType_t ) 1200 / portTICK_PERIOD_MS );
        lSerialPutString( comBTM, atEscape, strlen(atEscape) );
        // After the escape sequence there must be silence for 1s
        vTaskDelay( ( TickType_t ) 1200 / portTICK_PERIOD_MS );
        #endif

        // Send end of line
        lSerialPutString( comBTM, atEOL, strlen(atEOL) );
        // wait a little bit
        vTaskDelay( ( TickType_t ) 10 / portTICK_PERIOD_MS );
        // empty input buffer
        usartDrainInput(comBTM);
        
        // Send plain AT
        lSerialPutString( comBTM, atTest, strlen(atTest) );
        // vTaskDelay( ( TickType_t ) 20 / portTICK_PERIOD_MS );
        
        // expect "OK\r\n"
    } while (btmExpectOK());

    // disable local echo
    const char *atDisableEcho = "ATE0\r";
    lSerialPutString( comBTM, atDisableEcho, strlen(atDisableEcho) );
    if (btmExpectOK()) {
        // failed
        assert_failed(__FILE__, __LINE__);
    }

    const char *atSetDeviceName = "AT*agln=\"PIP-Watch\",0\r\n";
    lSerialPutString( comBTM, atSetDeviceName, strlen(atSetDeviceName) );
    if (btmExpectOK()) {
        // failed
        assert_failed(__FILE__, __LINE__);
    }

    const char *atSetPin = "AT*agfp=\"1234\",0\r";
    lSerialPutString( comBTM, atSetPin, strlen(atSetPin) );
    if (btmExpectOK()) {
        // failed
        assert_failed(__FILE__, __LINE__);
    }

    const char *atToDataMode = "AT*addm\r";
    lSerialPutString( comBTM, atToDataMode, strlen(atToDataMode) );
    if (btmExpectOK()) {
        // failed
        assert_failed(__FILE__, __LINE__);
    }


    /* Try sending out a string all in one go, as a very basic test of the
    lSerialPutString() function. */
    // lSerialPutString( comBTM, pcLongishString, strlen( pcLongishString ) );

    int k = 0;
    char *buf = NULL;

    for( ;; )
    {
        /* Block to wait for a character to be received on COM0. */
        xSerialGetChar( comBTM, &cChar, portMAX_DELAY );

        /* Write the received character back to COM0. */
        xSerialPutChar( comBTM, cChar, 0 );

        if (!buf) {
            buf = pvPortMalloc(sizeof(char) * 32);

        #if 0
            /* start ADC conversion by software */
            // ADC_ClearFlag(ADC1, ADC_FLAG_EOC);
            ADC_ClearFlag(ADC1, ADC_FLAG_STRT);
            ADC_Cmd(ADC1, ENABLE);
        #if 0
            ADC_SoftwareStartConvCmd(ADC1, ENABLE);
            /* wait till the conversion starts */
            while (ADC_GetSoftwareStartConvStatus(ADC1) != RESET) { }
        #endif
            /* wait till the conversion ends */
            while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) != SET) { }
        #endif
            
            // k = 0;
            // k = itostr(buf, 32, RTC_GetCounter());
            // k = itostr(buf, 32, ADC_GetConversionValue(ADC1));
            // k = itostr(buf, 32, vbat_measured);
            k = itostr(buf, 32, vbat_percent);

            // ADC_ClearFlag(ADC1, ADC_FLAG_EOC);
        }

        buf[k++] = cChar;
        
        if (cChar == '\r' || k >= 30) {
            buf[k] = '\0';
            
            for (int i = 0; i < k-4; ++i) {
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

            if (xQueueSend(toDisplayStrQueue, &buf, 0) == pdTRUE) {
                // ok; will alloc new buffer
                buf = NULL;
            } else {
                // fail; ignore, keep buffer
            }
            k = 0;
            xSerialPutChar( comBTM, '\n', 0 );
        }

    }
}
