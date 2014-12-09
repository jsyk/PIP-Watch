#ifndef BATTERY_H
#define BATTERY_H

#include "gui_window.h"
#include "utils.h"

/* minimum and maximum battery voltage in milivolts,
 * corresponding to 0 and 100% */
#define VBAT_0_PERCENT      3500
#define VBAT_100_PERCENT    4150

#define USBPOW_Port         GPIOA
#define USBPOW_Pin          GPIO_Pin_9

#define CHARGESTAT_Port     GPIOB
#define CHARGESTAT_Pin      GPIO_Pin_11


/* for batt_state: */
#define BATTERY_UNKNOWN         '?'
#define BATTERY_DISCHARGING     '-'
#define BATTERY_CHARGING        '+'
#define BATTERY_FULL            '='

/* battery voltage in milivolts */
extern int vbat_measured;
extern int vbat_percent;
extern char batt_state;

/* CPU temperature in celsius */
extern int temp_celsius;


/* OS Task - monitoring the battery voltage  */
void BatteryTask(void *pvParameters);

/* IRQ handler for the ADC interrupt */
void ADC_IRQHandler(void);


// --------------------------------------------------------------------------

/* battery symbol graphical dimensions */
#define DEFAULT_BATTERY_WIDTH       20
#define DEFAULT_BATTERY_HEIGHT      8


/* Graphical symbol of a battery */
struct GuiBattery {
    /* the window base class; MUST BE THE FIRST ELEMENT */
    struct GuiWindow win;

    /* battery: voltage, percantage of the charge, charging state */
	int vbat_measured;
	int percent;
	char batt_state;

};


/* drawing callback for battery */
int gui_battery_draw_cb(u8g_t *pu8g, struct GuiWindow *win,
                struct GuiPoint abspos);

/* allocate battery graphical symbol */
struct GuiBattery *gui_battery_alloc(void);


#endif
