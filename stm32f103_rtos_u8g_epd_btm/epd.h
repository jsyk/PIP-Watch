#ifndef _EPD_H
#define _EPD_H

#include <u8g.h>

extern u8g_dev_t u8g_dev_ssd1606_172x72_hw_spi;

void epdInitInterface(void);

void epdInitSSD1606(void);

void epdShowPicturesTask(void *pvParameters);

#endif
