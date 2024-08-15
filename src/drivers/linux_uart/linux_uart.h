#ifndef __LINUX_UART_H__
#define __LINUX_UART_H__

#ifdef __cplusplus
extern "C" {
#endif


extern int open_uart(unsigned char channel,void *dev_path,unsigned int bps,unsigned char databits,unsigned char stopbits,char iParity,unsigned int (*uart_data_arrive)(void *,unsigned int));


extern int uart_send_bytes(unsigned char channel,void *buffer,unsigned int length);

extern void reg_uart_recv_thread_callback(void (*pfunc)(void));

#ifdef __cplusplus
}
#endif

#endif

