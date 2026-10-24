#ifndef __TCP_CLIENT_H__
#define __TCP_CLIENT_H__

#ifdef __cplusplus
extern "C"
{
#endif

    extern void *tcp_recv_thread(void *parg);

    extern void *tcp_connect_thread(void *parg);
    extern void *tcp_connect_special_thread(void *parg);

    extern unsigned int push_data_send_to_buffer(unsigned char channel, void *pdat, unsigned int datlen);

    extern void reg_tcp_connect_thread_callback(void (*func)(void));
    extern void reg_net_data_arrive_callback(void (*func)(unsigned char channel, void *pdat, unsigned int datlen));

    extern int save_log(void *ptip, void *pdat, unsigned int datlen);

    extern int save_cache_log(void *ptip, void *pdat, unsigned int datlen);

#ifdef __cplusplus
}
#endif

#endif
