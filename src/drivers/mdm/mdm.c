#include "mdm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int update_sim_info_proc(void)
{
    char imei[32] = {0};
    char iccid[32] = {0};
    SimCard_Status_type simstatus;

    if (strlen(get_imei_ptr()) < 3)
    {
        memset(imei, 0, 32);
        getIMEI(imei, 32);
        if (is_string(imei, strlen(imei)))
            set_imei(imei, strlen(imei));
    }

    if (strlen(get_iccid_ptr()) < 3)
    {
        int i = 0;
        do
        {
            i++;
            memset(iccid, 0, 32);
            getICCID(iccid, 32);
            if (is_string(iccid, strlen(iccid)))
            {
                set_iccid(iccid, strlen(iccid));
                break;
            }
            else
            {
                sleep_ms(500);
            }
        } while (i < 5);
    }

    getSimCardStatus(&simstatus);

    if (simstatus.card_status == 1)
    {
        DBG_Print(LEVEL0, "%s", "SimCard status is OK\r\n");
        set_sim_card_status(1);
    }
    else
    {
        set_sim_card_status(0);
        DBG_Print(LEVEL0, "%s", "SimCard status is NOT OK\r\n");
    }

    return 0;
}

static boolean autoconnect = TRUE;

void ql_qcmapclient_sighandler(int signal)
{
    switch (signal)
    {
    case SIGTERM:
        // Ql_NW_MobileAP_Disable();
        //  exit(0);
        break;

    default:
        // QL_WWAN_LOG("Receiver unexpected signal %d\n",signal);
        break;
    }
}

void auto_ppp_proc(void)
{
    int ret;
    unsigned char mode;
    static unsigned int mode_time = 9;
    unsigned char *pt = get_rtc_time();
    if (is_time_format_ok(pt))
    {
        int rlen = exe_system_cmd("ping ev.cwjh.cn -c 1", buf, sizeof(buf));
        if (rlen)
        {
            if (strstr(buf, "seq=0"))
            {
                set_ppp_status(0, 1);
            }
            else
            {
                set_ppp_status(0, 0);
            }
        }
    }
    else
    {
        set_ppp_status(0, 0);
    }

    if (get_ppp_status(0) == 0)
    {
        mode_time++;
        if (mode_time > 10)
        {
            mode_time = 0;
            ret = dms_get_operating_mode(&mode);
            if (ret != 0)
            {
                DBG_Print(LEVEL0, "get operate mode fail!\n");
            }
            DBG_Print(LEVEL1, "operate mode: %d, time:%u\n", mode, (unsigned int)get_run_time());

            if (mode == 0)
            {
                ret = dms_set_operating_mode(1);
                if (ret != 0)
                {
                    DBG_Print(LEVEL0, "set operate mode fail!\n");
                }
            }
            else
            {
                ret = dms_set_operating_mode(0);
                if (ret != 0)
                {
                    DBG_Print(LEVEL0, "set operate mode fail!\n");
                }
            }
        }
    }
}

typedef void (*read_modem_at_func)(void *pdat);
unsigned int h_atc = 0;
read_modem_at_func at_callback_ptr = 0;

void modem_at_port_init(void (*p_read_modem_at_cb)(void *pdat))
{
    if (p_read_modem_at_cb)
        at_callback_ptr = p_read_modem_at_cb;
}

void send_at_cmd(void *pcmd)
{
    char finalRsp[1152 + 1] = {0};
    char resp_buf[1152 + 1] = {0};
    if (h_atc)
    {
        sendATCmd((char *)pcmd, finalRsp, resp_buf, 1152, 20);
        if (at_callback_ptr)
        {
            at_callback_ptr(finalRsp);
            at_callback_ptr(resp_buf);
        }

        DBG_Print(LEVEL2, "%s\r\n", resp_buf);
    }
}