#include "ftp.h"


typedef struct tagFTPStruction
{
    connect_tcp_func connect;
    write_tcp_func write;
    read_tcp_func read;
    bind_port_and_listen_func bind_and_listen;
    accept_func accept;

    char usr[64];
    char psw[64];
    char file_path[256];
    char file_name[64];
    unsigned char buf[1024];

    char ftp_server_ip[64];
    unsigned short cmd_port;
    unsigned short data_port;
    unsigned char port_or_pasv_mode;    //1:port mode   0:pasv mode


    unsigned char upload_or_download_ok_flag;

    void (*p_upload_finish)(unsigned int ret);
}FTPStruction;

FTPStruction sFTPStruction = {0};

void *ftp_data_thread(void *parg)
{
    int data_server_socket = 0;
    int data_client_socket = 0;
    int rlen = 0;

    unsigned char upload_flag = (unsigned int)parg;

    if (sFTPStruction.port_or_pasv_mode == 1)           //listen
    {
        printf("listen ftp data port:%u\r\n",sFTPStruction.data_port);

        if (sFTPStruction.bind_and_listen)
        {
            data_server_socket = sFTPStruction.bind_and_listen(sFTPStruction.data_port);
            if (data_server_socket > 0) printf("bind port ok\r\n");
        }

        if (sFTPStruction.accept && (data_server_socket > 0))
        {
            data_client_socket = sFTPStruction.accept(data_server_socket);
            if (data_client_socket > 0)
            {
                printf("accept client connect\r\n");
            }
        }
    }
    else
    {
        data_client_socket = sFTPStruction.connect(sFTPStruction.ftp_server_ip,sFTPStruction.data_port);
    }

    if (data_client_socket <= 0)
    {
        printf("data thread connect false\n");
        
        if (upload_flag && sFTPStruction.p_upload_finish) sFTPStruction.p_upload_finish(1);
        sFTPStruction.upload_or_download_ok_flag = 1;

        return (void *)1;
    }

    if (upload_flag)
    {
        unsigned int offset = 0;
        unsigned int cnt = 0;

        printf("start upload data\r\n");
        while (1)
        {
            rlen = myreadfile1(sFTPStruction.file_path,offset,sFTPStruction.buf,sizeof(sFTPStruction.buf));
            if (sFTPStruction.write && rlen)
            {
                sFTPStruction.write(data_client_socket,sFTPStruction.buf,rlen);
            }
            offset += rlen;

            //printf("upload %uBytes\r\n",offset);
            cnt++;
            if (cnt >= 20)
            {
                cnt = 0;
                sleep(1);
            }

            if ((rlen == 0) || (rlen < 1024))
            {
                break;
            }
        }
        if (data_client_socket > 0) close(data_client_socket);
        sFTPStruction.upload_or_download_ok_flag = 1;

        if (sFTPStruction.p_upload_finish) sFTPStruction.p_upload_finish(0);
    }
    else
    {
        printf("start download data\r\n");
        while (1)
        {
            rlen = sFTPStruction.read(data_client_socket,sFTPStruction.buf,sizeof(sFTPStruction.buf));
            if (rlen <= 0) break;
            if (rlen > 0)
            {
                printf("recv ftp data:%u(%s)\r\n",rlen,sFTPStruction.buf);
                break;
            }
        }
        sFTPStruction.upload_or_download_ok_flag = 1;
    }

    return 0;
}

unsigned int _exe_ftp_cmd(int s,void *cmd,unsigned int cmdlen,void *pResponesBuf,unsigned int responsebuflen)
{
    unsigned int rlen = 0;


    if ((s <= 0) || (cmd == 0) || (cmdlen == 0))
    {
        return 0;
    }

    memset(pResponesBuf,0,responsebuflen);

    sFTPStruction.write(s,cmd,cmdlen);
    printf("->ftp:%s",(char *)cmd);
    if (pResponesBuf && responsebuflen)
    {
        rlen = sFTPStruction.read(s,pResponesBuf,responsebuflen);
        printf("ftp->:%s\r\n",(char *)pResponesBuf);
    }

    return rlen;
}

unsigned int get_port_by_str(void *buf)
{
    char *ptr = 0;
    char *ptemp = (char *)buf;
    unsigned char cnt = 0;
    unsigned char high8 = 0;
    unsigned char low8 = 0;

    if (buf == 0)
    {
        return 0;
    }

    while (1)
    {
        ptr = strstr(ptemp,",");
        if (ptr)
        {
            cnt++;
            if (cnt >= 4)
            {
                high8 = atoi(ptr + 1);

                ptemp = ptr + 1;
                ptr = strstr(ptemp,",");
                if (ptr)
                {
                    low8 = atoi(ptr + 1);
                    break;
                }
                else
                {
                    return 0;
                }
            }

            ptemp = ptr + 1;
        }
        else
        {
            return 0;
        }
    }

    return ((high8 * 256) + low8);
}

void *ftp_upload_cmd_thread(void *parg)
{
    int s = 0;
    unsigned char buf[256] = {0};
    pthread_t data_tid;
    char cmd[256]  = {0};

    while (1)
    {
        s = sFTPStruction.connect(sFTPStruction.ftp_server_ip,sFTPStruction.cmd_port);
        if (s > 0)
        {
            break;
        }
        else
        {
            sleep(3);
        }
    }
    printf("connect ftp cmd port successfully\r\n");

    sFTPStruction.read(s,buf,sizeof(buf));
    printf("ftp->:%s\r\n",buf);


    sprintf(cmd,"USER %s\r\n",sFTPStruction.usr);
    _exe_ftp_cmd(s,cmd,strlen(cmd),buf,sizeof(buf));


    sprintf(cmd,"PASS %s\r\n",sFTPStruction.psw);
    _exe_ftp_cmd(s,cmd,strlen(cmd),buf,sizeof(buf));
    
    
    sprintf(cmd,"CWD /\r\n");
    _exe_ftp_cmd(s,cmd,strlen(cmd),buf,sizeof(buf));

    sFTPStruction.port_or_pasv_mode = 0;

    if (sFTPStruction.port_or_pasv_mode)
    {
        sFTPStruction.data_port = 40000;

        sprintf(cmd,"PORT 127,0,0,1,%u,%u\r\n",(sFTPStruction.data_port >> 8),sFTPStruction.data_port & 0xFF);
    }
    else
    {
        sprintf(cmd,"PASV\r\n");
    }
    _exe_ftp_cmd(s,cmd,strlen(cmd),buf,sizeof(buf));


    if (sFTPStruction.port_or_pasv_mode == 0)
    {
        sFTPStruction.data_port = get_port_by_str(buf);
        printf("host data port:%u\r\n",sFTPStruction.data_port);
    }

    //sprintf(cmd,"LIST\r\n");
    //_exe_ftp_cmd(s,cmd,strlen(cmd),buf,sizeof(buf));

    //_exe_ftp_cmd(s,"APPE 2.txt\r\n",strlen("APPE 2.txt\r\n"),0,0);
    sprintf(cmd,"STOR %s\r\n",sFTPStruction.file_name);
    _exe_ftp_cmd(s,cmd,strlen(cmd),buf,sizeof(buf));

    sFTPStruction.upload_or_download_ok_flag = 0;
    pthread_create(&data_tid, NULL, ftp_data_thread,(void *)1);

    //wait down or upload finish
    while (sFTPStruction.upload_or_download_ok_flag == 0)
    {
        sleep(2);
    }

    _exe_ftp_cmd(s,"QUIT\r\n",strlen("QUIT\r\n"),buf,sizeof(buf));
    close(s);
    printf("ftp download/upload finish.\r\n");
    
    return 0;
}

void *ftp_download_cmd_thread(void *parg)
{
    int s = 0;
    unsigned char buf[256] = {0};
    pthread_t data_tid;
    char cmd[128]  = {0};

    while (1)
    {
        s = sFTPStruction.connect(sFTPStruction.ftp_server_ip,sFTPStruction.cmd_port);
        if (s > 0)
        {
            break;
        }
        else
        {
            sleep(2);
        }
    }
    printf("connect ftp cmd port successfully\r\n");

    sFTPStruction.read(s,buf,256);
    printf("ftp->:%s\r\n",buf);


    sprintf(cmd,"USER %s\r\n",sFTPStruction.usr);
    _exe_ftp_cmd(s,cmd,strlen(cmd),buf,sizeof(buf));


    sprintf(cmd,"PASS %s\r\n",sFTPStruction.psw);
    _exe_ftp_cmd(s,cmd,strlen(cmd),buf,sizeof(buf));
    
    
    sprintf(cmd,"CWD /\r\n");
    _exe_ftp_cmd(s,cmd,strlen(cmd),buf,sizeof(buf));


    sFTPStruction.port_or_pasv_mode = 0;

    if (sFTPStruction.port_or_pasv_mode)
    {
        sFTPStruction.data_port = 40000;

        sprintf(cmd,"PORT 127,0,0,1,%u,%u\r\n",(sFTPStruction.data_port >> 8),sFTPStruction.data_port & 0xFF);
    }
    else
    {
        sprintf(cmd,"PASV\r\n");
    }

    _exe_ftp_cmd(s,cmd,strlen(cmd),buf,sizeof(buf));

    if (sFTPStruction.port_or_pasv_mode == 0)
    {
        sFTPStruction.data_port = get_port_by_str(buf);
        printf("host data port:%u\r\n",sFTPStruction.data_port);
    }

    //sprintf(cmd,"LIST\r\n");
    //_exe_ftp_cmd(s,cmd,strlen(cmd),0,0);

    sprintf(cmd,"RETR %s\r\n",sFTPStruction.file_name);
    _exe_ftp_cmd(s,cmd,strlen(cmd),0,0);

    pthread_create(&data_tid, NULL, ftp_data_thread,0);
    sleep(10);
    return 0;
}


void ftp_init(int (*connect_tcp)(char *server, unsigned int port),
                int (*write_tcp)(int s,void *pbuf,unsigned int buflen),
                int (*read_tcp)(int s,void *pbuf,unsigned int buflen),
                int (*bind_port_and_listen_tcp)(unsigned int port),
                int (*accept_func)(int s)
            )
{
    sFTPStruction.connect = connect_tcp;
    sFTPStruction.write = write_tcp;
    sFTPStruction.read = read_tcp;
    sFTPStruction.bind_and_listen = bind_port_and_listen_tcp;
    sFTPStruction.accept = accept_func;
}


void ftp_download(void *ftp_url,void *user,void *psw)
{
    pthread_t cmd_tid;

    memcpy(sFTPStruction.ftp_server_ip,"47.111.240.147",strlen("47.111.240.147"));
    sFTPStruction.cmd_port = 21;

    if (user) strncpy(sFTPStruction.usr,user,sizeof(sFTPStruction.usr) - 1);
    if (psw) strncpy(sFTPStruction.psw,psw,sizeof(sFTPStruction.psw) - 1);

    pthread_create(&cmd_tid, NULL, ftp_download_cmd_thread,0);
}


void ftp_upload(void *sip,unsigned int sport,void *user,void *psw,void *full_file_path,void (*cb_upload_finish)(unsigned int ret))
{
    pthread_t cmd_tid;
    char *p = 0;
    char *ptail = 0;
    unsigned int templen = 0;

    if ((sip == 0) || (sport == 0) || (strlen(sip) >= 63))
    {
        return ;
    }

    sFTPStruction.p_upload_finish = cb_upload_finish;

    memcpy(sFTPStruction.ftp_server_ip,sip,strlen((char *)sip));
    sFTPStruction.cmd_port = sport;

    if (user) strncpy(sFTPStruction.usr,user,sizeof(sFTPStruction.usr) - 1);
    if (psw) strncpy(sFTPStruction.psw,psw,sizeof(sFTPStruction.psw) - 1);
    if (full_file_path) strncpy(sFTPStruction.file_path,full_file_path,sizeof(sFTPStruction.file_path) - 1);

    p = ReverseFindSubStr(sFTPStruction.file_path,strlen(sFTPStruction.file_path),"/",1);
    if (p)
    {
        ptail = sFTPStruction.file_path + strlen(sFTPStruction.file_path) - 1;
        templen = ptail - p;

        memset(sFTPStruction.file_name,0,sizeof(sFTPStruction.file_name));
        memcpy(sFTPStruction.file_name,p + 1,(templen > (sizeof(sFTPStruction.file_name) - 1)) ? (sizeof(sFTPStruction.file_name) - 1) : templen);

        printf("filename:%u\r\n",templen);
    }
    else
    {
        strcpy(sFTPStruction.file_name,"tbox_upload.dat");
    }


    pthread_create(&cmd_tid, NULL, ftp_upload_cmd_thread,0);
}
