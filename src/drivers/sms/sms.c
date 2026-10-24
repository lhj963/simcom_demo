#include "sms.h"

typedef struct tagSMS_Struction
{
    char phone[24];
    char send_content[140];
    unsigned char send_len;
}SMS_Struction;

SMS_Struction stSMS_Struction = {0};
char need_set_sms_center_num_flag = 0;

void analyze_sms(void *response,void (*sms_arrive_cb_func)(unsigned char *ptime,void *phone_num,void *psms,unsigned int sms_len))
{
    char *ptemp = (char *)response;
    char *ptemp1 = 0;
    char sms_content[384] = {0};
    char phone_num[24] = {0};
    char flag = 0;

    if ((ptemp == 0) || (strlen(ptemp) == 0) || (strlen(ptemp) >= 1152))
    {
        return ;
    }

    if (GetSubString(ptemp,"+CSCA: \"","\"",sms_content,sizeof(sms_content)))
    {
        if (strlen(sms_content) == 0)
        {
            need_set_sms_center_num_flag = 1;
        }
    }

    while (1)
    {
        ptemp1 = strstr(ptemp,"+CMGL:");
        if (ptemp1 == 0)
        {
            break;
        }

        GetSubString(ptemp1,"AD\",\"","\"",phone_num,sizeof(phone_num));

        if (GetSubString(ptemp1,"\r\n","\r\n",sms_content,sizeof(sms_content)) || GetSubString(ptemp1,"\n\r","\n\r",sms_content,sizeof(sms_content)))
        {
            flag = 1;
            //printf("(sms:%s)\r\n",sms_content);
            if (sms_arrive_cb_func)
            {
                sms_arrive_cb_func("\x17\x02\x02\x02\x02\x02",phone_num,sms_content,strlen(sms_content));
            }
        }

        ptemp = ptemp1 + strlen("+CMGL:");
    }

    //del sms
    if (flag)
    {
        send_at_cmd("AT+CMGD=1,4\r\n");
    }
}

int _send_sms(void *phone_num,void *sms,unsigned int sms_len)
{
    char cmd[256] = {0};
    unsigned int templen = 0;

    if ((phone_num == 0) || (sms == 0) || (sms_len == 0) || (strlen((char *)phone_num) < 3))
    {
        return 1;
    }

    snprintf(cmd,250,"AT+CMGS=\"%s\"\r\n",(char *)phone_num);
    send_at_cmd(cmd);
    usleep(150000);
    templen = (sms_len >= 140) ? 140 : sms_len;
    memcpy(cmd,sms,templen);
    cmd[templen] = 0x1A;
    cmd[templen + 1] = '\r';
    cmd[templen + 2] = '\n';
    cmd[templen + 3] = 0;
    send_at_cmd(cmd);
}

void sms_proc(void)
{
    static unsigned int s_sms_cnt = 0;

    s_sms_cnt++;
    if (s_sms_cnt <= 10)
    {
        send_at_cmd("AT+CPMS=\"ME\",\"ME\",\"ME\"\r\n");
        send_at_cmd("AT+CMGF=1\r\n");
        send_at_cmd("AT+CSCA?\r\n");
        if (need_set_sms_center_num_flag)
        {
            need_set_sms_center_num_flag = 0;
            send_at_cmd("AT+CSCA=\"+8613800200569\"\r\n");
        }
    }

    send_at_cmd("AT+CMGL=\"ALL\"\r\n");

    if (stSMS_Struction.send_len)
    {
        _send_sms(stSMS_Struction.phone,stSMS_Struction.send_content,stSMS_Struction.send_len);
        stSMS_Struction.send_len = 0;
    }
}

int send_sms(void *phone_num,void *sms,unsigned int sms_len)
{
    unsigned int templen = 0;

    strncpy(stSMS_Struction.phone,(char *)phone_num,24);
    templen = (sms_len >= 140) ? 140 : sms_len;
    memcpy(stSMS_Struction.send_content,sms,templen);
    stSMS_Struction.send_len = templen;
}
