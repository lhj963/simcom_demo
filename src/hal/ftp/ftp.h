#ifndef __FTP_H__
#define __FTP_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*connect_tcp_func)(char *server, unsigned int port);
typedef int (*write_tcp_func)(int s,void *pbuf,unsigned int buflen);
typedef int (*read_tcp_func)(int s,void *pbuf,unsigned int buflen);

typedef int (*bind_port_and_listen_func)(unsigned int port);
typedef int (*accept_func)(int s);


extern void ftp_init(int (*connect_tcp)(char *server, unsigned int port),
                int (*write_tcp)(int s,void *pbuf,unsigned int buflen),
                int (*read_tcp)(int s,void *pbuf,unsigned int buflen),
                int (*bind_port_and_listen_tcp)(unsigned int port),
                int (*accept_func)(int s)
            );

extern void ftp_download(void *ftp_url,void *user,void *psw);

extern void ftp_upload(void *sip,unsigned int sport,void *user,void *psw,void *full_file_path,void (*cb_upload_finish)(unsigned int ret));

#ifdef __cplusplus
}
#endif
#endif



