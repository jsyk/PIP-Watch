/**
  ******************************************************************************
  * File Name          : main.c
  * Date               : 13/06/2014 13:16:23
  * Description        : Main program body
  ******************************************************************************
  *
  * COPYRIGHT(c) 2014 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
// #include "stm32f1xx_hal.h"
#include <unistd.h>
#include "gpio.h"
#include "stm32f10x_spi.h"
#include "epd_imgs.h"
#include "fontgen.h"

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

extern font_t font_FreeUniversal_Book;

#define DELAY       1000000

#define EPD_Pin_nRES      GPIO_Pin_2
#define EPD_Pin_nCS      GPIO_Pin_4
#define EPD_Pin_DC      GPIO_Pin_3
#define EPD_Pin_BUSY    GPIO_Pin_6

#define EPD_DATA        1
#define EPD_COMMAND     0

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

void somedelay(int tm)
{
    volatile int cnt = tm;
    while (--cnt > 0) {
    }
}

void epd_wait_nbusy(void)
{
    while (GPIO_ReadInputDataBit(GPIOA, EPD_Pin_BUSY) == Bit_SET) { }
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
        GPIO_SetBits(GPIOA, EPD_Pin_DC);        // DC=1  Data
    } else {
        // as command
        GPIO_ResetBits(GPIOA, EPD_Pin_DC);        // DC=0  Cmd
    }
    // send byte
    SPI_I2S_SendData(SPI1, dt);

    // arghhh!!
    somedelay(100);

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

void epd_set_ncs(FunctionalState NewState)
{
    if (NewState != DISABLE) {
        // enable
        GPIO_ResetBits(GPIOA, EPD_Pin_nCS);       // nCS=0 enable
    } else {
        // disable
        GPIO_SetBits(GPIOA, EPD_Pin_nCS);       // nCS=1 disable
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

    // data bytes...
#if 1
    // send Cmd 24: write data
    epd_wait_nbusy();
    epd_set_ncs(ENABLE);
    epd_sendbyte(EPD_COMMAND,   0x24);
    for (i = 0; i < 3096; i++) {
        epd_sendbyte(EPD_DATA,      gImage[i]);
        // epd_sendbyte(EPD_DATA,      gImage_chessboard1[i]);      // nejake cislo
        // epd_sendbyte(EPD_DATA,      gImage_eluosi[i]);           // gradient
        // epd_sendbyte(EPD_DATA,      gImage_gongsi[i]);           // lopata?
        // epd_sendbyte(EPD_DATA,      gImage_hanwen[i]);          // cinstina
        // epd_sendbyte(EPD_DATA,      gImage_nokiac5[i]);          // nokia price tag
        // epd_sendbyte(EPD_DATA,      gImage_OED[i]);          // 
        // epd_sendbyte(EPD_DATA,      gImage_oedbook[i]);          // 
        // epd_sendbyte(EPD_DATA,      gImage_riwen[i]);          // 
        // epd_sendbyte(EPD_DATA,      gImage_shousi[i]);          // 
        // epd_sendbyte(EPD_DATA,      gImage_SUNING[i]);           // price tag
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

int main(void)
{
    int i;

    /* USER CODE BEGIN 1 */

    /* USER CODE END 1 */

    /* MCU Configuration----------------------------------------------------------*/

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

    GPIO_ResetBits(GPIOA, EPD_Pin_nCS);       // nCS=0 enable
    GPIO_SetBits(GPIOA, EPD_Pin_DC);        // DC=1  Data
    GPIO_ResetBits(GPIOA, EPD_Pin_nRES);        // nRES enable
    somedelay(DELAY);

    GPIO_SetBits(GPIOA, EPD_Pin_nRES);        // nRES disable
    somedelay(DELAY);

    GPIO_SetBits(GPIOA, EPD_Pin_nCS);       // nCS=1 disable
    somedelay(DELAY);

    somedelay(font_FreeUniversal_Book.nglyphs);




#if 0
    GPIO_ResetBits(GPIOA, EPD_Pin_DC);        // DC=0  Cmd
    GPIO_ResetBits(GPIOA, EPD_Pin_nCS);       // nCS=0 enable
    SPI_I2S_SendData(SPI1, 0x20);           // Command 0x20: Master activation

    // while (!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE)) {
    // }
    // while (!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE)) {
    // }
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY)) {
    }
    // (void)SPI_I2S_ReceiveData(SPI1);
    somedelay(1000);
    GPIO_SetBits(GPIOA, EPD_Pin_nCS);       // nCS=1 disable
#endif


    volatile int cnt = DELAY;
    i = 1;
    int k = 0;
    while (1) {
        cnt = DELAY;
        while (--cnt > 0) {
            // if (!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) && SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE)) {
            //     // not busy
            //     SPI_SSOutputCmd(SPI1, DISABLE);
            // }
        }
        
        if (--i == 0) {
            i = 5;
            epd_draw_screen(gImg_list[k]);
            if (gImg_list[++k] == NULL) {
                k = 0;
            }
        }

#if 0
        if (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE)) {
            SPI_SSOutputCmd(SPI1, ENABLE);
            SPI_I2S_SendData(SPI1, d);
            GPIO_Write(GPIOA, d ^ 0x02);
        }
#endif
#if 0
        // SPI_Cmd(SPI1, ENABLE);
        // GPIO_ResetBits(GPIOA, EPD_Pin_DC);        // DC=0  Cmd
        // SPI_SSOutputCmd(SPI1, ENABLE);
        somedelay(1000);
        GPIO_ResetBits(GPIOA, EPD_Pin_nCS);       // nCS=1 disable
        SPI_I2S_SendData(SPI1, 0x20);           // Command 0x20: Master activation

        // while (!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE)) {
        // }
        // while (!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE)) {
        // }
        while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY)) {
        }
        // (void)SPI_I2S_ReceiveData(SPI1);
        somedelay(1000);
        GPIO_SetBits(GPIOA, EPD_Pin_nCS);       // nCS=1 disable

        // SPI_SSOutputCmd(SPI1, DISABLE);
#endif

        // invert LED
        if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1)) {
            GPIO_ResetBits(GPIOA, GPIO_Pin_1);
        } else {
            GPIO_SetBits(GPIOA, GPIO_Pin_1);
        }
        // uint16_t d = GPIO_ReadOutputData(GPIOA);
        // GPIO_Write(GPIOA, d ^ 0x02);
    }

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

}

#endif

/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
