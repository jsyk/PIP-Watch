#ifndef BATTERY_H
#define BATTERY_H

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

void BatteryTask(void *pvParameters);

void ADC_IRQHandler(void);

#endif
