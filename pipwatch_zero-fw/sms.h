#ifndef SMS_H
#define SMS_H

#include "rtclock.h"

struct smstext {
    rtclock_t tm_recv;          /* received time */
    char *sender_name;
    char *sender_phone;
    char *text;
};

#endif
