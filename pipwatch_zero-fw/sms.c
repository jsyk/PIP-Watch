#include "sms.h"

#include "FreeRTOS.h"
#include "task.h"

#include <string.h>


/* allocate SMS text */
struct smstext *sms_alloc(void)
{
    struct smstext *sms = pvPortMalloc(sizeof(struct smstext));
    if (sms != NULL) {
        memset(sms, 0, sizeof(struct smstext));
    }
    return sms;
}

/* release SMS text */
void sms_free(struct smstext *sms)
{
    if (sms != NULL) {
        vPortFree(sms->sender_name);
        vPortFree(sms->sender_phone);
        vPortFree(sms->text);
        vPortFree(sms);
    }
}

struct GuiSMS *gui_sms_alloc(void)
{
    struct GuiSMS *sms = pvPortMalloc(sizeof(struct GuiSMS));
    if (sms != NULL) {
        memset(sms, 0, sizeof(struct GuiSMS));
        sms->win.draw_window_fn = gui_sms_draw_cb;
        sms->txtbox = gui_textbox_alloc(2);
        sms->txtbox->markup = 1;
        sms->txtbox->wraplines = 1;
        sms->txtbox->escexpand = 1;
    }
    return sms;
}

int gui_sms_draw_cb(u8g_t *u8g, struct GuiWindow *win, struct GuiPoint abspos)
{
    struct GuiSMS *sms = (struct GuiSMS *)win;

    vPortFree(sms->txtbox->txt.textlines[0]);
    sms->txtbox->txt.textlines[1] = NULL;

    if (sms->num_smspool > 0) {
        if (sms->selected_sms >= sms->num_smspool) {
            sms->selected_sms = sms->num_smspool - 1;
        }
        sms->txtbox->txt.textlines[1] = sms->sms_pool[sms->selected_sms]->text;
    } else {
        char *buf = newstrn(NULL, 64);
        char *ps = buf;
        ps += itostr(ps, 64-(ps-buf), sms->selected_sms);
        *ps++ = '/';
        ps += itostr(ps, 64-(ps-buf), sms->num_smspool);
        *ps++ = ':';
        *ps++ = '\0';

        sms->txtbox->txt.textlines[0] = buf;
    }

    sms->txtbox->win.draw_window_fn(u8g, &sms->txtbox->win, abspos);

    return 0;
}

void gui_sms_delete(struct GuiSMS *sms, int idx, int num)
{
    for (int i = idx; i < sms->num_smspool; ++i) {
        if (i < idx+num) {
            sms_free(sms->sms_pool[i]);
            sms->sms_pool[i] = NULL;
        } else {
            sms->sms_pool[i-num] = sms->sms_pool[i];
            sms->sms_pool[i] = NULL;
        }
    }
    sms->num_smspool -= num;
}

void gui_sms_add(struct GuiSMS *sms, struct smstext *smstxt)
{
    if (sms->num_smspool > MAX_SMS_POOL-1) {
        gui_sms_delete(sms, 0, 1);
    }
    sms->sms_pool[sms->num_smspool] = smstxt;
    ++sms->num_smspool;
}
