#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>

#include "ATControl.h"
#include "OTAUpgrade.h"
#include "LedControl.h"
#include "VoiceCall.h"
#include "simcom_common.h"
#include "WiFiControl.h"
#include "SMSControl.h"
#include "UIMControl.h"
#include "DataCall.h"
#include "WDSControl.h"
#include "NASControl.h"
#include "GpioControl.h"
// #include "GPSControl.h"
#include "simcom_loc.h"

#include "UbusContext.h"
#include "simcom_test_main.h"

#define APN1 "3gnet"
#define APN2 "3gwap"
pthread_mutex_t channel_mutex[9];
#include "src.c"

int main(int argc, char *argv[])
{
    int i = 0;

    init_data_buffer();
    SimcomUbusInit();
    atctrl_init();

    for (i = 0; i < 9; i++)
    {
        pthread_mutex_init(&(channel_mutex[i]), NULL);
    }

    for (i = 0; i < MAX_CHANNEL_CNT; i++)
    {
        pthread_mutex_init(&(s_astUartStruction[i].lock), NULL);
    }

    start_tbox();

    while (1)
    {
        sleep(5);
    }
    for (i = 0; i < MAX_CHANNEL_CNT; i++)
    {
        pthread_mutex_destroy(&(s_astUartStruction[i].lock));
    }

    for (i = 0; i < 9; i++)
    {
        pthread_mutex_destroy(&(channel_mutex[i]));
    }
    atctrl_deinit();
    SimcomUbusDeinit();
    if (p_data_send_buffer != NULL)
    {
        free(p_data_send_buffer);
    }
}

void watch_time_daemon_process(void)
{
    FILE *fp;
    unsigned int rlen = 0;
    unsigned char *retcmd = 0;
    char cmd[384 + 1] = {0};
    char des_process_exsit_flag = 0;

    retcmd = (unsigned char *)malloc(1024 + 16);
    if (retcmd)
    {
        memcpy(cmd, "ps -ef | grep time_daemon", strlen("ps -ef | grep time_daemon"));
        fp = popen(cmd, "r");
        if (fp)
        {
            rlen = fread(retcmd, 1, 1024, fp);
            pclose(fp);
            if (rlen)
            {
                printf("cmd ack(%uBytes):%s", rlen, retcmd);
                if (strstr((char *)retcmd, "/usr/bin/time_daemon"))
                {
                    des_process_exsit_flag = 1;
                }
            }
        }

        free(retcmd);
    }

    if (des_process_exsit_flag == 0)
    {
        printf("des process is no exsit\r\n");
        // system("/usr/bin/time_daemon");
    }
}

int main2(int argc, char *argv[])
{
    while (1)
    {
        watch_time_daemon_process();
        sleep(5);
    }
}
