#ifndef __GLOBALPARAM_H__
#define __GLOBALPARAM_H__

#ifdef __cplusplus
extern "C"
{
#endif

#define __CWJH_PLAT__ 1
#define __DST_PLAT__ 0
#define __DF_HJ_PLAT__ 0
#define __EH43_PLAT__ 0
#define __SDDF_PLAT__ 0
#define __CN6_PLAT__ 0

#if __CWJH_PLAT__
#define SOFTWARE_VERSION "BYSD_Z2021_MPU_V5.1.48"
#endif

#if __DST_PLAT__
#define SOFTWARE_VERSION "BYSD_Z2021_MPU_V5.2.48"
#endif

#if __DF_HJ_PLAT__
#define SOFTWARE_VERSION "BYSD_Z2021_MPU_V5.3.48"
#endif

#if __EH43_PLAT__
#define SOFTWARE_VERSION "BYSD_Z2021_MPU_V5.4.48"
#endif

#if __SDDF_PLAT__
#define SOFTWARE_VERSION "BYSD_Z2021_MPU_V5.5.48"
#endif

#if __CN6_PLAT__
#define SOFTWARE_VERSION "BYSD_Z2021_MPU_V5.6.48"
#endif

#define MAX_SVR_LINK 9
#define MAX_APN_CNT 2

#define MAX_SVR_ADDR_LEN 36
#define MAX_APN_LEN 36
#define MAX_APN_USR_LEN 36
#define MAX_APN_PSW_LEN 36

#define MAX_CACHE0_UNIT_CNT 63

#define MAX_CACHE1_UNIT_CNT 63

    typedef enum tagFuncSelect
    {
        FuncSelect_None = 0,
        FuncSelect_JMRH_SM4,
    } FuncSelect;

    typedef struct tagGlobalParam
    {
        unsigned short magic_flag;
        char vin[17 + 1];
        char serveraddr[MAX_SVR_LINK][MAX_SVR_ADDR_LEN + 1]; // 0:32960 1:17691 2:svr1 3:svr2
        unsigned int serverport[MAX_SVR_LINK];

        char apn[MAX_APN_CNT][MAX_APN_LEN + 1];         // apn
        char apn_usr[MAX_APN_CNT][MAX_APN_USR_LEN + 1]; // apn usr
        char apn_psw[MAX_APN_CNT][MAX_APN_PSW_LEN + 1]; // apn psw

        unsigned char func_select; // function select

        unsigned char collect_file_flag; // collect_file
        unsigned char jiami_chip_id[16 + 1];
    } GlobalParam;

    typedef struct tagCapriceParam
    {
        unsigned int lon;
        unsigned int lat;
    } CapriceParam;

#define AT_CMD_BUF_LEN 1536

    typedef struct tagSysRunParam
    {
        unsigned long run_time;

        char mcu_ver[36];

        char imei[21];
        char imsi[21];
        char iccid[21];
        int dbm;

        unsigned char easy_change_param_init_flag;
        unsigned char rtc_time_update_flag;
        unsigned char rtc_time[6];
        unsigned char star_num;
        unsigned char out_sys_status_flag;
        unsigned char report_at_ack_flag;
        char at_cmd_buf[AT_CMD_BUF_LEN];

        unsigned int server_changed_flag;

        unsigned char handshake_flag;
        unsigned int gps_speed;
        unsigned int gps_gaodu;
        unsigned int gps_angle;

        unsigned char cgatt_flag;
        int net_mode;
        unsigned char ppp_status[3];
        unsigned char update_mcu_mode;

        unsigned int main_power_vol;
        unsigned int bat_vol;
        unsigned char mcu_report_sw;
        unsigned int blind_data_cnt[9];

        // mcu_status1
        // 0bit gps ant
        // 1bit gprs ant
        // 2bit can0
        // 3bit can1
        // 4bit can2
        // 5bit acc status
        // 6bit g-senser
        // 7bit at24c08
        // 8bit gps module
        // 9bit store cache data flag
        unsigned int mcu_status1;
        unsigned int mcu_status2;
        unsigned int mcu_status3;
        unsigned int mcu_status4;

        // mpu_status1
        // 0bit: jiami_chip
        // 1bit: emmc
        // 2bit: gps local status
        // 3bit:channel0 connect status
        // 4bit:channel1 connect status
        // 5bit:channel2 connect status
        // 6bit:channel3 connect status
        // 7bit:channel4 connect status
        // 8bit:channel5 connect status
        // 9bit:channel6 connect status
        // 10bit:channel7 connect status
        // 11bit:channel8 connect status
        // 12bit:channel9 connect status
        // 13bit:wifi module status
        // 14bit:wifi client connect ok
        unsigned int mpu_status1;
        unsigned int mpu_status2;
        unsigned int mpu_status3;
        unsigned int mpu_status4;

        unsigned char public_key[128];

        unsigned char cache_buf[MAX_CACHE0_UNIT_CNT][1536];
        unsigned int cache_data_len[MAX_CACHE0_UNIT_CNT];
        unsigned int cache_data_cur_write_idx;

        unsigned char cache1_buf[MAX_CACHE1_UNIT_CNT][1536];
        unsigned int cache1_data_len[MAX_CACHE1_UNIT_CNT];
        unsigned int cache1_data_cur_write_idx;

    } SysRunParam;

    typedef struct tagTSERVER
    {
        char *ip;
        unsigned int port;
    } TSERVER;

    extern void sysparam_init(unsigned char is_sysparam_need_analyze,
                              int (*load_sysparam_func)(void *sysparam, unsigned int sysparamlen),
                              int (*save_sysparam_func)(void *sysparam, unsigned int sysparamlen),
                              int (*load_capriceparam_func)(void *param, unsigned int paramlen),
                              int (*save_capriceparam_func)(void *param, unsigned int paramlen),

                              int (*load_capriceparam_bak_func)(void *param, unsigned int paramlen),
                              int (*save_capriceparam_bak_func)(void *param, unsigned int paramlen),

                              int (*disp_param_func)(void *, ...));

    extern void save_caprice_param_proc(void);
    extern void save_caprice_param_bak_proc(void);
    extern void save_sysparam_proc(void);

    extern int save_sysparam_to_store(void);

    extern void set_easy_change_param_init_flag(unsigned char val);
    extern unsigned char get_easy_change_param_init_flag(void);

    extern void push_data_to_cache(unsigned char cache_data_type_id, void *pdat, unsigned int datlen);
    extern void *get_cache_data_by_idx(unsigned char cache_data_type_id, unsigned int idx);
    extern unsigned int get_cache_data_len_by_idx(unsigned char cache_data_type_id, unsigned int idx);

    extern unsigned int get_blind_data_cnt(unsigned char channel);
    extern void set_blind_data_cnt(unsigned char channel, unsigned int val);
    extern unsigned int get_main_power_vol(void);
    extern void set_main_power_vol(unsigned int val);
    extern unsigned int get_bat_vol(void);
    extern void set_bat_vol(unsigned int val);
    extern unsigned char get_mcu_report_sw(void);
    extern void set_mcu_report_sw(unsigned char val);

    extern unsigned char is_update_mcu_mode(void);
    extern void set_update_mcu_mode(unsigned char val);
    extern unsigned char get_ppp_status(unsigned char channel);
    extern void set_ppp_status(unsigned char channel, unsigned char val);
    extern unsigned char get_cgatt_flag(void);
    extern void set_cgatt_flag(unsigned char val);
    extern int get_net_mode(void);
    extern void set_net_mode(int val);
    extern unsigned int get_gps_speed(void);
    extern unsigned int get_gps_gaodu(void);
    extern unsigned int get_gps_angle(void);
    extern void set_gps_speed(unsigned int val);
    extern void set_gps_gaodu(unsigned int val);
    extern void set_gps_angle(unsigned int val);

    extern unsigned char get_report_at_ack_flag(void);
    extern void set_report_at_ack_flag(unsigned char val);
    extern char *get_at_cmd_buf_ptr(void);

    extern unsigned char get_handshake_flag(void);
    extern void set_handshake_flag(unsigned char val);
    extern unsigned char *get_public_key_ptr(void);

    extern unsigned char is_can_ok(unsigned char channel);
    extern unsigned char is_gps_ant_connect(void);
    extern unsigned char is_gprs_ant_connect(void);
    extern unsigned char is_acc_on(void);
    extern unsigned char get_gsenser_chip_status(void);
    extern unsigned char get_at24c08_chip_status(void);
    extern unsigned char get_store_cache_data_flag(void);
    extern unsigned char get_main_power_off_flag(void);

    extern unsigned int get_mcu_status1(void);
    extern void set_mcu_status1(unsigned int val);

    extern unsigned char is_jiami_chip_ok(void);
    extern void set_jiami_chip_status(unsigned char val);
    extern unsigned char is_emmc_ok(void);
    extern void set_emmc_status(unsigned char val);
    extern void set_mpu_gps_local_status(unsigned char val);
    extern unsigned char get_server_connect_status(unsigned char channel);
    extern void set_server_connect_status(unsigned char channel, unsigned char val);
    extern void set_wifi_module_status(unsigned char val);
    extern unsigned char get_wifi_module_status(void);
    extern void set_wifi_client_status(unsigned char val);
    extern unsigned char get_wifi_client_status(void);

    extern unsigned int get_mpu_status1(void);
    extern void set_mpu_status1(unsigned int val);
    extern void set_dbm(int val);
    extern int get_dbm(void);
    extern void set_server_change_flag(unsigned char channel, unsigned char val);
    extern unsigned char get_server_change_flag(unsigned char channel);
    extern void set_rtc_time_update_flag(unsigned char val);
    extern unsigned char get_rtc_time_update_flag(void);
    extern unsigned char *get_rtc_time(void);
    extern void set_rtc_time(unsigned char time[6]);
    extern unsigned char get_out_sys_status_flag(void);
    extern void set_out_sys_status_flag(unsigned char val);

    extern char *get_mcu_ver(void);
    extern void set_mcu_ver(void *ptr, unsigned int len);
    extern void set_iccid(void *ptr, unsigned int len);
    extern void set_imei(void *ptr, unsigned int len);
    extern char *get_iccid_ptr(void);
    extern char *get_imei_ptr(void);
    extern unsigned long get_run_time(void);
    extern void set_run_time(unsigned long val);

    extern unsigned char sysparam_get_collect_file(void);
    extern void sysparam_set_collect_file(unsigned char val);
    extern FuncSelect sysparam_get_func_select(void);
    extern void sysparam_set_func_select(FuncSelect val);
    extern unsigned short sysparam_get_magic_flag(void);
    extern char *sysparam_get_vin(void);
    extern char *sysparam_get_serveraddr(unsigned char channel);
    extern unsigned int sysparam_get_serverport(unsigned char channel);
    extern char *sysparam_get_apn(unsigned char channel);
    extern char *sysparam_get_apn_usr(unsigned char channel);
    extern char *sysparam_get_apn_psw(unsigned char channel);

    extern void sysparam_set_magic_flag(unsigned short val);
    extern void sysparam_set_serveraddr(unsigned char channel, void *pcontent, unsigned int contentlen);
    extern void sysparam_set_apn(unsigned char channel, void *pcontent, unsigned int contentlen);
    extern void sysparam_set_apn_usr(unsigned char channel, void *pcontent, unsigned int contentlen);
    extern void sysparam_set_apn_psw(unsigned char channel, void *pcontent, unsigned int contentlen);
    extern void sysparam_set_serverport(unsigned char channel, unsigned int val);
    extern unsigned char *sysparam_get_jiami_chip_id(void);
    extern void sysparam_set_jiami_chip_id(void *chipid, unsigned int chipidlen);

    extern void display_sysparam(void);

#ifdef __cplusplus
}
#endif

#endif
