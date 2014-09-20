#ifndef BTM_H
#define BTM_H

/* COM port and baud rate used by the echo task. */
#define comBTM                                 ( 1 )
#define mainBAUD_RATE                       ( 9600 )

/* PC4 = Reset */
#define BTM_Reset_Port              GPIOC
#define BTM_Reset_Pin               GPIO_Pin_4


#define BTMST_NonConfigured         0
#define BTMST_Configured            1
#define BTMST_Listening             2
#define BTMST_Connected             3


void BluetoothModemTask( void *pvParameters );

#endif
