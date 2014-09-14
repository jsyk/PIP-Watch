#ifndef BATTERY_H
#define BATTERY_H

/* minimum and maximum battery voltage in milivolts,
 * corresponding to 0 and 100% */
#define VBAT_0_PERCENT      3500
#define VBAT_100_PERCENT    4200


/* battery voltage in milivolts */
extern int vbat_measured;
extern int vbat_percent;

extern int temp_celsius;

void BatteryTask(void *pvParameters);

void ADC_IRQHandler(void);

#endif
