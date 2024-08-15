#include "globalparam.h"

#if __CWJH_PLAT__
#define DEFAULT_MAGIC_FLAG 0x1114
const TSERVER defaultserver[] =
    {
        {"ev.cwjh.cn", 17055},
        {"10.0.1.95", 0},
        {"58.248.167.170", 0},
        {"111.111.111.111", 0},
        {"depot32960.flybees.com.cn", 0},
        {"g6check.vecc.org.cn", 0},
        {"111.111.111.111", 0},
        {"111.111.111.111", 0},

        {"129.211.170.236", 9300}};
#endif

#if __DST_PLAT__
#define DEFAULT_MAGIC_FLAG 0x1115
const TSERVER defaultserver[] =
    {
        {"ev.cwjh.cn", 17055},
        {"10.0.1.95", 0},
        {"58.248.167.170", 0},
        {"111.111.111.111", 0},
        {"depot32960.flybees.com.cn", 6658},
        {"g6check.vecc.org.cn", 0},
        {"111.111.111.111", 0},
        {"111.111.111.111", 0},

        {"129.211.170.236", 9300}};
#endif

#if __DF_HJ_PLAT__
#define DEFAULT_MAGIC_FLAG 0x1116
const TSERVER defaultserver[] =
    {
        {"ev.cwjh.cn", 0},
        {"10.0.1.95", 0},
        {"lcv.cwjh.cn", 18055},
        {"111.111.111.111", 0},
        {"depot32960.flybees.com.cn", 0},
        {"g6check.vecc.org.cn", 0},
        {"111.111.111.111", 0},
        {"111.111.111.111", 0},

        {"129.211.170.236", 9300}};
#endif

#if __EH43_PLAT__
#define DEFAULT_MAGIC_FLAG 0x1117
const TSERVER defaultserver[] =
    {
        {"gbevgw.dcvp.dfmc.com.cn", 15005},
        {"10.0.1.95", 0},
        {"58.248.167.170", 0},
        {"111.111.111.111", 0},
        {"depot32960.flybees.com.cn", 0},
        {"g6check.vecc.org.cn", 0},
        {"111.111.111.111", 0},
        {"111.111.111.111", 0},

        {"129.211.170.236", 9300}};
#endif

#if __SDDF_PLAT__
#define DEFAULT_MAGIC_FLAG 0x1118
const TSERVER defaultserver[] =
    {
        {"ev.cwjh.cn", 17055},
        {"10.0.1.95", 0},
        {"58.248.167.170", 0},
        {"111.111.111.111", 0},
        {"depot32960.flybees.com.cn", 0},
        {"g6check.vecc.org.cn", 0},
        {"comdata-dpca-tbox.catles.com", 1888},
        {"111.111.111.111", 0},

        {"129.211.170.236", 9300}};
#endif

#if __CN6_PLAT__
#define DEFAULT_MAGIC_FLAG 0x1119
const TSERVER defaultserver[] =
    {
        {"ev.cwjh.cn", 0},
        {"10.0.1.95", 0},
        {"58.248.167.170", 0},
        {"lcv.cwjh.cn", 16056},
        {"depot32960.flybees.com.cn", 0},
        {"g6check.vecc.org.cn", 0},
        {"comdata-dpca-tbox.catles.com", 0},
        {"111.111.111.111", 0},

        {"129.211.170.236", 9300}};
#endif

const char *default_apn[] =
    {
        "CMNET",
        "ROOT",
        "ROOT",
        "0",
        "0",
        "0",
};

typedef struct tagHWSysparam
{
    int (*load_sysparam_func)(void *sysparam, unsigned int sysparamlen);
    int (*save_sysparam_func)(void *sysparam, unsigned int sysparamlen);
    int (*load_capriceparam_func)(void *param, unsigned int paramlen);
    int (*save_capriceparam_func)(void *param, unsigned int paramlen);

    int (*load_capriceparam_bak_func)(void *param, unsigned int paramlen);
    int (*save_capriceparam_bak_func)(void *param, unsigned int paramlen);

    int (*disp_param_func)(void *, ...);
    unsigned char is_sysparam_need_analyze;
} HWSysparam;

HWSysparam s_stHWSysparam = {0};

char s_save_flag = 0;
GlobalParam s_stGlobalParam = {0};
GlobalParam s_stYzGlobalParam = {0};

SysRunParam s_stSysRunParam = {0};
CapriceParam s_stCapriceParam = {0};
CapriceParam s_stYzCapriceParam = {0};
CapriceParam s_stBakCapriceParam = {0};

void set_easy_change_param_init_flag(unsigned char val)
{
    s_stSysRunParam.easy_change_param_init_flag = val;
}

unsigned char get_easy_change_param_init_flag(void)
{
    return s_stSysRunParam.easy_change_param_init_flag;
}

void push_data_to_cache0(void *pdat, unsigned int datlen)
{
    unsigned int idx = s_stSysRunParam.cache_data_cur_write_idx;

    if ((pdat == 0) || (datlen == 0))
    {
        return;
    }

    if (datlen >= 1500)
    {
        datlen = 1500;
    }

    memcpy(s_stSysRunParam.cache_buf[idx], pdat, datlen);
    s_stSysRunParam.cache_data_len[idx] = datlen;

    s_stSysRunParam.cache_data_cur_write_idx = idx + 1;
    if (s_stSysRunParam.cache_data_cur_write_idx >= MAX_CACHE0_UNIT_CNT)
    {
        s_stSysRunParam.cache_data_cur_write_idx = 0;
    }
}

void push_data_to_cache1(void *pdat, unsigned int datlen)
{
    unsigned int idx = s_stSysRunParam.cache1_data_cur_write_idx;

    if ((pdat == 0) || (datlen == 0))
    {
        return;
    }

    if (datlen >= 1500)
    {
        datlen = 1500;
    }

    memcpy(s_stSysRunParam.cache1_buf[idx], pdat, datlen);
    s_stSysRunParam.cache1_data_len[idx] = datlen;

    s_stSysRunParam.cache1_data_cur_write_idx = idx + 1;
    if (s_stSysRunParam.cache1_data_cur_write_idx >= MAX_CACHE1_UNIT_CNT)
    {
        s_stSysRunParam.cache1_data_cur_write_idx = 0;
    }
}

void push_data_to_cache(unsigned char cache_data_type_id, void *pdat, unsigned int datlen)
{
    if (cache_data_type_id == 0)
    {
        push_data_to_cache0(pdat, datlen);
    }
    else if (cache_data_type_id == 1)
    {
        push_data_to_cache1(pdat, datlen);
    }
}

// 1 the last data   2 the second last data  3 the third last data ...
void *get_cache0_data_by_idx(unsigned int idx)
{
    unsigned int temp = 0;

    idx += 1;
    if (idx >= MAX_CACHE0_UNIT_CNT)
    {
        return 0;
    }

    if (s_stSysRunParam.cache_data_cur_write_idx >= idx)
    {
        temp = s_stSysRunParam.cache_data_cur_write_idx - idx;
    }
    else
    {
        temp = MAX_CACHE0_UNIT_CNT + s_stSysRunParam.cache_data_cur_write_idx - idx;
    }
    return s_stSysRunParam.cache_buf[temp];
}

void *get_cache1_data_by_idx(unsigned int idx)
{
    unsigned int temp = 0;

    idx += 1;
    if (idx >= MAX_CACHE1_UNIT_CNT)
    {
        return 0;
    }

    if (s_stSysRunParam.cache1_data_cur_write_idx >= idx)
    {
        temp = s_stSysRunParam.cache1_data_cur_write_idx - idx;
    }
    else
    {
        temp = MAX_CACHE1_UNIT_CNT + s_stSysRunParam.cache1_data_cur_write_idx - idx;
    }
    return s_stSysRunParam.cache1_buf[temp];
}

void *get_cache_data_by_idx(unsigned char cache_data_type_id, unsigned int idx)
{
    if (cache_data_type_id == 0)
    {
        return get_cache0_data_by_idx(idx);
    }
    else if (cache_data_type_id == 1)
    {
        return get_cache1_data_by_idx(idx);
    }
    else
    {
        return get_cache0_data_by_idx(idx);
    }
}

unsigned int get_cache0_data_len_by_idx(unsigned int idx)
{
    unsigned int temp = 0;

    idx += 1;
    if (idx >= MAX_CACHE0_UNIT_CNT)
    {
        return 0;
    }

    if (s_stSysRunParam.cache_data_cur_write_idx >= idx)
    {
        temp = s_stSysRunParam.cache_data_cur_write_idx - idx;
    }
    else
    {
        temp = MAX_CACHE0_UNIT_CNT + s_stSysRunParam.cache_data_cur_write_idx - idx;
    }

    return s_stSysRunParam.cache_data_len[temp];
}

unsigned int get_cache1_data_len_by_idx(unsigned int idx)
{
    unsigned int temp = 0;

    idx += 1;
    if (idx >= MAX_CACHE1_UNIT_CNT)
    {
        return 0;
    }

    if (s_stSysRunParam.cache1_data_cur_write_idx >= idx)
    {
        temp = s_stSysRunParam.cache1_data_cur_write_idx - idx;
    }
    else
    {
        temp = MAX_CACHE1_UNIT_CNT + s_stSysRunParam.cache1_data_cur_write_idx - idx;
    }

    return s_stSysRunParam.cache1_data_len[temp];
}

unsigned int get_cache_data_len_by_idx(unsigned char cache_data_type_id, unsigned int idx)
{
    if (cache_data_type_id == 0)
    {
        return get_cache0_data_len_by_idx(idx);
    }
    else if (cache_data_type_id == 1)
    {
        return get_cache1_data_len_by_idx(idx);
    }
    else
    {
        return get_cache0_data_len_by_idx(idx);
    }
}

unsigned char is_update_mcu_mode(void)
{
    return s_stSysRunParam.update_mcu_mode;
}

void set_update_mcu_mode(unsigned char val)
{
    s_stSysRunParam.update_mcu_mode = val;
}

unsigned char get_ppp_status(unsigned char channel)
{
    if (channel >= 3)
    {
        return 0;
    }
    // return s_stSysRunParam.ppp_status[channel];
    return 1;
}

void set_ppp_status(unsigned char channel, unsigned char val)
{
    if (channel >= 3)
    {
        return;
    }
    s_stSysRunParam.ppp_status[channel] = val;
}

unsigned char get_cgatt_flag(void)
{
    return s_stSysRunParam.cgatt_flag;
}

void set_cgatt_flag(unsigned char val)
{
    s_stSysRunParam.cgatt_flag = val;
}

int get_net_mode(void)
{
    return s_stSysRunParam.net_mode;
}

void set_net_mode(int val)
{
    s_stSysRunParam.net_mode = val;
}

unsigned int get_gps_speed(void)
{
    return s_stSysRunParam.gps_speed;
}

unsigned int get_gps_gaodu(void)
{
    return s_stSysRunParam.gps_gaodu;
}

unsigned int get_gps_angle(void)
{
    return s_stSysRunParam.gps_angle;
}

void set_gps_speed(unsigned int val)
{
    s_stSysRunParam.gps_speed = val;
}

void set_gps_gaodu(unsigned int val)
{
    s_stSysRunParam.gps_gaodu = val;
}

void set_gps_angle(unsigned int val)
{
    s_stSysRunParam.gps_angle = val;
}

unsigned char is_gps_ant_connect(void)
{
    return (s_stSysRunParam.mcu_status1 & (1 << 0)) ? 1 : 0;
}

unsigned char is_gprs_ant_connect(void)
{
    return (s_stSysRunParam.mcu_status1 & (1 << 1)) ? 1 : 0;
}

unsigned char is_can_ok(unsigned char channel)
{
    return (s_stSysRunParam.mcu_status1 & (1 << (2 + channel))) ? 1 : 0;
}

unsigned char is_acc_on(void)
{
    return (s_stSysRunParam.mcu_status1 & (1 << 5)) ? 1 : 0;
}

unsigned char get_gsenser_chip_status(void)
{
    return (s_stSysRunParam.mcu_status1 & (1 << 6)) ? 1 : 0;
}

unsigned char get_at24c08_chip_status(void)
{
    return (s_stSysRunParam.mcu_status1 & (1 << 7)) ? 1 : 0;
}

unsigned char get_store_cache_data_flag(void)
{
    return (s_stSysRunParam.mcu_status1 & (1 << 9)) ? 1 : 0;
}

unsigned char get_main_power_off_flag(void)
{
    return (s_stSysRunParam.mcu_status1 & (1 << 10)) ? 1 : 0;
}

unsigned int get_mcu_status1(void)
{
    return s_stSysRunParam.mcu_status1;
}

void set_mcu_status1(unsigned int val)
{
    s_stSysRunParam.mcu_status1 = val;
}

unsigned char is_jiami_chip_ok(void)
{
    return (s_stSysRunParam.mpu_status1 & (1 << 0)) ? 1 : 0;
}

void set_jiami_chip_status(unsigned char val)
{
    if (val)
    {
        s_stSysRunParam.mpu_status1 |= (1 << 0);
    }
    else
    {
        s_stSysRunParam.mpu_status1 &= (~(1 << 0));
    }
}

unsigned char is_emmc_ok(void)
{
    return (s_stSysRunParam.mpu_status1 & (1 << 1)) ? 1 : 0;
}

void set_emmc_status(unsigned char val)
{
    if (val)
    {
        s_stSysRunParam.mpu_status1 |= (1 << 1);
    }
    else
    {
        s_stSysRunParam.mpu_status1 &= (~(1 << 1));
    }
}

void set_mpu_gps_local_status(unsigned char val)
{
    if (val)
    {
        s_stSysRunParam.mpu_status1 |= (1 << 2);
    }
    else
    {
        s_stSysRunParam.mpu_status1 &= (~(1 << 2));
    }
}

unsigned char get_server_connect_status(unsigned char channel)
{
    return (s_stSysRunParam.mpu_status1 & (1 << (3 + channel))) ? 1 : 0;
}

void set_server_connect_status(unsigned char channel, unsigned char val)
{
    if (val)
    {
        s_stSysRunParam.mpu_status1 |= (1 << (3 + channel));
    }
    else
    {
        s_stSysRunParam.mpu_status1 &= (~(1 << (3 + channel)));
    }
}

void set_wifi_module_status(unsigned char val)
{
    if (val)
    {
        s_stSysRunParam.mpu_status1 |= (1 << 13);
    }
    else
    {
        s_stSysRunParam.mpu_status1 &= (~(1 << 13));
    }
}

unsigned char get_wifi_module_status(void)
{
    return (s_stSysRunParam.mpu_status1 & (1 << 13)) ? 1 : 0;
}

void set_wifi_client_status(unsigned char val)
{
    if (val)
    {
        s_stSysRunParam.mpu_status1 |= (1 << 14);
    }
    else
    {
        s_stSysRunParam.mpu_status1 &= (~(1 << 14));
    }
}

unsigned char get_wifi_client_status(void)
{
    return (s_stSysRunParam.mpu_status1 & (1 << 14)) ? 1 : 0;
}

unsigned char get_report_at_ack_flag(void)
{
    return s_stSysRunParam.report_at_ack_flag;
}

void set_report_at_ack_flag(unsigned char val)
{
    s_stSysRunParam.report_at_ack_flag = val;
}

char *get_at_cmd_buf_ptr(void)
{
    return s_stSysRunParam.at_cmd_buf;
}

unsigned char get_handshake_flag(void)
{
    return s_stSysRunParam.handshake_flag;
}

void set_handshake_flag(unsigned char val)
{
    s_stSysRunParam.handshake_flag = val;
}

unsigned int get_mpu_status1(void)
{
    return s_stSysRunParam.mpu_status1;
}

void set_mpu_status1(unsigned int val)
{
    s_stSysRunParam.mpu_status1 = val;
}

void set_dbm(int val)
{
    s_stSysRunParam.dbm = val;
}

int get_dbm(void)
{
    return s_stSysRunParam.dbm;
}

void set_server_change_flag(unsigned char channel, unsigned char val)
{
    if (val)
    {
        s_stSysRunParam.server_changed_flag |= (1 << channel);
    }
    else
    {
        s_stSysRunParam.server_changed_flag &= (~(1 << channel));
    }
}

unsigned char get_server_change_flag(unsigned char channel)
{
    return (s_stSysRunParam.server_changed_flag & (1 << channel)) ? 1 : 0;
}

void set_rtc_time_update_flag(unsigned char val)
{
    s_stSysRunParam.rtc_time_update_flag = val;
}

unsigned char get_rtc_time_update_flag(void)
{
    return s_stSysRunParam.rtc_time_update_flag;
}

unsigned char *get_rtc_time(void)
{
    return s_stSysRunParam.rtc_time;
}

void set_rtc_time(unsigned char time[6])
{
    if (time)
    {
        memcpy(s_stSysRunParam.rtc_time, time, 6);
    }
}
unsigned char get_out_sys_status_flag(void)
{
    return s_stSysRunParam.out_sys_status_flag;
}

void set_out_sys_status_flag(unsigned char val)
{
    s_stSysRunParam.out_sys_status_flag = val;
}

unsigned char *get_public_key_ptr(void)
{
    return s_stSysRunParam.public_key;
}

void _set_str(void *param, unsigned int param_size, void *pcontent, unsigned int contentlen)
{
    unsigned int templen = 0;

    if (pcontent && contentlen)
    {
        templen = contentlen;
        if (templen >= param_size)
        {
            templen = param_size;
        }
        memset(param, 0, param_size);
        memcpy(param, pcontent, templen);
    }
}

char *get_mcu_ver(void)
{
    return s_stSysRunParam.mcu_ver;
}

void set_mcu_ver(void *ptr, unsigned int len)
{
    _set_str(s_stSysRunParam.mcu_ver, sizeof(s_stSysRunParam.mcu_ver), ptr, len);
}

void set_iccid(void *ptr, unsigned int len)
{
    _set_str(s_stSysRunParam.iccid, sizeof(s_stSysRunParam.iccid), ptr, len);
}

void set_imei(void *ptr, unsigned int len)
{
    _set_str(s_stSysRunParam.imei, sizeof(s_stSysRunParam.imei), ptr, len);
}

char *get_iccid_ptr(void)
{
    return s_stSysRunParam.iccid;
}

char *get_imei_ptr(void)
{
    return s_stSysRunParam.imei;
}

unsigned int get_blind_data_cnt(unsigned char channel)
{
    if (channel >= 9)
    {
        return 0;
    }
    return s_stSysRunParam.blind_data_cnt[channel];
}

void set_blind_data_cnt(unsigned char channel, unsigned int val)
{
    if (channel >= 9)
    {
        return;
    }
    s_stSysRunParam.blind_data_cnt[channel] = val;
}

unsigned int get_main_power_vol(void)
{
    return s_stSysRunParam.main_power_vol;
}

void set_main_power_vol(unsigned int val)
{
    s_stSysRunParam.main_power_vol = val;
}

unsigned int get_bat_vol(void)
{
    return s_stSysRunParam.bat_vol;
}

void set_bat_vol(unsigned int val)
{
    s_stSysRunParam.bat_vol = val;
}

unsigned char get_mcu_report_sw(void)
{
    return s_stSysRunParam.mcu_report_sw;
}

void set_mcu_report_sw(unsigned char val)
{
    s_stSysRunParam.mcu_report_sw = val;
}

unsigned long get_run_time(void)
{
    return s_stSysRunParam.run_time;
}

void set_run_time(unsigned long val)
{
    s_stSysRunParam.run_time = val;
}

void _save_sysparam(void)
{
    if (s_stHWSysparam.save_sysparam_func)
    {
        if (s_save_flag)
        {
            s_save_flag = 0;
            s_stHWSysparam.save_sysparam_func(&s_stGlobalParam, sizeof(s_stGlobalParam));
        }
    }
}

void default_sysparam(void)
{
    unsigned int i = 0;

    sysparam_set_magic_flag(DEFAULT_MAGIC_FLAG);
    for (i = 0; i < MAX_SVR_LINK; i++)
    {
        sysparam_set_serveraddr(i, defaultserver[i].ip, strlen(defaultserver[i].ip));

        sysparam_set_serverport(i, defaultserver[i].port);
    }

    for (i = 0; i < MAX_APN_CNT; i++)
    {
        sysparam_set_apn(i, (char *)(default_apn[3 * i + 0]), strlen(default_apn[3 * i + 0]));
        sysparam_set_apn_usr(i, (char *)(default_apn[3 * i + 1]), strlen(default_apn[3 * i + 1]));
        sysparam_set_apn_psw(i, (char *)(default_apn[3 * i + 2]), strlen(default_apn[3 * i + 2]));
    }

    sysparam_set_func_select(FuncSelect_None);
}

void load_cfg_sysparam(void)
{
    char buf[1024] = {0};
    unsigned int i = 0;
    char tip[32] = {0};
    char tempbuf[32] = {0};

    if (s_stHWSysparam.load_sysparam_func)
        s_stHWSysparam.load_sysparam_func(buf, 1024);

    // magic flag
    if (GetSubString(buf, "<magic:", ">", tempbuf, sizeof(tempbuf)))
    {
        sysparam_set_magic_flag(atoi(tempbuf));
    }

    GetSubString(buf, "<vin:", ">", sysparam_get_vin(), 17);

    // ip port
    for (i = 0; i < MAX_SVR_LINK; i++)
    {
        sprintf(tip, "<ip%u:", i);
        GetSubString(buf, tip, ">", sysparam_get_serveraddr(i), MAX_SVR_ADDR_LEN);

        sprintf(tip, "<port%u:", i);
        GetSubString(buf, tip, ">", tempbuf, sizeof(tempbuf));

        sysparam_set_serverport(i, atoi(tempbuf));
    }

    // apn usr psw
    for (i = 0; i < MAX_APN_CNT; i++)
    {
        sprintf(tip, "<apn%u:", i);
        GetSubString(buf, tip, ">", sysparam_get_apn(i), MAX_APN_LEN);

        sprintf(tip, "<apnusr%u:", i);
        GetSubString(buf, tip, ">", sysparam_get_apn_usr(i), MAX_APN_USR_LEN);

        sprintf(tip, "<apnpsw%u:", i);
        GetSubString(buf, tip, ">", sysparam_get_apn_psw(i), MAX_APN_PSW_LEN);
    }

    if (GetSubString(buf, "<func_select:", ">", tempbuf, sizeof(tempbuf)))
    {
        sysparam_set_func_select(atoi(tempbuf));
    }

    if (GetSubString(buf, "<collect_file:", ">", tempbuf, sizeof(tempbuf)))
    {
        sysparam_set_collect_file(atoi(tempbuf));
    }

    if (GetSubString(buf, "<chipid:", ">", tempbuf, sizeof(tempbuf)))
    {
        sysparam_set_jiami_chip_id(tempbuf, strlen(tempbuf));
    }
}

int save_sysparam_file(void)
{
    char buf[1024] = {0};
    unsigned int rlen = 0;
    unsigned int i = 0;

    rlen += sprintf(buf + rlen, "<magic:%u>\r\n", sysparam_get_magic_flag());

    rlen += sprintf(buf + rlen, "<vin:%s>\r\n", sysparam_get_vin());

    for (i = 0; i < MAX_SVR_LINK; i++)
    {
        rlen += sprintf(buf + rlen, "<ip%u:%s>", i, sysparam_get_serveraddr(i));
        rlen += sprintf(buf + rlen, "<port%u:%d>", i, sysparam_get_serverport(i));
        rlen += sprintf(buf + rlen, "\r\n");
    }

    for (i = 0; i < MAX_APN_CNT; i++)
    {
        rlen += sprintf(buf + rlen, "<apn%u:%s>", i, sysparam_get_apn(i));
        rlen += sprintf(buf + rlen, "<apnusr%u:%s>", i, sysparam_get_apn_usr(i));
        rlen += sprintf(buf + rlen, "<apnpsw%u:%s>", i, sysparam_get_apn_psw(i));
        rlen += sprintf(buf + rlen, "\r\n");
    }

    rlen += sprintf(buf + rlen, "<func_select:%u>\r\n", sysparam_get_func_select());

    rlen += sprintf(buf + rlen, "<collect_file:%u>\r\n", sysparam_get_collect_file());

    rlen += sprintf(buf + rlen, "<chipid:%s>\r\n", sysparam_get_jiami_chip_id());

    if (s_stHWSysparam.save_sysparam_func)
    {
        s_stHWSysparam.save_sysparam_func(buf, rlen);
    }

    return 0;
}

int save_sysparam_to_store(void)
{
    if (s_stHWSysparam.is_sysparam_need_analyze) // need analyze format
    {
        save_sysparam_file();
    }
    else // store direct
    {
        if (s_stHWSysparam.save_sysparam_func)
        {
            s_stHWSysparam.save_sysparam_func(&s_stGlobalParam, sizeof(s_stGlobalParam));
        }
    }
    return 0;
}

void sysparam_init(unsigned char is_sysparam_need_analyze,
                   int (*load_sysparam_func)(void *sysparam, unsigned int sysparamlen),
                   int (*save_sysparam_func)(void *sysparam, unsigned int sysparamlen),
                   int (*load_capriceparam_func)(void *param, unsigned int paramlen),
                   int (*save_capriceparam_func)(void *param, unsigned int paramlen),

                   int (*load_capriceparam_bak_func)(void *param, unsigned int paramlen),
                   int (*save_capriceparam_bak_func)(void *param, unsigned int paramlen),

                   int (*disp_param_func)(void *, ...))
{
    s_stHWSysparam.load_sysparam_func = load_sysparam_func;
    s_stHWSysparam.save_sysparam_func = save_sysparam_func;
    s_stHWSysparam.disp_param_func = disp_param_func;
    s_stHWSysparam.is_sysparam_need_analyze = is_sysparam_need_analyze;

    s_stHWSysparam.load_capriceparam_func = load_capriceparam_func;
    s_stHWSysparam.save_capriceparam_func = save_capriceparam_func;
    s_stHWSysparam.load_capriceparam_bak_func = load_capriceparam_bak_func;
    s_stHWSysparam.save_capriceparam_bak_func = save_capriceparam_bak_func;

    if (load_capriceparam_func)
    {
        load_capriceparam_func(&s_stCapriceParam, sizeof(s_stCapriceParam));
    }

    if (load_capriceparam_bak_func)
    {
        load_capriceparam_bak_func(&s_stBakCapriceParam, sizeof(s_stBakCapriceParam));
    }

    // use the bak
    if ((s_stCapriceParam.lat == 0) || (s_stCapriceParam.lon == 0))
    {
        memcpy(&s_stCapriceParam, &s_stBakCapriceParam, sizeof(s_stBakCapriceParam));
    }
    memcpy(&s_stYzCapriceParam, &s_stCapriceParam, sizeof(s_stCapriceParam));

    if (load_sysparam_func)
    {
        if (is_sysparam_need_analyze)
        {
            load_cfg_sysparam();

            if (sysparam_get_magic_flag() != DEFAULT_MAGIC_FLAG)
            {
                printf("[analyze]%04X %04X create default sysparam\r\n", sysparam_get_magic_flag(), DEFAULT_MAGIC_FLAG);
                default_sysparam();
                save_sysparam_to_store();
            }
        }
        else
        {
            load_sysparam_func(&s_stGlobalParam, sizeof(s_stGlobalParam));

            if (sysparam_get_magic_flag() != DEFAULT_MAGIC_FLAG)
            {
                printf("[direct store]%04X %04X create default sysparam\r\n", sysparam_get_magic_flag(), DEFAULT_MAGIC_FLAG);
                default_sysparam();
                save_sysparam_to_store();
            }
        }
        memcpy(&s_stYzGlobalParam, &s_stGlobalParam, sizeof(s_stGlobalParam));
    }
}

unsigned char sysparam_get_collect_file(void)
{
    return s_stGlobalParam.collect_file_flag;
}

void sysparam_set_collect_file(unsigned char val)
{
    s_stGlobalParam.collect_file_flag = val;
}

FuncSelect sysparam_get_func_select(void)
{
    return (FuncSelect)(s_stGlobalParam.func_select);
}

void sysparam_set_func_select(FuncSelect val)
{
    s_stGlobalParam.func_select = (unsigned char)val;
}

unsigned short sysparam_get_magic_flag(void)
{
    return s_stGlobalParam.magic_flag;
}

void sysparam_set_magic_flag(unsigned short val)
{
    s_stGlobalParam.magic_flag = val;
}

char *sysparam_get_vin(void)
{
    return s_stGlobalParam.vin;
}

char *sysparam_get_serveraddr(unsigned char channel)
{
    if (channel >= MAX_SVR_LINK)
    {
        channel = 0;
    }
    return s_stGlobalParam.serveraddr[channel];
}

unsigned int sysparam_get_serverport(unsigned char channel)
{
    if (channel >= MAX_SVR_LINK)
    {
        channel = 0;
    }
    return s_stGlobalParam.serverport[channel];
}

char *sysparam_get_apn(unsigned char channel)
{
    if (channel >= MAX_APN_CNT)
    {
        channel = 0;
    }
    return s_stGlobalParam.apn[channel];
}

char *sysparam_get_apn_usr(unsigned char channel)
{
    if (channel >= MAX_APN_CNT)
    {
        channel = 0;
    }
    return s_stGlobalParam.apn_usr[channel];
}

char *sysparam_get_apn_psw(unsigned char channel)
{
    if (channel >= MAX_APN_CNT)
    {
        channel = 0;
    }
    return s_stGlobalParam.apn_psw[channel];
}

void sysparam_set_serverport(unsigned char channel, unsigned int val)
{
    if (channel >= MAX_SVR_LINK)
    {
        return;
    }
    if (s_stGlobalParam.serverport[channel] != val)
    {
        s_stGlobalParam.serverport[channel] = val;

        set_server_change_flag(channel, 1);
    }
}

char __sysparam_set_str(void *param, unsigned int param_size, void *pcontent, unsigned int contentlen)
{
    unsigned int templen = 0;

    if (param && param_size && pcontent && contentlen)
    {
        templen = contentlen;
        if (templen >= param_size)
        {
            templen = param_size;
        }
        if ((templen == strlen((char *)param)) && (memcmp(param, pcontent, templen) == 0))
        {
            return 0;
        }
        memset(param, 0, param_size);
        memcpy(param, pcontent, templen);
        return 1;
    }
    return 0;
}

char _sysparam_set_str(unsigned char channel, void *param, unsigned int param_size, void *pcontent, unsigned int contentlen, unsigned char max_channel)
{
    if (channel >= max_channel)
    {
        return 0;
    }

    return __sysparam_set_str(param, param_size, pcontent, contentlen);
}

void sysparam_set_serveraddr(unsigned char channel, void *pcontent, unsigned int contentlen)
{
    if (_sysparam_set_str(channel, &s_stGlobalParam.serveraddr[channel], MAX_SVR_ADDR_LEN, pcontent, contentlen, MAX_SVR_LINK))
    {
        set_server_change_flag(channel, 1);
    }
}

void sysparam_set_apn(unsigned char channel, void *pcontent, unsigned int contentlen)
{
    _sysparam_set_str(channel, &s_stGlobalParam.apn[channel], MAX_APN_LEN, pcontent, contentlen, MAX_APN_CNT);
}

void sysparam_set_apn_usr(unsigned char channel, void *pcontent, unsigned int contentlen)
{
    _sysparam_set_str(channel, &s_stGlobalParam.apn_usr[channel], MAX_APN_USR_LEN, pcontent, contentlen, MAX_APN_CNT);
}

void sysparam_set_apn_psw(unsigned char channel, void *pcontent, unsigned int contentlen)
{
    _sysparam_set_str(channel, &s_stGlobalParam.apn_psw[channel], MAX_APN_PSW_LEN, pcontent, contentlen, MAX_APN_CNT);
}

// jiami chip id
unsigned char *sysparam_get_jiami_chip_id(void)
{
    return s_stGlobalParam.jiami_chip_id;
}

void sysparam_set_jiami_chip_id(void *chipid, unsigned int chipidlen)
{
    if (chipid && chipidlen)
    {
        memset(s_stGlobalParam.jiami_chip_id, 0, sizeof(s_stGlobalParam.jiami_chip_id));
        memcpy(s_stGlobalParam.jiami_chip_id, chipid, (chipidlen >= sizeof(s_stGlobalParam.jiami_chip_id)) ? sizeof(s_stGlobalParam.jiami_chip_id) : chipidlen);
    }
}

unsigned int get_lon(void)
{
    return s_stCapriceParam.lon;
}

unsigned int get_lat(void)
{
    return s_stCapriceParam.lat;
}

void set_lon(unsigned int val)
{
    s_stCapriceParam.lon = val;
}

void set_lat(unsigned int val)
{
    s_stCapriceParam.lat = val;
}

void save_sysparam_proc(void)
{
    if (memcmp(&s_stYzGlobalParam, &s_stGlobalParam, sizeof(s_stGlobalParam)))
    {
        save_sysparam_to_store();
        memcpy(&s_stYzGlobalParam, &s_stGlobalParam, sizeof(s_stGlobalParam));
    }
}

void save_caprice_param_proc(void)
{
    if (memcmp(&s_stYzCapriceParam, &s_stCapriceParam, sizeof(s_stCapriceParam)))
    {
        if (s_stHWSysparam.save_capriceparam_func)
        {
            s_stHWSysparam.save_capriceparam_func(&s_stCapriceParam, sizeof(s_stCapriceParam));
        }
        memcpy(&s_stYzCapriceParam, &s_stCapriceParam, sizeof(s_stCapriceParam));
    }
}

void save_caprice_param_bak_proc(void)
{
    if (memcmp(&s_stBakCapriceParam, &s_stCapriceParam, sizeof(s_stCapriceParam)))
    {
        if (s_stHWSysparam.save_capriceparam_bak_func)
        {
            s_stHWSysparam.save_capriceparam_bak_func(&s_stCapriceParam, sizeof(s_stCapriceParam));
        }
        memcpy(&s_stBakCapriceParam, &s_stCapriceParam, sizeof(s_stCapriceParam));
    }
}

void display_sysparam(void)
{
    if (s_stHWSysparam.disp_param_func)
    {
        unsigned int i = 0;

        s_stHWSysparam.disp_param_func("vin:%s\r\n", sysparam_get_vin());

        for (i = 0; i < MAX_SVR_LINK; i++)
        {
            s_stHWSysparam.disp_param_func("(%u)%s:%u\r\n", i, sysparam_get_serveraddr(i), sysparam_get_serverport(i));
        }

        for (i = 0; i < 2; i++)
        {
            s_stHWSysparam.disp_param_func("(APN%u)%s %s %s\r\n", i, sysparam_get_apn(i), sysparam_get_apn_usr(i), sysparam_get_apn_psw(i));
        }

        s_stHWSysparam.disp_param_func("func_select:%u\r\n", sysparam_get_func_select());
    }
}
