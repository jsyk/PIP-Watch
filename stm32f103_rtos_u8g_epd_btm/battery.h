#ifndef BATTERY_H
#define BATTERY_H

#define VBAT_0_PERCENT      350
#define VBAT_100_PERCENT    420


/* battery voltage, scaled by 100x: vbat*100 */
extern int vbat_measured;
extern int vbat_percent;

void BatteryTask(void *pvParameters);

void ADC_IRQHandler(void);

#endif
