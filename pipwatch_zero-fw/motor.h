#ifndef MOTOR_H
#define MOTOR_H

#include "FreeRTOS.h"

#define MOTOR_Port          GPIOB
#define MOTOR_Pin           GPIO_Pin_13


#define MOTOR_DUR_SHORT         (( TickType_t ) 70 / portTICK_PERIOD_MS)
#define MOTOR_DUR_MEDIUM         (( TickType_t ) 150 / portTICK_PERIOD_MS)
#define MOTOR_DUR_LONG         (( TickType_t ) 400 / portTICK_PERIOD_MS)

// #define MOTOR_ACT_OFF           0
// #define MOTOR_ACT_FULL          100

void MotorTask(void *pvParameters);

void Motor_Pulse(int dur);

#endif
