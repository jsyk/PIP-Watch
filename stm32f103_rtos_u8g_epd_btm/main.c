
/* Standard includes. */
#include <string.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* Library includes. */
#include "stm32f10x_it.h"

/* Demo app includes. */
#include "flash.h"
#include "partest.h"

/* Driver includes. */
#include "STM32_USART.h"
#include "epd.h"


/* The time between cycles of the 'check' task - which depends on whether the
check task has detected an error or not. */
#define mainCHECK_DELAY_NO_ERROR			( ( TickType_t ) 5000 / portTICK_PERIOD_MS )
#define mainCHECK_DELAY_ERROR				( ( TickType_t ) 500 / portTICK_PERIOD_MS )

/* The LED controlled by the 'check' task. */
#define mainCHECK_LED						( 3 )

/* Task priorities. */
#define mainSEM_TEST_PRIORITY				( tskIDLE_PRIORITY + 1 )
#define mainBLOCK_Q_PRIORITY				( tskIDLE_PRIORITY + 2 )
#define mainCHECK_TASK_PRIORITY				( tskIDLE_PRIORITY + 3 )
#define mainFLASH_TASK_PRIORITY				( tskIDLE_PRIORITY + 2 )
#define mainECHO_TASK_PRIORITY				( tskIDLE_PRIORITY + 1 )
#define mainINTEGER_TASK_PRIORITY           ( tskIDLE_PRIORITY )
#define mainGEN_QUEUE_TASK_PRIORITY			( tskIDLE_PRIORITY )

/* COM port and baud rate used by the echo task. */
#define comBTM							       ( 1 )
#define mainBAUD_RATE						( 57600 )

/*-----------------------------------------------------------*/

/*
 * Configure the hardware for the demo.
 */
static void prvSetupHardware( void );

/* The 'check' task as described at the top of this file. */
static void prvCheckTask( void *pvParameters );

/* A simple task that echoes all the characters that are received on COM0
(USART1). */
static void prvUSARTEchoTask( void *pvParameters );

void epdDemoU8GTask(void *pvParameters);

/*-----------------------------------------------------------*/
/* Global variables */

u8g_t u8g;

char hello_text[4][32] = {"Hello World!", "", "", ""};

QueueHandle_t toDisplayStrQueue = NULL;


void assert_failed( char *pucFile, unsigned long ulLine );

/*-----------------------------------------------------------*/

int main( void )
{
#ifdef DEBUG
  debug();
#endif

	/* Set up the clocks and memory interface. */
	prvSetupHardware();

    toDisplayStrQueue = xQueueCreate(16, sizeof(char *));

	/* Start the standard demo tasks.  These are just here to exercise the
	kernel port and provide examples of how the FreeRTOS API can be used. */
	// vStartBlockingQueueTasks( mainBLOCK_Q_PRIORITY );
    // vStartSemaphoreTasks( mainSEM_TEST_PRIORITY );
    // vStartIntegerMathTasks( mainINTEGER_TASK_PRIORITY );
    // vStartGenericQueueTasks( mainGEN_QUEUE_TASK_PRIORITY );
	// vStartLEDFlashTasks( mainFLASH_TASK_PRIORITY );
    // vStartQueuePeekTasks();
    // vStartRecursiveMutexTasks();

	/* Create the 'echo' task, which is also defined within this file. */
    xTaskCreate( prvUSARTEchoTask, "Echo", configMINIMAL_STACK_SIZE, NULL, mainECHO_TASK_PRIORITY, NULL );

    // epd Task
    // xTaskCreate( epdShowPicturesTask, "EPD", configMINIMAL_STACK_SIZE, NULL, mainECHO_TASK_PRIORITY, NULL );
	xTaskCreate( epdDemoU8GTask, "EPDU8G", configMINIMAL_STACK_SIZE, NULL, mainECHO_TASK_PRIORITY, NULL );

	/* Create the 'check' task, which is also defined within this file. */
	// xTaskCreate( prvCheckTask, "Check", configMINIMAL_STACK_SIZE, NULL, mainCHECK_TASK_PRIORITY, NULL );

    /* Start the scheduler. */
	vTaskStartScheduler();

    /* Will only get here if there was insufficient memory to create the idle
    task.  The idle task is created within vTaskStartScheduler(). */
	for( ;; );
}

/*-----------------------------------------------------------*/

/* Described at the top of this file. */
static void prvCheckTask( void *pvParameters )
{
TickType_t xLastExecutionTime;
unsigned long ulTicksToWait = mainCHECK_DELAY_NO_ERROR;

	/* Just to remove the compiler warning about the unused parameter. */
	( void ) pvParameters;

	/* Initialise the variable used to control our iteration rate prior to
	its first use. */
	xLastExecutionTime = xTaskGetTickCount();

	for( ;; )
	{
		/* Wait until it is time to run the tests again. */
		vTaskDelayUntil( &xLastExecutionTime, ulTicksToWait );
#if 0
		/* Has an error been found in any task? */
		if( xAreGenericQueueTasksStillRunning() != pdTRUE )
		{
			/* Reduce the time between cycles of this task - which has the
			effect of increasing the rate at which the 'check' LED toggles to
			indicate the existence of an error to an observer. */
			ulTicksToWait = mainCHECK_DELAY_ERROR;
		}
		else if( xAreQueuePeekTasksStillRunning() != pdTRUE )
		{
			ulTicksToWait = mainCHECK_DELAY_ERROR;
		}
		else if( xAreBlockingQueuesStillRunning() != pdTRUE )
		{
			ulTicksToWait = mainCHECK_DELAY_ERROR;
		}
	    else if( xAreSemaphoreTasksStillRunning() != pdTRUE )
	    {
	        ulTicksToWait = mainCHECK_DELAY_ERROR;
	    }
	    else if( xAreIntegerMathsTaskStillRunning() != pdTRUE )
	    {
	        ulTicksToWait = mainCHECK_DELAY_ERROR;
	    }
	    else if( xAreRecursiveMutexTasksStillRunning() != pdTRUE )
	    {
	    	ulTicksToWait = mainCHECK_DELAY_ERROR;
	    }
#endif
		// vParTestToggleLED( mainCHECK_LED );
	}
}
/*-----------------------------------------------------------*/

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

#if 0
    char cChar;

    xSerialGetChar( comBTM, &cChar, portMAX_DELAY );
    if (cChar != 'O')
        return 1;
    
    xSerialGetChar( comBTM, &cChar, portMAX_DELAY );
    if (cChar != 'K')
        return 2;

#if 0
    xSerialGetChar( comBTM, &cChar, portMAX_DELAY );
    if (cChar != '\r')
        return 3;
#endif

    xSerialGetChar( comBTM, &cChar, portMAX_DELAY );
    if (cChar != '\n')
        return 4;
#endif
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
static void prvUSARTEchoTask( void *pvParameters )
{
    char cChar;

/* String declared static to ensure it does not end up on the stack, no matter
what the optimisation level. */
static const char *pcLongishString =
"ABBA was a Swedish pop music group formed in Stockholm in 1972, consisting of Anni-Frid Frida Lyngstad, "
"Björn Ulvaeus, Benny Andersson and Agnetha Fältskog. Throughout the band's existence, Fältskog and Ulvaeus "
"were a married couple, as were Lyngstad and Andersson - although both couples later divorced. They became one "
"of the most commercially successful acts in the history of popular music, and they topped the charts worldwide "
"from 1972 to 1983.  ABBA gained international popularity employing catchy song hooks, simple lyrics, sound "
"effects (reverb, phasing) and a Wall of Sound achieved by overdubbing the female singers' voices in multiple "
"harmonies. As their popularity grew, they were sought after to tour Europe, Australia, and North America, drawing "
"crowds of ardent fans, notably in Australia. Touring became a contentious issue, being particularly cumbersome for "
"Fältskog, but they continued to release studio albums to widespread commercial success. At the height of their "
"popularity, however, both relationships began suffering strain that led ultimately to the collapse of first the "
"Ulvaeus-Fältskog marriage (in 1979) and then of the Andersson-Lyngstad marriage in 1981. In the late 1970s and early "
"1980s these relationship changes began manifesting in the group's music, as they produced more thoughtful, "
"introspective lyrics with different compositions.";

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

    int k = 0, m = 0;
    char *buf = NULL;

	for( ;; )
	{
		/* Block to wait for a character to be received on COM0. */
		xSerialGetChar( comBTM, &cChar, portMAX_DELAY );

		/* Write the received character back to COM0. */
		xSerialPutChar( comBTM, cChar, 0 );

        if (!buf) {
            buf = pvPortMalloc(sizeof(char) * 32);
            k = itostr(buf, 32, RTC_GetCounter());
        }

        buf[k++] = cChar;
        
        if (cChar == '\r' || k >= 30) {
            buf[k] = '\0';
            if (xQueueSend(toDisplayStrQueue, &buf, 0) == pdTRUE) {
                // ok; will alloc new buffer
                buf = NULL;
            } else {
                // fail; ignore, keep buffer
            }
            k = 0;
            xSerialPutChar( comBTM, '\n', 0 );
        }

#if 0
        hello_text[m][k++] = cChar;

        if (cChar == '\r' || k >= 30) {
            hello_text[m][k] = 0;
            k = 0;
            if (++m > 3) {
                m = 0;
            }
            xSerialPutChar( comBTM, '\n', 0 );
        }
#endif
	}
}
/*-----------------------------------------------------------*/

static void prvSetupHardware( void )
{
	/* RCC system reset(for debug purpose). */
	RCC_DeInit ();

    /* Enable HSE. */
	RCC_HSEConfig( RCC_HSE_ON );

	/* Wait till HSE is ready. */
	while (RCC_GetFlagStatus(RCC_FLAG_HSERDY) == RESET);

    /* HCLK = SYSCLK. */
	RCC_HCLKConfig( RCC_SYSCLK_Div1 );

    /* PCLK2  = HCLK. */
	RCC_PCLK2Config( RCC_HCLK_Div1 );

    /* PCLK1  = HCLK/2. */
    RCC_PCLK1Config( RCC_HCLK_Div2 );
	// RCC_PCLK1Config( RCC_HCLK_Div1 );

	/* ADCCLK = PCLK2/4. */
	RCC_ADCCLKConfig( RCC_PCLK2_Div4 );

    /* Flash 2 wait state. */
	*( volatile unsigned long  * )0x40022000 = 0x01;

	/* PLLCLK = 8MHz * 9 = 72 MHz */
    // RCC_PLLConfig( RCC_PLLSource_HSE_Div1, RCC_PLLMul_9 );   //orig
    // RCC_PLLConfig( RCC_PLLSource_HSI_Div2, RCC_PLLMul_9 );     // 8/2 * 9 = 36 MHz
	RCC_PLLConfig( RCC_PLLSource_PREDIV1, RCC_PLLMul_9 );     // 8 * 9 = 72 MHz

    /* Enable PLL. */
	RCC_PLLCmd( ENABLE );

	/* Wait till PLL is ready. */
	while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);

	/* Select PLL as system clock source. */
	RCC_SYSCLKConfig (RCC_SYSCLKSource_PLLCLK);

	/* Wait till PLL is used as system clock source. */
	while (RCC_GetSYSCLKSource() != 0x08);

	/* Enable GPIOA, GPIOB, GPIOC, GPIOD, GPIOE and AFIO clocks */
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB |RCC_APB2Periph_GPIOC
							| RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE | RCC_APB2Periph_AFIO, ENABLE );

	/* Set the Vector Table base address at 0x08000000. */
	NVIC_SetVectorTable( NVIC_VectTab_FLASH, 0x0 );

	NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 );

	/* Configure HCLK clock as SysTick clock source. */
	SysTick_CLKSourceConfig( SysTick_CLKSource_HCLK );

    /* Update the SystemCoreClock variable based on current register settings */
    SystemCoreClockUpdate();

	/* Initialise the IO used for the LED outputs. */
	// vParTestInitialise();

    /* Init SPI and GPIO for EPD */
    epdInitInterface();

	/* APB1 Periph clock enable */
    // FIXME: RCC_APB1Periph_SPI2 not used?
	RCC_APB1PeriphClockCmd( RCC_APB1Periph_SPI2 | RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE );

    /* RTC */

    /* Enable access to backup registers and RTC */
    PWR_BackupAccessCmd(ENABLE);

    /* Reset backup domain; this allows one-time configuration of RTC clock below. */
    BKP_DeInit();
    /* Enable LSE extenal xtal osci */
    RCC_LSEConfig(RCC_LSE_ON);
    /* wait until LSE xtal osci is stable */
    while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET) { }
    /* Set RTC clock source = external 32.768kHz xtal */
    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
    // RCC_RTCCLKConfig(RCC_RTCCLKSource_HSE_Div128);
    /* Enable RTC clock after it has been selected */
    RCC_RTCCLKCmd(ENABLE);

    RTC_ExitConfigMode();
    RTC_WaitForSynchro();
    RTC_WaitForLastTask();


    // RTC_ITConfig()
    RTC_SetPrescaler(32768);


    RTC_WaitForLastTask();
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
	/* This function will get called if a task overflows its stack.   If the
	parameters are corrupt then inspect pxCurrentTCB to find which was the
	offending task. */

	( void ) pxTask;
	( void ) pcTaskName;

	for( ;; );
}
/*-----------------------------------------------------------*/

void assert_failed( char *pucFile, unsigned long ulLine )
{
	( void ) pucFile;
	( void ) ulLine;

	for( ;; );
}

/*-----------------------------------------------------------*/

void draw(int pos)
{
    u8g_SetFont(&u8g, u8g_font_helvR12);
    u8g_DrawStr(&u8g,  0, 15+15*0, hello_text[0]);
    u8g_DrawStr(&u8g,  0, 15+15*1, hello_text[1]);
    u8g_DrawStr(&u8g,  0, 15+15*2, hello_text[2]);
    u8g_DrawStr(&u8g,  0, 15+15*3, hello_text[3]);
    // u8g_DrawStr(&u8g,  0, 12+10*pos, "A B C D E F");
    u8g_DrawLine(&u8g, 150, 35, 170, 45);
    u8g_DrawCircle(&u8g, 150, 35, 20, U8G_DRAW_ALL);
}

void epdDemoU8GTask(void *pvParameters)
{
    u8g_InitComFn(&u8g, &u8g_dev_ssd1606_172x72_hw_spi, u8g_com_null_fn);
    u8g_SetDefaultForegroundColor(&u8g);

    int pos = 0;
    char *buf = NULL;

    for (;;) {
        /* picture loop */
        u8g_FirstPage(&u8g);
        do {
            draw(pos);
        } while ( u8g_NextPage(&u8g) );

        /* refresh screen after some delay */
        // vTaskDelay(( ( TickType_t ) 2000 / portTICK_PERIOD_MS ));

        if (xQueueReceive(toDisplayStrQueue, &buf, portMAX_DELAY) == pdTRUE) {
            int i;
            for (i = 0; i < 3; ++i) {
                strncpy(hello_text[i], hello_text[i+1], 32);
            }
            strncpy(hello_text[3], buf, 32);
            vPortFree(buf);
            buf = NULL;
        }

        /* update position */
        pos++;
        pos &= 3;
    }  
}
