#ifndef _EPD_H
#define _EPD_H

#include <u8g.h>

#define WIDTH           172
#define HEIGHT          72

extern u8g_dev_t u8g_dev_ssd1606_172x72_hw_spi;


/* screen update range */
extern int epd_updrange_x1; 
extern int epd_updrange_x2;


void epdInitInterface(void);

void epdInitSSD1606(void);


#endif
