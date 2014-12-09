#ifndef SMS_H
#define SMS_H

#include "rtclock.h"
#include "gui_textbox.h"


/* One SMS message */
struct smstext {
	/* received time */
    rtclock_t tm_recv;
    /* sender name */
    char *sender_name;
    /* sender's phone number */
    char *sender_phone;
    /* the sms text */
    char *text;
};


struct smstext *sms_alloc(void);

void sms_free(struct smstext *sms);


#define MAX_SMS_POOL		32


/* SMS window app */
struct GuiSMS {
    /* the window base class; MUST BE THE FIRST ELEMENT */
    struct GuiWindow win;

    /* text box showing the selected SMS */
	struct GuiTextbox *txtbox;

	/* array of SMS texts received */
	struct smstext *sms_pool[MAX_SMS_POOL];
	/* number of SMS texts in the pool */
	int num_smspool;
	/* index of the selected SMS */
	int selected_sms;
};


struct GuiSMS *gui_sms_alloc(void);

int gui_sms_draw_cb(u8g_t *u8g, struct GuiWindow *win, struct GuiPoint abspos);

void gui_sms_add(struct GuiSMS *sms, struct smstext *smstxt);

#endif
