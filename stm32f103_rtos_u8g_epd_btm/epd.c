/* Includes ------------------------------------------------------------------*/
// #include "stm32f1xx_hal.h"
#include <unistd.h>
#include <FreeRTOS.h>
#include "stm32f10x_spi.h"
#include "epd.h"
#include "task.h"
#include "epd_imgs.h"

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

// extern font_t font_FreeUniversal_Book;

#define DELAY       1000000

#define EPD_Pin_nRES      GPIO_Pin_2
#define EPD_Port_nRES       GPIOB

#define EPD_Pin_nCS      GPIO_Pin_4
#define EPD_Port_nCS        GPIOA

#define EPD_Pin_DC      GPIO_Pin_10
#define EPD_Port_DC     GPIOB

#define EPD_Pin_BUSY    GPIO_Pin_6
#define EPD_Port_BUSY   GPIOA

#define EPD_DATA        1
#define EPD_COMMAND     0

#define WIDTH           172
#define HEIGHT          72

#if 0
#define WIDTH           72
#define HEIGHT          172
#endif

#define PAGE_HEIGHT     HEIGHT


const unsigned char epd_lut_init_data[90] = {
    0x00,0x00,0x00,0x55,0x00,0x00,0x55,0x55,
    0x00,0x55,0x55,0x55,0x55,0x55,0x55,0x55,
    0x55,0xAA,0x55,0x55,0xAA,0xAA,0xAA,0xAA,
    0x05,0x05,0x05,0x05,0x15,0x15,0x15,0x15,
    0x01,0x01,0x01,0x01,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x34,0x32,0xF1,0x74,0x14,0x00,0x00,0x00,
    0x00,0x00,
};

const unsigned char *gImg_list[] = {
    gImage_chessboard1,
    gImage_eluosi,
    gImage_gongsi,
    gImage_hanwen,
    gImage_nokiac5,
    gImage_OED,
    gImage_oedbook,
    gImage_riwen,
    gImage_shousi,
    gImage_SUNING,
    NULL            // end of list marker
};


uint8_t bitreverse(uint8_t b)
{
    b = ((b * 0x0802LU & 0x22110LU) | (b * 0x8020LU & 0x88440LU)) * 0x10101LU >> 16; 
    return b;
}

static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;

  /** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
  */

  /*Enable or disable APB2 peripheral clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

  /*Configure GPIO pin : PA */
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_4;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PA */
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PB */
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2|GPIO_Pin_10;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStruct);

  /** SPI1 GPIO Configuration  
  PA5   ------> SPI1_SCK
  PA7   ------> SPI1_MOSI
  */

  /*Enable or disable APB2 peripheral clock */
  // RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

  /*Configure GPIO pin : PA */
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5|GPIO_Pin_7;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStruct);

  /** USART1 GPIO Configuration  
  PA9   ------> USART1_TX
  PA10   ------> USART1_RX
  PA11   ------> USART1_CTS
  PA12   ------> USART1_RTS
  */

  /*Enable or disable APB2 peripheral clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

  /*Configure GPIO pin : PA */
  /*
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(GPIOA, &GPIO_InitStruct);
  */
}



void epd_wait_nbusy(void)
{
    while (GPIO_ReadInputDataBit(EPD_Port_BUSY, EPD_Pin_BUSY)) { }
}

void epd_sendbyte(uint8_t dc, uint8_t dt)
{
    // wait until SPI is idle
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY)) {
    }
    while (!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE)) {
    }
    // set command/data flag
    if (dc == EPD_DATA) {
        // as data
        GPIO_SetBits(EPD_Port_DC, EPD_Pin_DC);        // DC=1  Data
    } else {
        // as command
        GPIO_ResetBits(EPD_Port_DC, EPD_Pin_DC);        // DC=0  Cmd
    }
    // send byte
    SPI_I2S_SendData(SPI1, dt);

    // arghhh!!
    // somedelay(100);

    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY)) {
    }
    while (!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE)) {
    }
    // wait until physically done (RX buffer is empty)
    // while (!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE)) {
    // }
    // // dummy read
    // SPI_I2S_ReceiveData(SPI1);
}

void epd_sendbyte_raw(uint8_t dt)
{
    // wait until SPI is idle
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY)) {
    }
    while (!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE)) {
    }
    // send byte
    SPI_I2S_SendData(SPI1, dt);

    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY)) {
    }
    while (!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE)) {
    }
}


void epd_set_ncs(FunctionalState NewState)
{
    if (NewState != DISABLE) {
        // enable
        GPIO_ResetBits(EPD_Port_nCS, EPD_Pin_nCS);       // nCS=0 enable
    } else {
        // disable
        GPIO_SetBits(EPD_Port_nCS, EPD_Pin_nCS);       // nCS=1 disable
    }
}


void epd_draw_screen(const unsigned char *gImage)
{
    int i;

#if 1
    // send Cmd 10: Exit deep sleep mode
    epd_wait_nbusy();
    epd_set_ncs(ENABLE);
    epd_sendbyte(EPD_COMMAND,   0x10);
    epd_sendbyte(EPD_DATA,      0x00);
    epd_set_ncs(DISABLE);
    epd_wait_nbusy();
#endif


#if 1
    // send Cmd 01: Panel config
    epd_wait_nbusy();
    epd_set_ncs(ENABLE);
    epd_sendbyte(EPD_COMMAND,   0x01);
    epd_sendbyte(EPD_DATA,      0xB3);      // 179+1 horizontal pixels.
    epd_sendbyte(EPD_DATA,      0x00);      // GD=SM=TB=0
    epd_set_ncs(DISABLE);
    epd_wait_nbusy();
#endif

#if 1
    // send Cmd 03: VGH/VGL Set
    epd_wait_nbusy();
    epd_set_ncs(ENABLE);
    epd_sendbyte(EPD_COMMAND,   0x03);
    epd_sendbyte(EPD_DATA,      0xEA);
    epd_set_ncs(DISABLE);
    epd_wait_nbusy();
#endif

#if 1
    // send Cmd 04: VSH/VSL Set
    epd_wait_nbusy();
    epd_set_ncs(ENABLE);
    epd_sendbyte(EPD_COMMAND,   0x04);
    epd_sendbyte(EPD_DATA,      0x0A);
    epd_set_ncs(DISABLE);
    epd_wait_nbusy();
#endif

#if 1
    // send Cmd 3A: set dummy line pulse period
    epd_wait_nbusy();
    epd_set_ncs(ENABLE);
    epd_sendbyte(EPD_COMMAND,   0x3A);
    epd_sendbyte(EPD_DATA,      0x04);
    epd_set_ncs(DISABLE);
    epd_wait_nbusy();
#endif

#if 1
    // send Cmd 3B: set gate line width
    epd_wait_nbusy();
    epd_set_ncs(ENABLE);
    epd_sendbyte(EPD_COMMAND,   0x3B);
    epd_sendbyte(EPD_DATA,      0x08);
    epd_set_ncs(DISABLE);
    epd_wait_nbusy();
#endif

#if 1
    // send Cmd 2C: write vcom reg
    epd_wait_nbusy();
    epd_set_ncs(ENABLE);
    epd_sendbyte(EPD_COMMAND,   0x2C);
    epd_sendbyte(EPD_DATA,      0xA0);
    epd_set_ncs(DISABLE);
    epd_wait_nbusy();
#endif

#if 1
    // send Cmd 3C: select border waveform
    epd_wait_nbusy();
    epd_set_ncs(ENABLE);
    epd_sendbyte(EPD_COMMAND,   0x3C);
    epd_sendbyte(EPD_DATA,      0x63);
    epd_set_ncs(DISABLE);
    epd_wait_nbusy();
#endif

    // data entry...

#if 0
#if 1
    // send Cmd 11: Data entry mode
    epd_wait_nbusy();
    epd_set_ncs(ENABLE);
    epd_sendbyte(EPD_COMMAND,   0x11);
    epd_sendbyte(EPD_DATA,      0x03);
    epd_set_ncs(DISABLE);
    epd_wait_nbusy();
#endif

#if 1
    // send Cmd 44: X-ram address start
    epd_wait_nbusy();
    epd_set_ncs(ENABLE);
    epd_sendbyte(EPD_COMMAND,   0x44);
    epd_sendbyte(EPD_DATA,      0x00);      // XStart=00
    epd_sendbyte(EPD_DATA,      0x11);      // XEnd=17; RAM x address end at 11h(17)->72: [because 1F(31)->128 and 12(18)->76] 
    epd_set_ncs(DISABLE);
    epd_wait_nbusy();
#endif

#if 1
    // send Cmd 45: Y-ram start
    epd_wait_nbusy();
    epd_set_ncs(ENABLE);
    epd_sendbyte(EPD_COMMAND,   0x45);
    epd_sendbyte(EPD_DATA,      0x00);  // YStart=0
    epd_sendbyte(EPD_DATA,      0xAB);  // YEnd=171; RAM y address start at ABh(171)->172: [because B3(179)->180]
    epd_set_ncs(DISABLE);
    epd_wait_nbusy();
#endif

#if 1
    // send Cmd 4E: RAM X address cnt
    epd_wait_nbusy();
    epd_set_ncs(ENABLE);
    epd_sendbyte(EPD_COMMAND,   0x4E);
    epd_sendbyte(EPD_DATA,      0x00);
    epd_set_ncs(DISABLE);
    epd_wait_nbusy();
#endif

#if 1
    // send Cmd 4F: RAM Y address cnt
    epd_wait_nbusy();
    epd_set_ncs(ENABLE);
    epd_sendbyte(EPD_COMMAND,   0x4F);
    // epd_sendbyte(EPD_DATA,      0xAB);
    epd_sendbyte(EPD_DATA,      0x00);
    epd_set_ncs(DISABLE);
    epd_wait_nbusy();
#endif
#endif

#if 1
#if 1
    // send Cmd 11: Data entry mode
    epd_wait_nbusy();
    epd_set_ncs(ENABLE);
    epd_sendbyte(EPD_COMMAND,   0x11);
    // epd_sendbyte(EPD_DATA,      0x07);
    epd_sendbyte(EPD_DATA,      0x05);
    epd_set_ncs(DISABLE);
    epd_wait_nbusy();
#endif

#if 1
    // send Cmd 44: X-ram address start
    epd_wait_nbusy();
    epd_set_ncs(ENABLE);
    epd_sendbyte(EPD_COMMAND,   0x44);
    epd_sendbyte(EPD_DATA,      0x00);      // XStart=00
    epd_sendbyte(EPD_DATA,      0x11);      // XEnd=17; RAM x address end at 11h(17)->72: [because 1F(31)->128 and 12(18)->76] 
    epd_set_ncs(DISABLE);
    epd_wait_nbusy();
#endif

#if 1
    // send Cmd 45: Y-ram start
    epd_wait_nbusy();
    epd_set_ncs(ENABLE);
    epd_sendbyte(EPD_COMMAND,   0x45);
#if 0
    epd_sendbyte(EPD_DATA,      0x00);  // YStart=0
    epd_sendbyte(EPD_DATA,      0xAB);  // YEnd=171; RAM y address start at ABh(171)->172: [because B3(179)->180]
#endif
    epd_sendbyte(EPD_DATA,      0xAB);  // YStart=0
    epd_sendbyte(EPD_DATA,      0x00);  // YEnd=171; RAM y address start at ABh(171)->172: [because B3(179)->180]
    epd_set_ncs(DISABLE);
    epd_wait_nbusy();
#endif

#if 1
    // send Cmd 4E: RAM X address cnt
    epd_wait_nbusy();
    epd_set_ncs(ENABLE);
    epd_sendbyte(EPD_COMMAND,   0x4E);
    epd_sendbyte(EPD_DATA,      0x00);
    epd_set_ncs(DISABLE);
    epd_wait_nbusy();
#endif

#if 1
    // send Cmd 4F: RAM Y address cnt
    epd_wait_nbusy();
    epd_set_ncs(ENABLE);
    epd_sendbyte(EPD_COMMAND,   0x4F);
    // epd_sendbyte(EPD_DATA,      0x00);
    epd_sendbyte(EPD_DATA,      0xAB);
    epd_set_ncs(DISABLE);
    epd_wait_nbusy();
#endif
#endif


    // data bytes...
#if 1
    // send Cmd 24: write data
    epd_wait_nbusy();
    epd_set_ncs(ENABLE);
    epd_sendbyte(EPD_COMMAND,   0x24);
    for (i = 0; i < 3096; i++) {
#if 1
        epd_sendbyte(EPD_DATA,      bitreverse(gImage[i]) ^ 0xFF);
#endif
        // epd_sendbyte(EPD_DATA,      (gImage[i]) ^ 0xFF);
    }
#endif

#if 1
    // send Cmd F0: Internal feedback selection
    epd_wait_nbusy();
    epd_set_ncs(ENABLE);
    epd_sendbyte(EPD_COMMAND,   0xF0);
    epd_sendbyte(EPD_DATA,      0x1F);
    epd_set_ncs(DISABLE);
    epd_wait_nbusy();
#endif

#if 1
    // send Cmd 0x21: option for display update
    epd_wait_nbusy();
    epd_set_ncs(ENABLE);
    epd_sendbyte(EPD_COMMAND,   0x21);
    epd_sendbyte(EPD_DATA,      0x03);      // InitialUpdate_SourceControl=3=GS0,GS3
    epd_set_ncs(DISABLE);
    epd_wait_nbusy();
#endif

#if 1
    // send Cmd 0x22: sequence for display update
    epd_wait_nbusy();
    epd_set_ncs(ENABLE);
    epd_sendbyte(EPD_COMMAND,   0x22);
    epd_sendbyte(EPD_DATA,      0xC4);      // enable clock, enable CP, display pattern
    epd_set_ncs(DISABLE);
    epd_wait_nbusy();
#endif

#if 1
    // send Cmd 0x32: write LUT register
    epd_wait_nbusy();
    epd_set_ncs(ENABLE);
    epd_sendbyte(EPD_COMMAND,   0x32);
    for(i = 0; i < 90; i++) {
        epd_sendbyte(EPD_DATA,  epd_lut_init_data[i]);
    }
    epd_set_ncs(DISABLE);
    epd_wait_nbusy();
#endif



#if 1
    // send Cmd 0x20: Master activation - redraw the screen
    epd_wait_nbusy();
    epd_set_ncs(ENABLE);
    epd_sendbyte(EPD_COMMAND,   0x20);
    epd_set_ncs(DISABLE);
    epd_wait_nbusy();
#endif


#if 1
    // send Cmd 0x22: sequence for display update; Booster disable
    epd_wait_nbusy();
    epd_set_ncs(ENABLE);
    epd_sendbyte(EPD_COMMAND,   0x22);
    epd_sendbyte(EPD_DATA,      0x03);      // disable CP, disable Clock
    epd_set_ncs(DISABLE);
    epd_wait_nbusy();
#endif

#if 1
    // send Cmd 0x20: Master activation - booster disable
    epd_wait_nbusy();
    epd_set_ncs(ENABLE);
    epd_sendbyte(EPD_COMMAND,   0x20);
    epd_set_ncs(DISABLE);
    epd_wait_nbusy();
#endif
}

void epdInitInterface(void)
{
    /* Initialize all configured peripherals */
    MX_GPIO_Init();

    /*Enable or disable APB2 peripheral clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

    SPI_InitTypeDef SPI_init;
    SPI_StructInit(&SPI_init);
    SPI_init.SPI_Direction = SPI_Direction_1Line_Tx;  // SPI_Direction_2Lines_FullDuplex
    SPI_init.SPI_Mode = SPI_Mode_Master;
    SPI_init.SPI_DataSize = SPI_DataSize_8b;
    SPI_init.SPI_CPOL = SPI_CPOL_Low;   // ?
    SPI_init.SPI_CPHA = SPI_CPHA_1Edge; // ?
    SPI_init.SPI_NSS = SPI_NSS_Soft; //SPI_NSS_Hard;
    SPI_init.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
    SPI_init.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_init.SPI_CRCPolynomial = 0;   // ?
    SPI_Init(SPI1, &SPI_init);

    SPI_Cmd(SPI1, ENABLE);
    SPI_SSOutputCmd(SPI1, ENABLE);
}


void epdInitSSD1606(void)
{
    GPIO_ResetBits(EPD_Port_nCS, EPD_Pin_nCS);       // nCS=0 enable
    GPIO_SetBits(EPD_Port_DC, EPD_Pin_DC);        // DC=1  Data
    GPIO_ResetBits(EPD_Port_nRES, EPD_Pin_nRES);        // nRES enable
    vTaskDelay(( ( TickType_t ) 1 / portTICK_PERIOD_MS ));

    GPIO_SetBits(EPD_Port_nRES, EPD_Pin_nRES);        // nRES disable
    vTaskDelay(( ( TickType_t ) 1 / portTICK_PERIOD_MS ));

    GPIO_SetBits(EPD_Port_nCS, EPD_Pin_nCS);       // nCS=1 disable
    vTaskDelay(( ( TickType_t ) 1 / portTICK_PERIOD_MS ));
}

void epdShowPicturesTask(void *pvParameters)
{
    int i;

    /* USER CODE BEGIN 1 */

    /* USER CODE END 1 */

    /* MCU Configuration----------------------------------------------------------*/
    epdInitSSD1606();


    i = 1;
    int k = 0;
    while (1) {
        vTaskDelay(( ( TickType_t ) 1000 / portTICK_PERIOD_MS ));

        if (--i == 0) {
            i = 5;
            epd_draw_screen(gImg_list[k]);
            if (gImg_list[++k] == NULL) {
                k = 0;
            }
        }

        // invert LED
        if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1)) {
            GPIO_ResetBits(GPIOA, GPIO_Pin_1);
        } else {
            GPIO_SetBits(GPIOA, GPIO_Pin_1);
        }
    }

}

uint8_t u8g_com_hw_spi_fn(u8g_t *u8g, uint8_t msg, uint8_t arg_val, void *arg_ptr)
{
    switch(msg)
    {
        case U8G_COM_MSG_STOP:
            // Shut down display (not used at the moment)
            break;

        case U8G_COM_MSG_INIT:
            /* Setup and init the display. "arg_val" contains a hint about the possible SPI speed.
             arg_val: 
                U8G_SPI_CLK_CYCLE_50NS, U8G_SPI_CLK_CYCLE_300NS, U8G_SPI_CLK_CYCLE_400NS, U8G_SPI_CLK_CYCLE_NONE
                U8G_SPI_CLK_CYCLE_NONE has the largest value.
            */
            // epdInitInterface();
            u8g_MicroDelay();      
            break;

        case U8G_COM_MSG_ADDRESS:                     /* define cmd (arg_val = 0) or data mode (arg_val = 1) */
            /* Set the address line (or data/instruction input) of the display. 
                  arg_val: 
                    0: low level
                    1: high level
            */
            // set command/data flag
            if (arg_val == 1) {
                // 1: as data
                GPIO_SetBits(EPD_Port_DC, EPD_Pin_DC);        // DC=1  Data
            } else {
                // 0: as command
                GPIO_ResetBits(EPD_Port_DC, EPD_Pin_DC);        // DC=0  Cmd
            }
            // u8g_10MicroDelay();
            // set_gpio_level(u8g_pin_a0, arg_val);
            // u8g_10MicroDelay();
            break;

        case U8G_COM_MSG_CHIP_SELECT:
            /* Enable the controller chip.
                  arg_val:
                    0: no controller enabled
                    1: enable 1st controller
                    2: enable 2nd controller
                    3: enable 3rd controller
            */
            if ( arg_val == 0 ) {
                epd_set_ncs(DISABLE);
            } else {
                epd_set_ncs(ENABLE);
            }
            break;
      
        case U8G_COM_MSG_RESET:
            /*
                Set level for the reset input of the display.
              arg_val: 
                0: low level
                1: high level
            */
            if (arg_val == 0) {
                GPIO_ResetBits(EPD_Port_nRES, EPD_Pin_nRES);        // nRES enable
            } else {
                GPIO_SetBits(EPD_Port_nRES, EPD_Pin_nRES);        // nRES disable
            }
            u8g_10MicroDelay();
            break;
      
        case U8G_COM_MSG_WRITE_BYTE:
            /*
                Send single byte to the display.
              arg_val: 
                Value, that should be sent to the display.
            */
            epd_sendbyte_raw(arg_val);
            u8g_MicroDelay();
            break;

        case U8G_COM_MSG_WRITE_SEQ:
        case U8G_COM_MSG_WRITE_SEQ_P:
            /*
                Send a sequence of bytes to the display.
              arg_val: 
                Number of bytes to sent
              arg_ptr:
                Pointer to the memory location of the first value.
                For AVR controller, the memory location can be in 
                flash (message U8G_COM_MSG_WRITE_SEQ_P),
            */
            {
                uint8_t *ptr = arg_ptr;
                while( arg_val > 0 ) {
                    epd_sendbyte_raw(*ptr++);
                    arg_val--;
                }
            }
            break;
    }
    return 1;
}

uint8_t u8g_dev_ssd1606_172x72_fn(u8g_t *u8g, u8g_dev_t *dev, uint8_t msg, void *arg)
{
    switch(msg)
    {
        case U8G_DEV_MSG_INIT:
            // u8g_InitCom(u8g, dev, U8G_SPI_CLK_CYCLE_300NS);
            // u8g_WriteEscSeqP(u8g, dev, u8g_dev_ssd1306_128x32_init_seq);
            epdInitSSD1606();
            break;

        case U8G_DEV_MSG_STOP:
            break;

        case U8G_DEV_MSG_PAGE_NEXT: {
                u8g_pb_t *pb = (u8g_pb_t *)(dev->dev_mem);
                epd_draw_screen(pb->buf);
                break;
            }

        case U8G_DEV_MSG_SLEEP_ON:
            // u8g_WriteEscSeqP(u8g, dev, u8g_dev_ssd13xx_sleep_on);    
            return 1;

        case U8G_DEV_MSG_SLEEP_OFF:
            // u8g_WriteEscSeqP(u8g, dev, u8g_dev_ssd13xx_sleep_off);    
            return 1;
        
        // case U8G_DEV_MSG_GET_MODE:
        //     return U8G_MODE_GRAY2BIT;
    }
    // return u8g_dev_pb8h2_base_fn(u8g, dev, msg, arg);
    // return u8g_dev_pb16v2_base_fn(u8g, dev, msg, arg);
    return u8g_dev_pbxv2_base_fn(u8g, dev, msg, arg);
}

// U8G_PB_DEV(u8g_dev_ssd1606_172x72_hw_spi, WIDTH, HEIGHT, PAGE_HEIGHT, u8g_dev_ssd1606_172x72_fn, U8G_COM_HW_SPI);

/* local frame buffer memory, almost 4kB */
uint8_t u8g_dev_ssd1606_172x72_hw_spi_buf[WIDTH*HEIGHT/4] U8G_NOCOMMON ; 
/* frame/page buffer descriptor */
u8g_pb_t u8g_dev_ssd1606_172x72_hw_spi_pb = { {PAGE_HEIGHT, HEIGHT, 0, 0, 0},  WIDTH, u8g_dev_ssd1606_172x72_hw_spi_buf}; 
/* device descriptor */
u8g_dev_t u8g_dev_ssd1606_172x72_hw_spi = { u8g_dev_ssd1606_172x72_fn, &u8g_dev_ssd1606_172x72_hw_spi_pb, u8g_com_null_fn };
