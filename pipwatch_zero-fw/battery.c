#include "battery.h"
#include "gui.h"

/* Scheduler includes. */
#include "task.h"

/* Library includes. */
#include "stm32f10x_it.h"

/* queue of AD-converted data values received in irq handler */
QueueHandle_t adcValueQueue = NULL;

int vbat_measured = 0;
int vbat_percent = 0;
int temp_celsius = 0;
char batt_state = BATTERY_UNKNOWN;


/* milivolts on the temperature sensor ADC at 25 dgC */
#define VTEMP_25C           1430


void ADC_IRQHandler(void)
{
    long xHigherPriorityTaskWoken = pdFALSE;

    if (adcValueQueue != NULL && ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == SET) {
        int val = ADC_GetConversionValue(ADC1);
        xQueueSendFromISR(adcValueQueue, &val, &xHigherPriorityTaskWoken);
    }

    /* clear ADC1 irq pending bit */
    ADC_ClearITPendingBit(ADC1, ADC_IT_EOC);

    /* signal end-of-irq and possible reschedule point */
    portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
}

void BatteryTask(void *pvParameters)
{
    /* start the ADC calibration */
    ADC_StartCalibration(ADC1);
    do {
        /* wait 10 ms */
        vTaskDelay( ( TickType_t ) 10 / portTICK_PERIOD_MS );
        /* RESET = calibration done */
    } while (ADC_GetCalibrationStatus(ADC1) != RESET);

    /* queue of ADC data values received in irq handler */
    adcValueQueue = xQueueCreate(4, sizeof(int));

    while (1) {
        /* measure internal Vref=1.20V */
        ADC_RegularChannelConfig(ADC1, ADC_Channel_17, 1, ADC_SampleTime_71Cycles5);    // Vref
        ADC_ClearFlag(ADC1, ADC_FLAG_STRT);
        ADC_Cmd(ADC1, ENABLE);
        /* wait till the conversion ends */
        int ad_vref = 0;
        xQueueReceive(adcValueQueue, &ad_vref, portMAX_DELAY);

        /* measure temperature sensor voltage on channel 16 */
        ADC_RegularChannelConfig(ADC1, ADC_Channel_16, 1, ADC_SampleTime_71Cycles5);
        ADC_ClearFlag(ADC1, ADC_FLAG_STRT);
        ADC_Cmd(ADC1, ENABLE);
        /* wait till the conversion ends */
        int ad_vtemp = 0;
        xQueueReceive(adcValueQueue, &ad_vtemp, portMAX_DELAY);

        /* measure battery voltage on channel 8 */
        ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 1, ADC_SampleTime_71Cycles5);
        ADC_ClearFlag(ADC1, ADC_FLAG_STRT);
        ADC_Cmd(ADC1, ENABLE);
        /* wait till the conversion ends */
        int ad_vbat = 0;
        xQueueReceive(adcValueQueue, &ad_vbat, portMAX_DELAY);

        /* compensate for resistor divider by 2 */
        ad_vbat *= 2;

        /* digital value of ad_vref corresponds to 1.20 Volts */
        /* ad_vref/1.20 = ad_vbat/vbat */
        /* vbat = ad_vbat/ad_vref * 1.20 */
        vbat_measured = ad_vbat * 1200 / ad_vref;

        /* minimum voltage=3.50V = 0%, maximum voltage=4.20V = 100% */
        vbat_percent = (vbat_measured - VBAT_0_PERCENT) * 100 / (VBAT_100_PERCENT - VBAT_0_PERCENT);

        /* voltage on the temperature sensor in milivolts */
        int vtemp_measured = ad_vtemp * 1200 / ad_vref;

        /* 25°C = 1.43V, slope 4.3mV/C */
        temp_celsius = (VTEMP_25C-vtemp_measured)*10/43 + 25;

        /* battery state */
        char new_batt_state = BATTERY_UNKNOWN;
        if (GPIO_ReadInputDataBit(USBPOW_Port, USBPOW_Pin) == Bit_SET) {
            /* USB power is ON */
            if (GPIO_ReadInputDataBit(CHARGESTAT_Port, CHARGESTAT_Pin) == Bit_SET) {
                /* No charging -> battery full */
                new_batt_state = BATTERY_FULL;
            } else {
                /* Charging */
                new_batt_state = BATTERY_CHARGING;
            }
        } else {
            /* no USB poer */
            new_batt_state = BATTERY_DISCHARGING;
        }

        if (batt_state != new_batt_state) {
            batt_state = new_batt_state;

            struct guievent gevnt;
            gevnt.evnt = GUI_E_BATT;
            gevnt.buf = NULL;
            gevnt.kpar = batt_state;
            xQueueSend(toGuiQueue, &gevnt, 0);
        }

        /* wait 4 seconds */
        vTaskDelay( ( TickType_t ) 4000 / portTICK_PERIOD_MS );
    }
}
