#ifndef MAIN_H
#define MAIN_H

#include "FreeRTOS.h"
#include "queue.h"


extern QueueHandle_t toDisplayStrQueue;


void assert_failed( char *pucFile, unsigned long ulLine );

void printstr(char *buf);


#endif
