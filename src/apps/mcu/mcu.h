#ifndef __MCU_H__
#define __MCU_H__

#ifdef __cplusplus
extern "C" {
#endif

extern void get_time(unsigned char t[6]);

extern void send_msg_to_mcu(unsigned int msgid,void *content,unsigned int len);

extern void mpu_ack_mcu(unsigned int msgid,void *content,unsigned int len);

extern unsigned int mcu_data_arrive(void *pdat,unsigned int datlen);

extern void analyze_mcu_data(unsigned int msgid,void *content,unsigned int contentlen);

extern void mcu_error_data(void *pdat,unsigned int len);

extern void send_status_to_mcu(void);

extern void send_param_to_mcu(void);

extern void send_heartbeat_to_mcu(void);

extern void send_offline_msg_to_mcu(unsigned char channel);

extern void send_online_msg_to_mcu(unsigned char channel);

extern void send_easy_change_param_to_mcu(void);

extern void net_data_arrive(unsigned char channel,void *pdat,unsigned int datlen);


extern void send_start_mcu_update_msg(void);

extern void update_box_firmware(void);

extern int create_blind_file(unsigned char channel,void *pdat,unsigned int datlen);


#ifdef __cplusplus
}
#endif

#endif

