#ifndef __SMS_H__
#define __SMS_H__

#ifdef __cplusplus
extern "C" {
#endif

extern void analyze_sms(void *response,void (*sms_arrive_cb_func)(unsigned char *ptime,void *phone_num,void *psms,unsigned int sms_len));

extern void sms_proc(void);

extern int send_sms(void *phone_num,void *sms,unsigned int sms_len);


#ifdef __cplusplus
}
#endif

#endif

