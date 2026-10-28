#include "tcp_client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>

unsigned long s_tcp_send_total_bytes = 0;


int myaccept(int s)
{
    int new_fd = 0;
    int  sin_size = sizeof(struct sockaddr_in);
    struct sockaddr_in their_addr;

    new_fd = accept(s,(struct sockaddr *)&their_addr,&sin_size);
    return new_fd;
}

int bind_port_and_listen(unsigned int port)
{
    int sockfd;
    //int new_fd;
    struct sockaddr_in my_addr;
    //struct sockaddr_in their_addr;
    //int sin_size;

    if((sockfd = socket(AF_INET,SOCK_STREAM,0))==-1)
    {
        printf("create socket error");
        perror("socket");
        return 0;
    }

    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(port);
    my_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(my_addr.sin_zero),8);

    if(bind(sockfd,(struct sockaddr *)&my_addr,sizeof(struct sockaddr))==-1)
    {
        perror("bind socket error");
        return 0;
    }

    if(listen(sockfd,10)==-1)
    {
        perror("listen");
        return 0;
    }

    return sockfd;
}

int ls_poc_create_tcp_connect(char *server, unsigned int port)
{
    int cfd = 0;  
    struct sockaddr_in s_add = {0}; 
    unsigned short portnum = port;
    struct hostent *phost = 0;
    char szip[32 + 1] = {0};

    if ((server == 0) || (port == 0))
    {
        return -1;
    }

    //DBG_Print(LEVEL3,"ls_poc_create_tcp_connect(),server =%s , port = %d!\r\n",server,port);
    
    /*create TCP socket*/     
    cfd = socket(AF_INET, SOCK_STREAM, 0);
    if(-1 == cfd)
    {
        DBG_Print(LEVEL3,"socket fail ! \r\n");  
        return -1;  
    }

    bzero(&s_add,sizeof(struct sockaddr_in));

    if (is_ip_string(server))
    {
        strncpy(szip,server,sizeof(szip) - 1);
    }
    else
    {
        phost = gethostbyname((char *)server);
        if (phost)
        {
            sprintf(szip,"%u.%u.%u.%u",(phost->h_addr_list[0][0] & 0xFF),
                                        (phost->h_addr_list[0][1] & 0xFF),
                                        (phost->h_addr_list[0][2] & 0xFF),
                                        (phost->h_addr_list[0][3] & 0xFF));
            DBG_Print(LEVEL4,"dns(%s) ip:%s\r\n",server,szip);
        }
    }

    s_add.sin_family = AF_INET;
    s_add.sin_addr.s_addr = inet_addr(szip);
    s_add.sin_port = htons(portnum); 

    //printf("s_addr = %#x ,port : %#x\r\n",s_add.sin_addr.s_addr,s_add.sin_port);
    if(connect(cfd,(struct sockaddr *)(&s_add), sizeof(struct sockaddr)) < 0)  
    {
        close(cfd);
        DBG_Print(LEVEL3,"connect fail !\r\n");  
        return -1;
    }

    return cfd;
}


#define EVERY_DATA_SEND_BUF_LEN  (1024*10)

unsigned char data_send_buffer[9][EVERY_DATA_SEND_BUF_LEN] = {0};
unsigned int data_send_len[9] = {0};

unsigned int get_data_send_len(unsigned char channel)
{
    if (channel >= 9)
    {
        return 0;
    }
    return data_send_len[channel];
}

unsigned int push_data_send_to_buffer(unsigned char channel,void *pdat,unsigned int datlen)
{
    unsigned int templen = 0;

    if (pdat && datlen && (channel < 9))
    {
        unsigned int old_len = data_send_len[channel];

        templen = datlen;
        if (templen > (EVERY_DATA_SEND_BUF_LEN - old_len))
        {
            templen = (EVERY_DATA_SEND_BUF_LEN - old_len);
        }

        if (templen) memcpy((unsigned char *)(data_send_buffer[channel]) + old_len,pdat,templen);
        data_send_len[channel] += templen;
    }
    return templen;
}

void data_send_len_clr(unsigned char channel)
{
    if (channel >= 9)
    {
        return ;
    }
    data_send_len[channel] = 0;
}

unsigned char *get_data_send_ptr(unsigned char channel)
{
    if (channel >= 9)
    {
        channel = 0;
    }
    return data_send_buffer[channel];
}

int save_cache_log(void *ptip,void *pdat,unsigned int datlen)
{
    unsigned char *pt = get_rtc_time();

    if ((get_main_power_off_flag() == 0) && is_time_format_ok(pt) && is_emmc_ok() && sysparam_get_collect_file() && (ptip || (pdat && datlen)))
    {
        char tip[160] = {0};
        char fn[96] = {0};

        sprintf(fn,"/media/card/clog/clog%02d%02d%02d.txt",pt[0],pt[1],pt[2]);
        snprintf(tip,sizeof(tip) - 1,"[%02d-%02d-%02d %02d:%02d:%02d]%s",pt[0],pt[1],pt[2],pt[3],pt[4],pt[5],ptip ? ((char *)ptip) : " ");
        DBG_PrintBufEx(LEVEL0,fn,tip,pdat,datlen);
    }
    return 0;
}

int save_log(void *ptip,void *pdat,unsigned int datlen)
{
    unsigned char *pt = get_rtc_time();

    if ((get_main_power_off_flag() == 0) && is_time_format_ok(pt) && is_emmc_ok() && sysparam_get_collect_file() && (ptip || (pdat && datlen)))
    {
        char tip[160] = {0};
        char fn[96] = {0};

        sprintf(fn,"/media/card/vlog/vlog%02d%02d%02d.txt",pt[0],pt[1],pt[2]);
        snprintf(tip,sizeof(tip) - 1,"[%02d-%02d-%02d %02d:%02d:%02d]%s",pt[0],pt[1],pt[2],pt[3],pt[4],pt[5],ptip ? ((char *)ptip) : " ");
        DBG_PrintBufEx(LEVEL0,fn,tip,pdat,datlen);
    }
    return 0;
}

void record_net_log(unsigned char channel,void *ptip,void *pdat,unsigned int datlen)
{
    char tip[96] = {0};

    snprintf(tip,sizeof(tip) - 1,"[ch%d]%s(%uBytes)",channel,ptip ? ((char *)ptip) : " ",datlen);

    save_log(tip,pdat,datlen);
}

/*****blind proc**********/
typedef struct tagUserArgBlind
{
    int fd;
    unsigned char channel;
}UserArgBlind;

void blind_file_send(void *full_path_filename,void *p_user_arg)
{
    unsigned char *pbuf = 0;
    unsigned int rlen = 0;
    unsigned char channel = 0;
    int fd = 0;


    if ((p_user_arg == 0) || (full_path_filename == 0))
    {
        return ;
    }

    fd = ((UserArgBlind *)p_user_arg)->fd;
    channel = ((UserArgBlind *)p_user_arg)->channel;

    if (get_server_connect_status(channel) == 0)
    {
        return ;
    }

    pbuf = (unsigned char *)malloc(2048);
    if (pbuf)
    {
        rlen = myreadfile(full_path_filename,pbuf + 64,2048 - 64);
        if (rlen)
        {
            if ((channel == 1) && (sysparam_get_func_select() == FuncSelect_JMRH_SM4))
            {
                record_net_log(channel,"明文",pbuf + 64,rlen);

                rlen = jmrh_pack(pbuf + 64,rlen,pbuf,2048);

                DBG_PrintBuf(LEVEL3,"jmrh:\r\n",pbuf,rlen);
            }
            else
            {
                if ((channel == 2) || (channel == 5))
                {
                    rlen = hj1239_data_pack(pbuf + 64,rlen);
                }
                if (rlen) memcpy(pbuf,pbuf + 64,rlen);
            }

            if(rlen && (write(fd,pbuf,rlen) <= 0))
            {
                DBG_Print(LEVEL2,"[blind_proc]write tcp socket fail!\r\n");
                set_server_connect_status(channel,0);
            }
            else
            {                
                DBG_Print(LEVEL3,"%s proc finish\r\n",(char *)full_path_filename);
                remove((const char *)full_path_filename);

                record_net_log(channel,"发送",pbuf,rlen);

                s_tcp_send_total_bytes += rlen;
            }
        }
        else
        {
            remove((const char *)full_path_filename);
        }
        free(pbuf);
        usleep(100000);
    }
}

void stat_blind_data_cnt(unsigned char channel)
{
    char floder_path[256] = {0};
    
    sprintf(floder_path,"/media/card/blind/ch%u",channel);
    set_blind_data_cnt(channel,list_folder_no_sort(floder_path,0,0,0));
}

int blind_proc(unsigned char channel,int fd)
{
    char floder_path[256] = {0};
    UserArgBlind stUserArgBlind = {0};

    if ((channel >= 9) || (fd <= 0))
    {
        return 0;
    }

    if ((channel == 1) && (get_handshake_flag() == 0) && (sysparam_get_func_select() == FuncSelect_JMRH_SM4))
    {
        return 0;
    }

    sprintf(floder_path,"/media/card/blind/ch%u",channel);

    stUserArgBlind.channel = channel;
    stUserArgBlind.fd = fd;
    
    list_folder_no_sort(floder_path,blind_file_send,&stUserArgBlind,6);

    return 0;
}

typedef void (*net_data_arrive_cb)(unsigned char channel,void *,unsigned int);
typedef void (*tcp_connect_thread_cb)(void);

tcp_connect_thread_cb s_tcp_connect_thread_cb = 0;
net_data_arrive_cb s_net_data_arrive_cb = 0;

void reg_tcp_connect_thread_callback(void (*func)(void))
{
    s_tcp_connect_thread_cb = func;
}

void reg_net_data_arrive_callback(void (*func)(unsigned char channel,void *pdat,unsigned int datlen))
{
    s_net_data_arrive_cb = func;
}

#define TCP_THREAD_RECV_BUF_LEN     2048


int s_socketfd[9] = {0};
unsigned int connect_time_threshold[9] = {0};

static pthread_mutex_t s_sock_mu[9] = {
	PTHREAD_MUTEX_INITIALIZER,
	PTHREAD_MUTEX_INITIALIZER,
	PTHREAD_MUTEX_INITIALIZER,
	PTHREAD_MUTEX_INITIALIZER,
	PTHREAD_MUTEX_INITIALIZER,
	PTHREAD_MUTEX_INITIALIZER,
	PTHREAD_MUTEX_INITIALIZER,
	PTHREAD_MUTEX_INITIALIZER,
	PTHREAD_MUTEX_INITIALIZER};

static int sock_get(unsigned char channel)
{
	int fd;

	if (channel >= 9)
	{
		return -1;
	}
	pthread_mutex_lock(&s_sock_mu[channel]);
	fd = s_socketfd[channel];
	pthread_mutex_unlock(&s_sock_mu[channel]);
	return fd;
}

static int sock_snapshot(unsigned char channel, int *orig)
{
	int fd = -1;

	if (orig)
	{
		*orig = -1;
	}
	if (channel >= 9)
	{
		return -1;
	}
	pthread_mutex_lock(&s_sock_mu[channel]);
	if (s_socketfd[channel] > 0)
	{
		if (orig)
		{
			*orig = s_socketfd[channel];
		}
		fd = dup(s_socketfd[channel]);
	}
	pthread_mutex_unlock(&s_sock_mu[channel]);
	return fd;
}

static void sock_close_fd(int fd)
{
	if (fd > 0)
	{
		shutdown(fd, SHUT_RDWR);
		close(fd);
	}
}

static void sock_close(unsigned char channel)
{
	int fd = -1;

	if (channel >= 9)
	{
		return;
	}
	pthread_mutex_lock(&s_sock_mu[channel]);
	fd = s_socketfd[channel];
	s_socketfd[channel] = -1;
	pthread_mutex_unlock(&s_sock_mu[channel]);
	sock_close_fd(fd);
}

static void sock_close_if_same(unsigned char channel, int watched)
{
	int fd = -1;

	if ((channel >= 9) || (watched <= 0))
	{
		return;
	}
	pthread_mutex_lock(&s_sock_mu[channel]);
	if (s_socketfd[channel] == watched)
	{
		fd = s_socketfd[channel];
		s_socketfd[channel] = -1;
	}
	pthread_mutex_unlock(&s_sock_mu[channel]);
	sock_close_fd(fd);
}

static void sock_store(unsigned char channel, int newfd)
{
	int old = -1;
	int stored = (newfd > 0) ? newfd : -1;

	if (channel >= 9)
	{
		if (newfd > 0)
		{
			close(newfd);
		}
		return;
	}
	pthread_mutex_lock(&s_sock_mu[channel]);
	old = s_socketfd[channel];
	s_socketfd[channel] = stored;
	pthread_mutex_unlock(&s_sock_mu[channel]);
	if ((old > 0) && (old != stored))
	{
		sock_close_fd(old);
	}
}

void *tcp_connect_thread(void *parg)
{
    unsigned char channel = 0;
    unsigned long last_connect_tcp_time[9] = {0};


    ftp_init(ls_poc_create_tcp_connect,(int (*)(int ,void *,unsigned int))write,read,bind_port_and_listen,myaccept);

    while (1)
    {
        if (get_ppp_status(0))
        {
            for (channel = 0; channel < 7; channel++)
            {
                if (get_server_change_flag(channel))
                {
                    sock_close(channel);
                    set_server_change_flag(channel,0);
                    set_server_connect_status(channel,0);
                    send_offline_msg_to_mcu(channel);
                }

                if (sysparam_get_serverport(channel) && (sock_get(channel) <= 0)
                    && (((channel == 7) && get_wifi_client_status()) || (channel != 7))
                    )
                {
                    int fd;

                    set_server_connect_status(channel,0);
                    if ((get_run_time() - last_connect_tcp_time[channel]) >= connect_time_threshold[channel])
                    {
                        sleep(2);
                        last_connect_tcp_time[channel] = get_run_time();
                        fd = ls_poc_create_tcp_connect(sysparam_get_serveraddr(channel),sysparam_get_serverport(channel));
                        sock_store(channel, fd);
                        if (sock_get(channel) > 0)
                        {
                            set_server_connect_status(channel,1);
                            send_online_msg_to_mcu(channel);
                        }
                        else
                        {
                            set_server_connect_status(channel,0);
                            send_offline_msg_to_mcu(channel);
                        }
                    }
                }
            }
        }

        if (s_tcp_connect_thread_cb)
        {
            s_tcp_connect_thread_cb();
        }
        sleep(2);
    }
    return 0;
}

void *tcp_connect_special_thread(void *parg)
{
    unsigned char channel = 0;
    unsigned long last_connect_tcp_time[9] = {0};

    while (1)
    {
        if (get_ppp_status(0))
        {
            sleep(2);
            for (channel = 7; channel < 9; channel++)
            {
                if (get_server_change_flag(channel))
                {
                    sock_close(channel);
                    set_server_change_flag(channel,0);
                    set_server_connect_status(channel,0);
                    send_offline_msg_to_mcu(channel);
                }

                if (sysparam_get_serverport(channel) && (sock_get(channel) <= 0)
                    && (((channel == 7) && get_wifi_client_status()) || (channel != 7))
                    )
                {
                    int fd;

                    set_server_connect_status(channel,0);
                    if ((get_run_time() - last_connect_tcp_time[channel]) >= connect_time_threshold[channel])
                    {
                        sleep(2);
                        last_connect_tcp_time[channel] = get_run_time();
                        fd = ls_poc_create_tcp_connect(sysparam_get_serveraddr(channel),sysparam_get_serverport(channel));
                        sock_store(channel, fd);
                        if (sock_get(channel) > 0)
                        {
                            set_server_connect_status(channel,1);
                            send_online_msg_to_mcu(channel);
                        }
                        else
                        {
                            set_server_connect_status(channel,0);
                            send_offline_msg_to_mcu(channel);
                        }
                    }
                }
            }
        }        
    }
    return 0;
}

void *tcp_recv_thread(void *parg)
{
    static unsigned char s_buff[TCP_THREAD_RECV_BUF_LEN + 64] = {0};
    struct timeval tv = {0};
    fd_set rset;
    unsigned char channel = 0;
    int status = 0;
    int maxfd = 0;
    int ret = 0;
    unsigned char kick_cnt[9] = {0};
    unsigned char scnt = 0;
    unsigned char s_last_ppp_status = 0;
    unsigned int s_online_time[9] = {0};

    while (1)
    {
        int snap[9];
        int orig[9];

        maxfd = 0;
        FD_ZERO(&rset);
        for (channel = 0; channel < 9; channel++)
        {
            snap[channel] = -1;
            orig[channel] = -1;
            if (get_server_connect_status(channel))
            {
                snap[channel] = sock_snapshot(channel, &orig[channel]);
            }
            if (snap[channel] > 0)
            {
                if (maxfd < snap[channel])
                {
                    maxfd = snap[channel];
                }
                FD_SET(snap[channel], &rset);
            }
        }

        //recv data
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        if (maxfd)
        {
            status = select(maxfd + 1, &rset, NULL, NULL, &tv);
        }
        else
        {
            sleep(1);
            status = 0;
        }

        if (status > 0)
        {
            for (channel = 0; channel < 9; channel++)
            {
                if ((snap[channel] > 0) && FD_ISSET(snap[channel], &rset))
                {
                    //data arrive
                    memset(s_buff,0,sizeof(s_buff));
                    ret = read(snap[channel], s_buff, TCP_THREAD_RECV_BUF_LEN);
                    if (ret > 0)
                    {
                        record_net_log(channel,"recv",s_buff,ret);
                        if (s_net_data_arrive_cb)
                        {
                            s_net_data_arrive_cb(channel,s_buff,ret);
                        }
                    }
                    else
                    {
                        if (ret == 0)
                        {
                            kick_cnt[channel]++;
                            if (kick_cnt[channel] >= 7)
                            {
                                kick_cnt[channel] = 0;
                                connect_time_threshold[channel] = 30;
                            }
                        }
                        else
                        {
                            kick_cnt[channel] = 0;
                            connect_time_threshold[channel] = 0;
                        }
                        sock_close_if_same(channel, orig[channel]);
                        set_server_connect_status(channel,0);
                        send_offline_msg_to_mcu(channel);
                    }
                }
            }
        }
        else if (status == 0)               //time out
        {

        }
        else                                //error
        {
            static char s_select_flag = 0;
            if (s_select_flag == 0)
            {
                DBG_PrintEx(LEVEL1,"/data/error.txt","tcp select error\r\n",strlen("tcp select error\r\n"));
                s_select_flag = 1;
            }
        }
        for (channel = 0; channel < 9; channel++)
        {
            if (snap[channel] > 0)
            {
                close(snap[channel]);
            }
        }

        scnt++;

        if (get_ppp_status(0) != s_last_ppp_status)
        {
            s_last_ppp_status = get_ppp_status(0);
            if (get_ppp_status(0) == 0)
            {
                for (channel = 0; channel < 9; channel++)
                {
                    if (sock_get(channel) > 0)
                    {
                        set_server_connect_status(channel,0);
                        sock_close(channel);
                        send_offline_msg_to_mcu(channel);
                    }
                }
            }
        }

        //send data
        for (channel = 0; channel < 9; channel++)
        {
            if (get_server_connect_status(channel))
            {
                s_online_time[channel]++;
            }
            else
            {
                s_online_time[channel] = 0;
            }

            {
                int io_fd = -1;
                int orig_fd = -1;

                io_fd = sock_snapshot(channel, &orig_fd);
                if (get_server_connect_status(channel)
                    && (io_fd > 0)
                    && (get_ppp_status(0) > 0)
                    && (get_dbm() > -90)
                    )
                {
                    if (get_data_send_len(channel))
                    {
                        if(write(io_fd,get_data_send_ptr(channel),get_data_send_len(channel)) < 0)
                        {
                            DBG_Print(LEVEL2,"[tcp_thread]write tcp socket fail!\r\n");
                            set_server_connect_status(channel,0);
                            sock_close_if_same(channel, orig_fd);
                            send_offline_msg_to_mcu(channel);

                            if (channel != 4) create_blind_file(channel,get_data_send_ptr(channel),get_data_send_len(channel));
                        }
                        else
                        {
                            record_net_log(channel,"发送",get_data_send_ptr(channel),get_data_send_len(channel));
                            s_tcp_send_total_bytes += get_data_send_len(channel);
                        }
                        data_send_len_clr(channel);
                    }
                    else
                    {
                        if ((channel == 4) || (channel == 6))   //dst link and sddf link
                        {
                            if (s_online_time[channel] >= 4)
                            {
                                blind_proc(channel,io_fd);
                                if (get_server_connect_status(channel) == 0)
                                {
                                    sock_close_if_same(channel, orig_fd);
                                    send_offline_msg_to_mcu(channel);
                                }
                            }
                        }
                        else
                        {
                            blind_proc(channel,io_fd);
                            if (get_server_connect_status(channel) == 0)
                            {
                                sock_close_if_same(channel, orig_fd);
                                send_offline_msg_to_mcu(channel);
                            }
                        }
                    }
                }
                if (io_fd > 0)
                {
                    close(io_fd);
                }
            }

            if (0)
            {
                static unsigned long s_last_tcp_send_total_bytes = 0;

                if (s_tcp_send_total_bytes != s_last_tcp_send_total_bytes)
                {
                    s_last_tcp_send_total_bytes = s_tcp_send_total_bytes;
                    printf("send total bytes:%lu\r\n",s_tcp_send_total_bytes);
                }
            }

            if (scnt >= 6)
            {
                stat_blind_data_cnt(channel);
            }
        }

        if (scnt >= 6)
        {
            scnt = 0;
        }
    }
    return 0;
}
