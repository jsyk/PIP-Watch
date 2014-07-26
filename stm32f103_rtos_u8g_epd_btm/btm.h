#ifndef BTM_H
#define BTM_H

/* COM port and baud rate used by the echo task. */
#define comBTM                                 ( 1 )
#define mainBAUD_RATE                       ( 57600 )


void BluetoothModemTask( void *pvParameters );

#endif
