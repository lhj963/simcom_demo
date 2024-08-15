#ifndef __TBOX_MDM_H__
#define __TBOX_MDM_H__

#ifdef __cplusplus
extern "C" {
#endif

extern int update_sim_info_proc(void);
extern void auto_ppp_proc(void);
extern void modem_at_port_init(void (*p_read_modem_at_cb)(void * pdat));
extern void send_at_cmd(void *pcmd);


#ifdef __cplusplus
}
#endif

#endif

