#include "sms.h"

#include "FreeRTOS.h"
#include "task.h"

#include <string.h>



struct smstext *sms_alloc(void)
{
	struct smstext *sms = pvPortMalloc(sizeof(struct smstext));
	if (sms != NULL) {
		memset(sms, 0, sizeof(struct smstext));
	}
	return sms;
}


void sms_free(struct smstext *sms)
{
	if (sms != NULL) {
        vPortFree(sms->sender_name);
        vPortFree(sms->sender_phone);
        vPortFree(sms->text);
        vPortFree(sms);
	}
}

