#include "ATControl.h"
#ifdef FEATURE_DEMO_BT
#include "BTControl.h"
#endif
#include "DMSControl.h"
#include "DataCall.h"
#include "NASControl.h"
#include "OTAUpgrade.h"
#include "SMSControl.h"
#include "UIMControl.h"
#include "VoiceCall.h"
#include "WDSControl.h"
#include "DMZControl.h"
#include "WiFiControl.h"
#include "simcom_common.h"
#include <ctype.h>
#include <math.h>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef _SIMCOM_WINDOWS_
#include "app_timer.h"
#else
#include "LinuxTimer.h"
#endif
#include "ADC.h"
#include "ALSAControl.h"
#include "DEVControl.h"
#include "FTPControl.h"
//#include "GPSControl.h"
#include "GpioControl.h"
#include "PBControl.h"
#include "I2C.h"
#include "SSP.h"
#include "UbusContext.h"
#include "ftp_s.h"
#include "http_s.h"
#include "json-c.h"
#include "mqtt_s.h"
#include "openssl.h"
#include "simcom_demo_version.h"
#include "simcom_eatlbs.h"
#include "simcom_loc.h"
#include "simcom_test_main.h"
#include "wifi_c_test.h"
#include "sms_message_gsm.h"
#include "SystemControl.h"
#include "EcallControl.h"


#define SIG SIGALRM

#ifdef SIMCOM_TEST_UI
static pthread_mutex_t lock;
static pthread_cond_t cond;
static simcom_event_e event;

#ifdef FEATURE_DEMO_BT
static bt_msg s_bt_msg;
#endif

simcom_voice_call_info call_info;

static pthread_mutex_t s_btmutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t s_btcond = PTHREAD_COND_INITIALIZER;

static void simcom_wifi_test();
static void simcom_ecall_test();
static void simcom_nas_test();
static void simcom_voice_test();
static void simcom_wds_test();
static void simcom_datacall_test();
static void simcom_sms_test();
static void simcom_ota_test();
static void simcom_at_test();
static void simcom_gpio_test();
static void simcom_gps_test();
static void simcom_bt_test();
static void simcom_timer_test();
static void simcom_tcp_test();
static void simcom_adc_test();
static void simcom_uim_test();
static void simcom_dms_test();
static void simcom_i2c_test();
static void simcom_uart_test();
static void simcom_spi_test();
static void simcom_get_version();
static void simcom_eth_test();
static void simcom_ftp_test();
static void simcom_ssl_test();
static void simcom_http_s_test();
static void simcom_ftp_s_test();
static void simcom_mqtt_s_test();
static void simcom_alsa_test();
static void simcom_audio_test();
static void simcom_dev_test(void);
static void simcom_json_test();
static void simcom_system_test();
void simcom_lbs_test();
static void simcom_phonebook_test();
static void simcom_dmz_test();

#define QM_NR5G_INVALID_RSSI -32768
#define ENABLE_AUTO_SLEEP 1
#define DISABLE_AUTO_SLEEP 0
#define SIMCOM_GNSS_GPIO 4
#define LONGSHANG_GNSS_GPIO 8
#define SIMCOMDEMO_FALSE -1
#define SIMCOMDEMO_TRUE 0
#define MAX_SCAN_LENGTH 128



void print_option_menu(const char *options_list[], int array_size)
{
    int i, j;
    for (i = 0; i < (array_size / 2); i++)
    {
        printf("%s", options_list[2 * i]);
        for (j = 30 - strlen(options_list[2 * i]); j > 0; j--)
        {
            printf(" ");
        }
        printf("%s\n", options_list[2 * i + 1]);
    }

    if (array_size % 2 != 0)
    {
        printf("%s\n", options_list[array_size - 1]);
    }
}

void simcom_show_data_call_info(data_call_info_t *data_call_info)
{
    data_call_info_t *ptr = NULL;
    int i = 0;
    char tmp0[MAX_STR_LEN] = {0};
    char tmp1[MAX_STR_LEN] = {0};
    char tmp2[MAX_STR_LEN] = {0};
    char tmp3[MAX_STR_LEN] = {0};
    char tmp4[MAX_STR_LEN] = {0};

    if (ptr == data_call_info) 
    {
        printf("exit show_data_call_info: input data_call_response is NULL\n");
        return;
    }
    else
    {
        ptr = data_call_info;
    }

    printf("show_data_call_info: START\n");
    
    printf("cid:%d,status:%d,type:%d,suggestedRetryTime:%d,apn:[%s], netif=%s\n", ptr->cid, ptr->status,
            ptr->ip_type, ptr->suggested_retry_time, ptr->apn, ptr->netif);
    
    inet_ntop(AF_INET, &(ptr->ipv4_info.ip_addr), tmp0, MAX_STR_LEN);
    inet_ntop(AF_INET, &(ptr->ipv4_info.primary_dns), tmp1, MAX_STR_LEN);
    inet_ntop(AF_INET, &(ptr->ipv4_info.secondary_dns), tmp2, MAX_STR_LEN);
    inet_ntop(AF_INET, &(ptr->ipv4_info.gateway), tmp3, MAX_STR_LEN);
    inet_ntop(AF_INET, &(ptr->ipv4_info.netmask), tmp4, MAX_STR_LEN);

    printf("IPV4:ip_addr:%s,primary_dns:%s,secondary_dns:%s,gateway:%s,netmask:%s\n", 
        tmp0, tmp1, tmp2, tmp3, tmp4);

    memset(tmp0, 0x00, MAX_STR_LEN);
    memset(tmp1, 0x00, MAX_STR_LEN);
    memset(tmp2, 0x00, MAX_STR_LEN);
    memset(tmp3, 0x00, MAX_STR_LEN);
    memset(tmp4, 0x00, MAX_STR_LEN);
    inet_ntop(AF_INET6, &(ptr->ipv6_info.ipv6_addr), tmp0, MAX_STR_LEN);
    inet_ntop(AF_INET6, &(ptr->ipv6_info.primary_dns), tmp1, MAX_STR_LEN);
    inet_ntop(AF_INET6, &(ptr->ipv6_info.secondary_dns), tmp2, MAX_STR_LEN);
    inet_ntop(AF_INET6, &(ptr->ipv6_info.gateway), tmp3, MAX_STR_LEN);
    inet_ntop(AF_INET6, &(ptr->ipv6_info.netmask), tmp4, MAX_STR_LEN);

    printf("IPV6:ipv6_addr:%s,primary_dns:%s,secondary_dns:%s,gateway:%s,netmask:%s\n", 
        tmp0, tmp1, tmp2, tmp3, tmp4);
    
    printf("show_data_call_info: END\n");

}

int input_digitalized_check(char *input, int input_len)
{
    int i = 0;
    if (input[i] == '-' && input_len > 1)
    {
        i++;
    }

    for(; i < input_len; i++)
    {
        if(input[i] > '9' || input[i] < '0')
        {
            return -1;
        }
    }

    return 0;
}

void simcom_test_main()
{
    int opt = 0;
    int i, j;
    char scan_string[100] = {0};
    const char *options_list[] = {
#ifdef FEATURE_DEMO_WIFI
        "1. WIFI",
#endif
        "2. VOICE CALL",
        "3. DATA CALL",
        "4. SMS",
        "5. WDS(APN Manager)",
        "6. NAS",
        "7. AT",
        "8. OTA",
        "9. TTS",
        "10. GPIO",
        "11. GPS",
#ifdef FEATURE_DEMO_BT
        "12. Bluetooth",
#endif
        "13. TCP/UDP",
        "14. Timer",
        "15. ADC",
        "16. I2C",
        "17. UIM(SimCard)",
        "18. DMS(IMEI)",
        "19. UART",
        "20. SPI",
        "21. Version",
        "22. Ethernet",
        "23. FTP",
        "24. SSL",
        "25. HTTP(S)",
        "26. FTP(S)",
#ifdef FEATURE_DEMO_MQTTS
        "27. MQTT(S)",
#endif
        "28. JSON",
        "29. LBS",
        "30. SYSTEM",
        "31. PhoneBook",
        "32. DMZ",
        "33. ECALL",
        "99. EXIT",
    };

    int array_size = sizeof(options_list) / sizeof(options_list[0]);
    while (1)
    {
        printf("\nPlease select an option to test from the items listed below.\n");
        print_option_menu(options_list, sizeof(options_list) / sizeof(options_list[0]));
        printf("Option > ");

        memset(scan_string, 0, sizeof(scan_string));
        if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
            continue;

        /* Convert the option to an integer, and switch on the option entered. */
        opt = atoi(scan_string);
        switch (opt)
        {
        case 1:
#ifdef FEATURE_DEMO_WIFI
            simcom_wifi_test();
#else
            printf("WIFI Not supported Now...\n");
#endif

            break;
        case 2:
#ifdef FEATURE_DEMO_VCALL
            simcom_voice_test();
#else
            printf("Voice Call Not supported Now...\n");
#endif
            break;
        case 3:
#ifdef FEATURE_DEMO_DATACALL
            simcom_datacall_test();
#else
            printf("Data Call Not supported Now...\n");
#endif

            break;
        case 4:
#ifdef FEATURE_DEMO_SMS
            simcom_sms_test();
#else
            printf("SMS Not supported Now...\n");
#endif
            break;
        case 5:
#ifdef FEATURE_DEMO_DATACALL
            simcom_wds_test();
#else
            printf("Data Call Not supported Now...\n");
#endif
            break;
        case 6:
#ifdef FEATURE_DEMO_NAS
            simcom_nas_test();
#else
            printf("NAS Not supported Now...\n");
#endif
            break;
        case 7:
#ifdef FEATURE_DEMO_AT
            simcom_at_test();
#else
            printf("AT Not supported Now...\n");
#endif
            break;
        case 8:
#ifdef FEATURE_DEMO_FOTA
            simcom_ota_test();
#else
            printf("FOTA Not supported Now...\n");
#endif
            break;
        case 9:
#ifdef FEATURE_DEMO_TTS
            printf("\r\n please refer to simcomtts source code\n");
#else
            printf("TTS Not supported Now...\n");
#endif
            break;
        case 10:
#ifdef FEATURE_DEMO_GPIO
            simcom_gpio_test();
#else
            printf("GPIO Not supported Now...\n");
#endif
            break;
        case 11:
#ifdef FEATURE_DEMO_GPS
            simcom_gps_test();
#else
            printf("GPS Not supported Now...\n");
#endif
            break;
        case 12:
#ifdef FEATURE_DEMO_BT
            simcom_bt_test();
#else
            printf("BT Not supported Now...\n");
#endif
            break;
        case 13:
            simcom_tcp_test();
            break;
        case 14:
            simcom_timer_test();
            break;
        case 15:
#ifdef FEATURE_DEMO_ADC
            simcom_adc_test();
#else
            printf("ADC Not supported Now...\n");
#endif
            break;
        case 16:
#ifdef FEATURE_DEMO_I2C
            simcom_i2c_test();
#else
            printf("I2C Not supported Now...\n");
#endif
            break;
        case 17:
#ifdef FEATURE_DEMO_UIM
            simcom_uim_test();
#else
            printf("UIM Not supported Now...\n");
#endif
            break;
        case 18:
#ifdef FEATURE_DEMO_DMS
            simcom_dms_test();
#else
            printf("DMS Not supported Now...\n");
#endif
            break;
        case 19:
#ifdef FEATURE_DEMO_UART
            simcom_uart_test();
#else
            printf("UART Not supported Now...\n");
#endif
            break;
        case 20:
#ifdef FEATURE_DEMO_SPI
            simcom_spi_test();
#else
            printf("SPI Not supported Now...\n");
#endif
            break;
        case 21:
            simcom_get_version();
            break;
        case 22:
#ifdef FEATURE_DEMO_ETH
printf("\r\n please refer to simcometh source code\n");
#else
            printf("ETH Not supported Now...\n");
#endif
            break;
        case 23:
#ifdef FEATURE_DEMO_FTP
            simcom_ftp_test();
#else
            printf("FTP Not supported Now...\n");
#endif
            break;
        case 24:
#ifdef FEATURE_DEMO_SSL
            simcom_ssl_test();
#else
            printf("SSL Not supported Now...\n");
#endif
            break;
        case 25:
#ifdef FEATURE_DEMO_HTTPS
            simcom_http_s_test();
#else
            printf("HTTPS Not supported Now...\n");
#endif
            break;
        case 26:
#ifdef FEATURE_DEMO_FTPS
            simcom_ftp_s_test();
#else
            printf("FTPS Not supported Now...\n");
#endif
            break;
        case 27:
#ifdef FEATURE_DEMO_MQTTS
            simcom_mqtt_s_test();
#else
            printf("MQTTS Not supported Now...\n");
#endif
            break;
        case 28:
            simcom_json_test();
            break;
        case 29:
#ifdef FEATURE_DEMO_LBS
            simcom_lbs_test();
#else
            printf("LBS Not supported Now...\n");
#endif
            break;
        case 30:
#ifdef FEATURE_DEMO_SYSTEM
            simcom_system_test();
#else
            printf("SYSTEM Not supported Now...\n");
#endif
            break;
        case 31:
#ifdef FEATURE_DEMO_PHONEBOOK
            simcom_phonebook_test();
#else
            printf("PhoneBook Not supported Now...\n");
#endif
            break;
        case 32:
#ifdef FEATURE_DEMO_DMZ
            simcom_dmz_test();
#else
            printf("DMZ Not supported Now...\n");
#endif
            break;
        case 33:
#ifdef FEATURE_DEMO_ECALL
            simcom_ecall_test();
#else
            printf("ECALL Not supported Now...\n");
#endif
            break;

        case 99:
            exit(0);
        default:
            break;
        }
    }
}


#ifdef FEATURE_DEMO_LBS
void simcom_lbs_test()
{

    const char *options_list[] = {
        "1. Get longitude and latitude",
        "2. Get detail address",
        "99. back",
    };

    int opt;
    char scan_string[100] = {0};
    simcom_lbs_info_t lbs_info;
    int error_code = -1;

    /* LBS main menu*/
    while (1)
    {
        printf("\nPlease select an Type to test from the items listed below.\n");
        print_option_menu(options_list, sizeof(options_list) / sizeof(options_list[0]));
        printf("Type > ");

        memset(scan_string, 0x00, sizeof(scan_string));
        if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
            continue;

        opt = atoi(scan_string);
        switch (opt)
        {
        case 1: /*get longitude and latitude and detailaddress*/
        {
            memset(&lbs_info, 0x00, sizeof(simcom_lbs_info_t));
            error_code = simcom_lbs_get_info(&lbs_info);
            if ((error_code == SIMCOM_LBS_CODE_DEFUALT_VALUE)
             || (error_code == SIMCOM_LBS_CODE_SUCC_ERROR)
             || (error_code == SIMCOM_LBS_CODE_LBS_ERROR_SUCCESS)
             || (error_code == SIMCOM_LBS_CODE_LBS_PARAMETER_ERR)
             || (error_code == SIMCOM_LBS_CODE_LBS_ERROR_FAILED))
            {
                printf("+CLBS: %d,%f,%f,%u,%s\n", error_code, lbs_info.fLatitude,lbs_info.fLongitude,
                                    lbs_info.u16accuracy, lbs_info.u8DateTime);
            }
            else
            {
                printf("+CLBS: %d", error_code);
            }
        }
        break;

        case 2: /*get detail address*/
        {
            memset(&lbs_info, 0x00, sizeof(simcom_lbs_info_t));
            error_code = simcom_lbs_get_info(&lbs_info);
            if ((error_code == SIMCOM_LBS_CODE_DEFUALT_VALUE)
             || (error_code == SIMCOM_LBS_CODE_SUCC_ERROR)
             || (error_code == SIMCOM_LBS_CODE_LBS_ERROR_SUCCESS)
             || (error_code == SIMCOM_LBS_CODE_LBS_PARAMETER_ERR)
             || (error_code == SIMCOM_LBS_CODE_LBS_ERROR_FAILED))
            {
                printf("+CLBS: %d,%s\n", error_code, lbs_info.u8LocAddress);
            }
            else
            {
                printf("+CLBS: %d", error_code);
            }
        }
        break;       

        default:
            break;
        }


        if (opt == 99)
        {
            break;
        }
    }
}
#endif

#ifdef FEATURE_DEMO_DATACALL
static void simcom_datacall_test()
{
    const char *options_list[] = {
        "1. initial attach apn",
        "2. start data call",
        "3. stop data call",
        "4. get data call info",
        "99. back",
    };

    int opt = 0;
    int i, j;
    char scan_string[200] = {0};

    if (datacall_init() != 0)
    {
        return;
    }

    while (1)
    {
        printf("\nPlease select an option to test from the items listed below.\n");
        print_option_menu(options_list, sizeof(options_list) / sizeof(options_list[0]));
        printf("Option > ");

        memset(scan_string, 0, sizeof(scan_string));
        if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
            continue;

        /* Convert the option to an integer, and switch on the option entered. */
        opt = atoi(scan_string);
        switch (opt)
        {
        case 1:
        {
            wds_apn_profile_info apn_info;

            memset(&apn_info, 0x00, sizeof(apn_info));
            printf("\nPlease input apn >");
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                break;

            if (strlen(scan_string) ==0 || strlen(scan_string) >= 32)
            {
                printf("input param error\n");
                break;
            }
            else
            {
                scan_string[strlen(scan_string) - 1] = '\0';
            }
            strncpy(apn_info.apn, scan_string, WDS_MAX_STR_LEN);
            

            printf("\nPlease input ip type:0-IPV4V6 1-IPV4 2-IPV6 >");
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                break;

            if (strlen(scan_string) ==0)
            {
                printf("input param error\n");
                break;
            }
            else
            {
                scan_string[strlen(scan_string) - 1] = '\0';
            }
            
            apn_info.ip_type = atoi(scan_string);
            if (apn_info.ip_type == WDS_PDP_IP_TYPE_IPV4V6
             || apn_info.ip_type == WDS_PDP_IP_TYPE_IPV4
             || apn_info.ip_type == WDS_PDP_IP_TYPE_IPV6)
            {
                //do nothing
            }
            else
            {
                printf("input param error\n");
                break;
            }

            apn_info.auth_type = WDS_PDP_AUTH_TYPE_NONE;
            set_initial_attach_apn(apn_info);
        }
        break;
        
        case 2: /* start data call */
        {
            int cid;
            wds_apn_profile_info apn_info;

            memset(&apn_info, 0x00, sizeof(apn_info));
            printf("\nPlease input profile index >");
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                break;
            cid = atoi(scan_string);
            if (cid < 1 || cid > 4)
            {
                printf("please select the correct profile.\n");
                break;
            }

            printf("\nPlease input apn >");
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                break;

            if (strlen(scan_string) ==0 || strlen(scan_string) >= 32)
            {
                printf("input param error\n");
                break;
            }
            else
            {
                scan_string[strlen(scan_string) - 1] = '\0';
            }
            strncpy(apn_info.apn, scan_string, WDS_MAX_STR_LEN);
            

            printf("\nPlease input ip type:0-IPV4V6 1-IPV4 2-IPV6 >");
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                break;

            if (strlen(scan_string) ==0)
            {
                printf("input param error\n");
                break;
            }
            else
            {
                scan_string[strlen(scan_string) - 1] = '\0';
            }
            
            apn_info.ip_type = atoi(scan_string);
            if (apn_info.ip_type == WDS_PDP_IP_TYPE_IPV4V6
             || apn_info.ip_type == WDS_PDP_IP_TYPE_IPV4
             || apn_info.ip_type == WDS_PDP_IP_TYPE_IPV6)
            {
                //do nothing
            }
            else
            {
                printf("input param error\n");
                break;
            }
            
            apn_info.auth_type = WDS_PDP_AUTH_TYPE_NONE;
            active_data_call(cid, apn_info);
        }
        break;
        case 3: /* stop data call */
        {            
            int cid;
            printf("\nPlease input profile index:\n");
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                break;
            cid = atoi(scan_string);
            if (cid < 1 || cid > 4)
            {
                printf("please select the correct profile.\n");
                break;
            }
            deactive_data_call(cid);
        }
        break;
        case 4:
        {
            /* get data call infomation*/
            int cid;
            int ret;
            data_call_info_t data_call_info;

            memset(&data_call_info, 0x00, sizeof(data_call_info_t));
            printf("\nPlease input profile index:\n");
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                break;
            cid = atoi(scan_string);

            data_call_info.cid = cid;
            ret = query_data_call(&data_call_info);
            if (ret == 0)
            {
                //hhhandle = callinfo.handle;
                simcom_show_data_call_info(&data_call_info);
            }
        }
        break;
        default:
            break;
        }

        if (opt == 99)
        {
            break;
        }
    }

    datacall_deinit();
}

static void simcom_wds_test()
{
    const char *options_list[] =
        {
            "1. active default pdp",
            "2. deactive default pdp",
            "3. get default apn info",
            "4. set default apn info",
            "5. active auto apn",
            "6. get pdp context",
            "7. set auto connect config",
            "99. back",
        };

    int opt = 0;
    int i, j;
    char scan_string[100] = {0};
    int ret;

    while (1)
    {
        printf("\nPlease select an option to test from the items listed below.\n");
        print_option_menu(options_list, sizeof(options_list) / sizeof(options_list[0]));
        printf("Option > ");

        memset(scan_string, 0, sizeof(scan_string));
        if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
        {
            continue;
        }
        /* Convert the option to an integer, and switch on the option entered. */
        opt = atoi(scan_string);
        switch (opt)
        {
        case 1:
        {
            ret = active_default_pdp_context();
            printf("WDS: ==============================================\n");
            if (ret == 0)
            {
                printf("\nactive default pdp context successfully\n");
            }
            else
            {
                printf("\nactive default pdp context failed\n");
            }
            
            break;
        }
        case 2:
        {
            ret = deactive_default_pdp_context();
            printf("WDS: ==============================================\n");
            if (ret == 0)
            {
                printf("\ndeactive default pdp context successfully\n");
            }
            else
            {
                printf("\ndeactive default pdp context failed\n");
            }

            break;
        }

        case 3:
        {
            wds_apn_profile_info apn_info;
            memset(&apn_info, 0x00, sizeof(apn_info));
            
            ret = get_apn_profile_info(&apn_info);
            if (ret == 0)
            {
                printf("apn_info.apn=%s\n", apn_info.apn);
                printf("apn_info.user=%s\n", apn_info.username);
                printf("apn_info.password=%s\n", apn_info.password);
                printf("apn_info.ip_type=%d\n", apn_info.ip_type);
                printf("apn_info.auth_type=%d\n", apn_info.auth_type);
            }
            else
            {
                printf("get apn profile info failed\n");
            }
            break;
        }

        case 4:
        {
            wds_apn_profile_info apn_info;
            int ip_type;
            char apn_str[128] = {0};
            char username[128] = {0};
            char password[128] = {0};
            
            memset(&apn_info, 0x00, sizeof(wds_apn_profile_info));
            memset(scan_string, 0, sizeof(scan_string));
            printf("\nPlease input ip_type:(0:IPV4V6 1:IPV4 2:IPV6)>");
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
            {
                break;
            }
            ip_type = atoi(scan_string);

            if ((ip_type > 2) || (ip_type < 0))
            {
                printf("\nPlease enter the correct pdp_type:(0:IPV4V6 1:IPV4 2:IPV6)\n");
                break;
            }
            apn_info.ip_type = ip_type;

            printf("\nPlease input apn:");
            if (fgets(apn_str, sizeof(apn_str), stdin) == NULL)
            {
                break;
            }
            for (i = 0; i <= strlen(apn_str) - 1; i++)
            {
                if ((apn_str[i] == 0x0A) || (apn_str[i] == 0x0D))
                {
                    apn_str[i] = '\0';
                    break;
                }
            }
            strncpy(apn_info.apn, apn_str, WDS_MAX_STR_LEN);

            printf("\nPlease input username:");
            if (fgets(username, sizeof(username), stdin) == NULL)
            {
                break;
            }

            for (i = 0; i <= strlen(username) - 1; i++)
            {
                if ((username[i] == 0x0A) || (username[i] == 0x0D))
                {
                    username[i] = '\0';
                    break;
                }
            }
            strncpy(apn_info.username, username, WDS_MAX_STR_LEN);

            printf("\nPlease input password:");
            if (fgets(password, sizeof(password), stdin) == NULL)
            {
                break;
            }

            for (i = 0; i <= strlen(password) - 1; i++)
            {
                if ((password[i] == 0x0A) || (password[i] == 0x0D))
                {
                    password[i] = '\0';
                    break;
                }
            }
            strncpy(apn_info.password, password, WDS_MAX_STR_LEN);
            
            apn_info.auth_type = WDS_PDP_AUTH_TYPE_NONE;
        
            ret = set_apn_profile_info(apn_info);
            if (ret == 0)
            {
                printf("set apn profile info successfully\n");
            }
            else
            {
                printf("set apn profile info failed\n");
            }
            break;
        }

        case 5:
        {
            ret = active_auto_apn();
            if (ret == 0)
            {
                printf("active auto apn successfully\n");
            }
            else
            {
                printf("active auto apn failed\n");
            }
            break;
        }

        case 6:
        {
            data_call_info_t default_pdp_info;

            memset(&default_pdp_info, 0x00, sizeof(data_call_info_t));
            ret = get_default_pdp_context(&default_pdp_info);
            if (ret == 0)
            {
                printf("get pdp context successfully\n");
                simcom_show_data_call_info(&default_pdp_info);
            }
            else
            {
                printf("get pdp context failed\n");
            }
            break;
        }

        case 7:
        {
            int auto_connect;
        
            printf("\nPlease input ip_type:(0:Manual 1:Auto)>");
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
            {
                break;
            }
            auto_connect = atoi(scan_string);

            if ((auto_connect != 1) && (auto_connect != 0))
            {
                printf("\nPlease enter the correct config:(0:Manual 1:Auto)\n");
                break;
            }
        
            ret = set_auto_connect_config(auto_connect);
            if (ret == 0)
            {
                printf("set auto connect config successful\n");
            }
            else
            {
                printf("set auto connect_config failed\n");
            }
            break;
        }
        
        case 99:
            break;
        default:
            break;
        }

        if (opt == 99)
        {
            break;
        }
    }
}
#endif /* FEATURE_DEMO_DATACALL */

#ifdef FEATURE_DEMO_VCALL
static int simcomvoice_get_call(uint8_t type, uint8_t *call_id)
{
    int ret = -1;
    int i;
    if (type == 0) //MO
    {
        for (i = 0; i < call_info.call_num; i++)
        {
            if (call_info.call_info_list[i].index != 0)
            {
                *call_id = call_info.call_info_list[i].index;
                ret = 0;
                break;
            }
        }
    }
    if (type == 1) //MT
    {
        for (i = 0; i < call_info.call_num; i++)
        {
            if (call_info.call_info_list[i].index != 0)
            {
                *call_id = call_info.call_info_list[i].index;
                ret = 0;
                break;
            }
        }
    }
    if (type == 2) //END
    {
        for (i = 0; i < call_info.call_num; i++)
        {
            if (call_info.call_info_list[i].index != 0)
            {
                *call_id = call_info.call_info_list[i].index;
                ret = 0;
                break;
            }
        }
    }
    return ret;
}

static void simcom_voice_test()
{
    const char *options_list[] = {
        "1. Dial",
        "2. Answer",
        "3. End",
        "4. Set call waiting",
        "5. Query call waiting",
        "6. Set RX volume",
        "7. Set TX volume",
        "8. Set RX mute",
        "9. Set TX mute",
        "10. Send DTMF",
        "11. Get call infos",
        "12. Set audio device",
		"13. Get call forward info",
        "14. dective call forward",
        "15. active call forward",
        "16. regist call forward",
        "17. erase call forward",
        "99. Back",
    };

    int opt = 0;
    char scan_string[100] = {0};
    uint8_t call_id;

    if(voice_init(process_simcom_ind_message) != 0)
    {
        printf("voice_init failed\n");
        return;
    }

    while (1)
    {
        printf("\nPlease select an option to test from the items listed below.\n");
        print_option_menu(options_list, sizeof(options_list) / sizeof(options_list[0]));
        printf("Option > ");
        memset(scan_string, 0, sizeof(scan_string));
        if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
            continue;

        /* Convert the option to an integer, and switch on the option entered. */
        opt = atoi(scan_string);
        switch (opt)
        {
        case 1: //DIAL
        {
            get_all_call_info_msg(&call_info);
            if (simcomvoice_get_call(0, &call_id) == 0)
            {
                printf("Now is calling,Please waiting...\n");
                break;
            }
            printf("\nPlease input phone_num:");
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                continue;
            scan_string[strlen(scan_string) - 1] = 0;
            int ret = voice_dial(scan_string);
            if(ret != EXIT_SUCCESS)
            {
                printf("voice_dial faild! ret = %d\n", ret);
            }
            else
            {
                printf("voice_dial success\n");
            }
        }
        break;
        case 2: //ANSWER
            printf("answer start");
            get_all_call_info_msg(&call_info);
            if (simcomvoice_get_call(1, &call_id) == -1)
            {
                printf("No Incoming call...\n");
                break;
            }
            if (call_info.call_num < 1)
            {
                break;
            }
            else if (call_info.call_num == 1)
            {
                voice_mtcall_process(VOICE_CALL_ANSWER);
            }
            else
            {
                int i = 0;
                int call_cmd = 0;
                printf("\nThere are many calls at the moment:\n");

                for (i = 0; i < call_info.call_num; i++)
                {
                    printf("MULTIPLE CALL:%d,%d,%s\n", call_info.call_info_list[i].index, call_info.call_info_list[i].state, call_info.call_info_list[i].number);
                }
                printf("\nplease select option:\n");
                printf("1:Accept the new call and place the active calls on hold\n");
                printf("2:Accept the new call and terminate the active calls\n");
                printf("3:Terminate the new call\n");
                memset(scan_string, 0, sizeof(scan_string));
                if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                    continue;
                call_cmd = atoi(scan_string);
                if (call_cmd == 1)
                {
                    voice_mtcall_process(VOICE_CALL_HOLD);
                }
                else if (call_cmd == 2)
                {
                    voice_mtcall_process(VOICE_END_FG);
                }
                else if (call_cmd == 3)
                {
                    voice_mtcall_process(VOICE_END_BG);
                }
                else
                {
                    continue;
                }
            }
            break;
        case 3: //END
            get_all_call_info_msg(&call_info);
            if (simcomvoice_get_call(2, &call_id) == -1)
            {
                printf("Nowing no calling...\n");
                break;
            }
            if (call_info.call_num < 1)
            {
                break;
            }
            else if (call_info.call_num == 1)
            {
                voice_mtcall_process(VOICE_CALL_HANDUP);
            }
            else
            {
                int i = 0;
                int call_cmd = 0;
                printf("\nThere are many calls at the moment:\n");

                for (i = 0; i < call_info.call_num; i++)
                {
                    printf("MULTIPLE CALL:%d,%d,%s\n", call_info.call_info_list[i].index, call_info.call_info_list[i].state, call_info.call_info_list[i].number);
                }
                printf("\nplease select option:\n");
                printf("1:Terminate all calls\n");
                printf("2:Terminate all active calls and accept the other calls\n");
                printf("3:Terminate all hold calls\n");
                memset(scan_string, 0, sizeof(scan_string));
                if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                    continue;
                call_cmd = atoi(scan_string);
                if (call_cmd == 1)
                {
                    voice_mtcall_process(VOICE_CALL_HANDUP);
                }
                else if (call_cmd == 2)
                {
                    voice_mtcall_process(VOICE_END_FG);
                }
                else if (call_cmd == 3)
                {
                    voice_mtcall_process(VOICE_END_BG);
                }
                else
                {
                    continue;
                }
            }
            break;
        case 4:
        {
            int action;
            printf("\nplease input call waiting action:0-disable 1:enable >");
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                continue;

            action = atoi(scan_string);
            if (action == 0 || action == 1)
            {
                if (set_call_waiting(action) == EXIT_SUCCESS)
                {
                    printf("%s call waiting successfully\n",(action == 1 ? "enable":"disable"));
                }
                else
                {
                    printf("set call waiting failed\n");
                }
            }
            else
            {
                printf("input error\n");
                continue;
            }
        }
        break;

        case 5:
        {
            int action = -1;
            if (query_call_waiting(&action) == EXIT_SUCCESS)
            {
                printf("call waiting:%d\n", action);
            }
            else
            {
                printf("query call waiting failed\n");
            }
        }
        break;

        case 6:
        {
            int volume;

            //volume
            printf("\nplease input volume,range[-36~12] >");
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                continue;

            if (input_digitalized_check(scan_string, strlen(scan_string) - 1) == -1)
            {
                printf("error input");
                continue;
            }

            volume = atoi(scan_string);
            if (volume < -36 || volume > 12)
            {
                printf("input error,range[-36~12]\n");
                continue;
            }

            if (set_volume(CALL_VOLUME_CONFIG_RX, volume) == EXIT_SUCCESS)
            {
                printf("set volume successfully\n");
            }
            else
            {
                printf("set volume failed\n");
            }

        }
        break;

        case 7:
        {
            int volume;

            //volume
            printf("\nplease input volume,range[-36~12] >");
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                continue;

            if (input_digitalized_check(scan_string, strlen(scan_string) - 1) == -1)
            {
                printf("error input");
                continue;
            }

            volume = atoi(scan_string);
            if (volume < -36 || volume > 12)
            {
                printf("input error,range[-36~12]\n");
                continue;
            }

            if (set_volume(CALL_VOLUME_CONFIG_TX, volume) == EXIT_SUCCESS)
            {
                printf("set volume successfully\n");
            }
            else
            {
                printf("set volume failed\n");
            }

        }
        break;

        case 8:
        {
            int action;

            //volume
            printf("\nplease input action:0-unmute 1:mute >");
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                continue;

            if(input_digitalized_check(scan_string, strlen(scan_string) - 1) == -1)
            {
                printf("error input");
                continue;
            }

            action = atoi(scan_string);
            if (action < 0 || action > 1)
            {
                printf("input error:0-unmute 1:mute\n");
                continue;
            }

            if (set_mute(CALL_VOLUME_CONFIG_RX, action) == EXIT_SUCCESS)
            {
                printf("set mute successfully\n");
            }
            else
            {
                printf("set mute failed\n");
            }

        }
        break;

        case 9:
        {
            int action;

            //volume
            printf("\nplease input action:0-unmute 1:mute >");
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                continue;
            
            if(input_digitalized_check(scan_string, strlen(scan_string) - 1) == -1)
            {
                printf("error input");
                continue;
            }
            action = atoi(scan_string);
            if (action < 0 || action > 1)
            {
                printf("input error:0-unmute 1:mute\n");
                continue;
            }

            if (set_mute(CALL_VOLUME_CONFIG_TX, action) == EXIT_SUCCESS)
            {
                printf("set mute successfully\n");
            }
            else
            {
                printf("set mute failed\n");
            }

        }
        break;
        
        case 10:
        {
            int code;

            //volume
            printf("\nplease input a DTMF (0,1,2,3,4,5,6,7,8,9,*,#,A,B,C,D) >");
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                continue;

            if (strlen(scan_string) > 2)//"1\n"
            {
                printf("error input\n");
                continue;
            }

            if (send_dtmf(scan_string[0]) == EXIT_SUCCESS)
            {
                printf("send DTMF successfully\n");
            }
            else
            {
                printf("send DTMF failed\n");
            }
        }
        break;

        case 11:
        {
            int i = 0;
            memset(&call_info, 0x00, sizeof(simcom_voice_call_info));
            get_all_call_info_msg(&call_info);
            if(call_info.call_num == 0)
            {
                printf("No Call Info Now\n");
            }
            for(i = 0; i < call_info.call_num; i++)
            {
                 if (call_info.call_info_list[i].index != 0) 
                 {
                     if (call_info.call_info_list[i].direction == VOICE_CALL_DIRECTION_MO)
                     {
                         printf("MO Call:", call_info.call_info_list[i].direction);
                     }
                     else if (call_info.call_info_list[i].direction == VOICE_CALL_DIRECTION_MT)
                     {
                         printf("MT Call:", call_info.call_info_list[i].direction);
                     }
                     else
                     {
                         printf("Direction[%d] Call:", call_info.call_info_list[i].direction);
                     }
            
                     switch(call_info.call_info_list[i].state)
                     {
                         case VOICE_CALL_STATE_ACTIVE:
                             printf("Call %s ACTIVE\n", call_info.call_info_list[i].number);
                             break;
                         case VOICE_CALL_STATE_HOLDING:
                             printf("Call %s HOLDING\n", call_info.call_info_list[i].number);
                             break;
                         case VOICE_CALL_STATE_DIALING:
                             printf("Call %s DIALING...\n", call_info.call_info_list[i].number);
                             break;
                         case  VOICE_CALL_STATE_ALERTING:
                             printf("Call %s ALERTING\n", call_info.call_info_list[i].number);
                             break;
                         case VOICE_CALL_STATE_INCOMING:
                             printf("Call %s INCOMING\n", call_info.call_info_list[i].number);
                             break;
                         case VOICE_CALL_STATE_WAITING:
                             printf("Call %s WAITING\n", call_info.call_info_list[i].number);
                             break;
                         case VOICE_CALL_STATE_OFFERING:
                             printf("Call %s OFFERING\n", call_info.call_info_list[i].number);
                             break;
                         case VOICE_CALL_STATE_DISCONNECTING:
                             printf("Call %s DISCONNECTING\n", call_info.call_info_list[i].number);
                             break;
                         case VOICE_CALL_STATE_DISCONNECTED:
                             printf("Call %s DISCONNECTED\n", call_info.call_info_list[i].number);
                             break;
                         default:
                             printf("unprocess call_state %d\n", call_info.call_info_list[i].state);
                             break;
                     }
                 }
                 else
                 {
                     printf(">>>>>>voice callback\n");
                 }
             }
        }
        break;

        case 12:
        {
            int type;

            //volume
            printf("\nplease input audio device (0-EARPIECE,1-SPEAKER,2-HEADSET) >");
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                continue;
            
            if(input_digitalized_check(scan_string, strlen(scan_string) - 1) == -1)
            {
                printf("error input");
                continue;
            }
            type = atoi(scan_string);

            if (type < AUDIO_DEVICE_EARPIECE || type > AUDIO_DEVICE_HEADSET)
            {
                printf("param error\n");
                continue;
            }

            if (set_audio_device((audio_device_type)type) == EXIT_SUCCESS)
            {
                printf("set audio device successfully\n");
            }
            else
            {
                printf("set audio device failed\n");
            }
        }
        break;

        case 13:
        {
            int reason = 0;
            CallForward_Info info = {0};
            printf("input reason(0:unconditional 1:mobile busy 2:no reply 3:not reachable):");
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
            {
                continue;
            }
            if(input_digitalized_check(scan_string, strlen(scan_string) - 1) == -1)
            {
                printf("error input");
                continue;
            }
            reason = atoi(scan_string);
            if(get_callforward_info(reason, &info) == EXIT_SUCCESS)
            {
                printf("status:%d, reason:%d, serviceClass:%d, number:%s\n", info.status, info.reason, info.serviceClass, info.number);
            }
            else
            {
                printf("fail\n");
            }
        }
        break;

        case 14:
        {
            int reason = 0;
            printf("input reason(0:unconditional 1:mobile busy 2:no reply 3:not reachable):");
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
            {
                continue;
            }
            if(input_digitalized_check(scan_string, strlen(scan_string) - 1) == -1)
            {
                printf("error input");
                continue;
            }
            reason = atoi(scan_string);
            if(dective_callforward(reason) == EXIT_SUCCESS)
            {
                printf("success\n");
            }
            else
            {
                printf("fail\n");
            }
        }
        break;

        case 15:
        {
            int reason = 0;
            printf("input reason(0:unconditional 1:mobile busy 2:no reply 3:not reachable):");
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
            {
                continue;
            }
            if(input_digitalized_check(scan_string, strlen(scan_string) - 1) == -1)
            {
                printf("error input");
                continue;
            }
            reason = atoi(scan_string);
            if(active_callforward(reason) == EXIT_SUCCESS)
            {
                printf("success\n");
            }
            else
            {
                printf("fail\n");
            }
        }
        break;

        case 16:
        {
            int reason = 0;
            int mode = 0;
            char number[VOICE_NUMBER_MAX] = {0};
            printf("input reason(0:unconditional 1:mobile busy 2:no reply 3:not reachable):");
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
            {
                continue;
            }
            if(input_digitalized_check(scan_string, strlen(scan_string) - 1) == -1)
            {
                printf("error input");
                continue;
            }
            reason = atoi(scan_string);
            printf("input number:");
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
            {
                continue;
            }
            memcpy(number, scan_string, strlen(scan_string));
            number[strlen(number) - 1] = '\0';
            printf("number:%s, length:%d\n", number, strlen(number));
            if(regist_callforward(reason, number) == EXIT_SUCCESS)
            {
                printf("success\n");
            }
            else
            {
                printf("fail\n");
            }
        }
        break;

        case 17:
        {
            int reason = 0;
            printf("input reason(0:unconditional 1:mobile busy 2:no reply 3:not reachable):");
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
            {
                continue;
            }
            if(input_digitalized_check(scan_string, strlen(scan_string) - 1) == -1)
            {
                printf("error input");
                continue;
            }
            reason = atoi(scan_string);
            if(erase_callforward(reason) == EXIT_SUCCESS)
            {
                printf("success\n");
            }
            else
            {
                printf("fail\n");
            }
        }
        break;
        default:
            break;
        }

        if (opt == 99)
        {
            break;
        }
    }

    voice_deinit();
}
#endif

#ifdef FEATURE_DEMO_NAS
int radio_tech_type_to_network_type(RIL_RadioTechnology netState)
{
    switch (netState)
    {
        case RADIO_TECH_GPRS:
        case RADIO_TECH_EDGE:
        case RADIO_TECH_IS95A:
        case RADIO_TECH_IS95B:
        case RADIO_TECH_1xRTT:            
        case RADIO_TECH_GSM:        
            return 2;//2G
        case RADIO_TECH_UMTS:
        case RADIO_TECH_EVDO_0:
        case RADIO_TECH_EVDO_A:
        case RADIO_TECH_HSDPA:
        case RADIO_TECH_HSUPA:
        case RADIO_TECH_HSPA:
        case RADIO_TECH_EVDO_B:
        case RADIO_TECH_EHRPD:
        case RADIO_TECH_HSPAP:
        case RADIO_TECH_TD_SCDMA:
        case RADIO_TECH_DC_HSPA:
            return 3;//3G
        case RADIO_TECH_LTE:
        case RADIO_TECH_LTEP:
            return 4;//4G
        case RADIO_TECH_IWLAN:
        default:
            return 0;
    } 

    return 0;
}

char * radio_tech_type_to_string(RIL_RadioTechnology netState)
{
    switch (netState)
    {
        case RADIO_TECH_GPRS:
            return "GPRS";
        case RADIO_TECH_EDGE:
            return "EDGE";
        case RADIO_TECH_UMTS:
            return "UMTS";
        case RADIO_TECH_IS95A:
            return "IS95A";
        case RADIO_TECH_IS95B:
            return "IS95B";
        case RADIO_TECH_1xRTT:
            return "1xRTT";
        case RADIO_TECH_EVDO_0:
            return "EVDO_0";
        case RADIO_TECH_EVDO_A:
            return "EVDO_A";
        case RADIO_TECH_HSDPA:
            return "HSDPA";
        case RADIO_TECH_HSUPA:
            return "HSUPA";
        case RADIO_TECH_HSPA:
            return "HSPA";
        case RADIO_TECH_EVDO_B:
            return "EVDO_B";
        case RADIO_TECH_EHRPD:
            return "EHRPD";
        case RADIO_TECH_LTE:
            return "LTE";
        case RADIO_TECH_HSPAP:
            return "HSPAP";
        case RADIO_TECH_GSM:
            return "GSM";
        case RADIO_TECH_TD_SCDMA:
            return "TD_SCDMA";
        case RADIO_TECH_IWLAN:
            return "IWLAN";
        case RADIO_TECH_LTEP:
            return "LTEP";
        case RADIO_TECH_DC_HSPA:
            return "DC_HSPA";
        default:
            return "Unknown";
    }
}


int app_nas_printf_serving_system(SIM_NetState serving_system)
{
    if (serving_system.csState == CS_STATE_REGISTERED)
    {
        printf("NAS: CS_REGISTERED!\n");
    }
    else if (serving_system.csState == CS_STATE_NOT_REGISTERED_SEARCHING)
    {
        printf("NAS: CS_NOT_REGISTERED_SEARCHING!\n");
    }
    else if (serving_system.csState == CS_STATE_REGISTRATION_DENIED)
    {
        printf("NAS: CS_REGISTRATION_DENIED!\n");
    }
    else if (serving_system.csState == CS_STATE_UNKNOWN)
    {
        printf("NAS: CS_REGISTRATION_UNKNOWN!\n");
    }
    else if (serving_system.csState == CS_STATE_REGISTERED_ROAMING)
    {
        printf("NAS: CS_REGISTRED_ROAMING!\n");
    }
    else
    {
        printf("NAS: CS_NOT_REGISTERED!\n");
    }

    if (serving_system.psState == PS_STATE_REGISTERED)
    {
        printf("NAS: PS_REGISTERED!\n");
    }
    else if (serving_system.psState == PS_STATE_NOT_REGISTERED_SEARCHING)
    {
        printf("NAS: PS_NOT_REGISTERED_SEARCHING!\n");
    }
    else if (serving_system.psState == PS_STATE_REGISTRATION_DENIED)
    {
        printf("NAS: PS_REGISTRATION_DENIED!\n");
    }
    else if (serving_system.psState == PS_STATE_UNKNOWN)
    {
        printf("NAS: PS_REGISTRATION_UNKNOWN!\n");
    }
    else if (serving_system.psState == PS_STATE_REGISTERED_ROAMING)
    {
        printf("NAS: PS_REGISTRED_ROAMING!\n");
    }
    else
    {
        printf("NAS: PS_NOT_REGISTERED!\n");
    }

    printf("RADIO TECH TYPE: %s\n", radio_tech_type_to_string(serving_system.netState));
    printf("NETWORK TYPE: %dG\n", radio_tech_type_to_network_type(serving_system.netState));    
}

int app_nas_printf_phonemode(int phone_mode)
{
    static char* phone_mode_map[] =
    {
        "online",
        "low power",
        "factory mode",
        "persistent low power",
        "offline",
    };

    if(phone_mode < 0 || phone_mode >= 5)
    {
        printf("error phone mode");
        return -1;
    }

    printf("phone mode:%d(%s)",phone_mode,phone_mode_map[phone_mode]);
}

int app_nas_printf_cell_info(CellInfo_Node *cellinfo_head)
{
    CellInfo_Node *pos = cellinfo_head;
    if (pos == NULL)
    {
        printf("%d,info is NULL\n",__LINE__);
        return -1;
    }

    static char* cellinfo_type_map[] =
	{
	    "ERROR",
        "GSM",
		"CDMA",
		"LTE",
		"WCDMA",
		"TD_SCDMA"
	};

    static char* timestamp_type_map[] =
	{
	    "UNKNOWN",
        "ANTENNA",
		"MODEM",
		"OEM_RIL",
		"JAVA_RIL"
	};
    printf("===============cell info===============\n");
    while(pos)
    {
        printf("cellInfoType:%s\n",cellinfo_type_map[pos->cellinfo.cellInfoType]);
        printf("registered:%s\n",pos->cellinfo.registered ? "serving cell":"neighbor cell");
        printf("timeStampType:%s\n",timestamp_type_map[pos->cellinfo.timeStampType]);
        printf("timeStamp:%d\n",pos->cellinfo.timeStampType);
        switch(pos->cellinfo.cellInfoType)
        {
            case RIL_CELL_INFO_TYPE_GSM:
            {
                printf("mcc:%d ",pos->cellinfo.CellInfo.gsm.cellIdentityGsm.mcc);
                printf("mnc:%d ",pos->cellinfo.CellInfo.gsm.cellIdentityGsm.mnc);
                printf("lac:%d ",pos->cellinfo.CellInfo.gsm.cellIdentityGsm.lac);
                printf("cid:%d ",pos->cellinfo.CellInfo.gsm.cellIdentityGsm.cid);
                printf("signalStrength:%d ",pos->cellinfo.CellInfo.gsm.signalStrengthGsm.signalStrength);
                printf("bitErrorRate:%d ",pos->cellinfo.CellInfo.gsm.signalStrengthGsm.bitErrorRate);
                printf("rscpdbm:%d ",pos->cellinfo.CellInfo.gsm.signalStrengthGsm.rscpdbm);
                printf("eciodb:%d\n",pos->cellinfo.CellInfo.gsm.signalStrengthGsm.eciodb);

            }
            break;
            case RIL_CELL_INFO_TYPE_CDMA:
            {
                printf("networkId:%d ",pos->cellinfo.CellInfo.cdma.cellIdentityCdma.networkId);
                printf("systemId:%d ",pos->cellinfo.CellInfo.cdma.cellIdentityCdma.systemId);
                printf("basestationId:%d ",pos->cellinfo.CellInfo.cdma.cellIdentityCdma.basestationId);
                printf("longitude:%d ",pos->cellinfo.CellInfo.cdma.cellIdentityCdma.longitude);
                printf("latitude:%d ",pos->cellinfo.CellInfo.cdma.cellIdentityCdma.latitude);
                printf("Cdma_dbm:%d ",pos->cellinfo.CellInfo.cdma.signalStrengthCdma.dbm);
                printf("Cdma_ecio:%d ",pos->cellinfo.CellInfo.cdma.signalStrengthCdma.ecio);
                printf("Evdo_dbm:%d ",pos->cellinfo.CellInfo.cdma.signalStrengthEvdo.dbm);
                printf("Evdo_ecio:%d ",pos->cellinfo.CellInfo.cdma.signalStrengthEvdo.ecio);
                printf("Evdo_signalNoiseRatio:%d\n",pos->cellinfo.CellInfo.cdma.signalStrengthEvdo.signalNoiseRatio);
            }
            break;
            case RIL_CELL_INFO_TYPE_LTE:
            {
                printf("mcc:%d ",pos->cellinfo.CellInfo.lte.cellIdentityLte.mcc);
                printf("mnc:%d ",pos->cellinfo.CellInfo.lte.cellIdentityLte.mnc);
                printf("ci:%d ",pos->cellinfo.CellInfo.lte.cellIdentityLte.ci) ;
                printf("pci:%d ",pos->cellinfo.CellInfo.lte.cellIdentityLte.pci);
                printf("tac:%d ",pos->cellinfo.CellInfo.lte.cellIdentityLte.tac) ;
                printf("signalStrength:%d ",pos->cellinfo.CellInfo.lte.signalStrengthLte.signalStrength) ;
                printf("rsrp:%d ",pos->cellinfo.CellInfo.lte.signalStrengthLte.rsrp);
                printf("rsrq:%d ",pos->cellinfo.CellInfo.lte.signalStrengthLte.rsrq) ;
                printf("rssnr:%d ",pos->cellinfo.CellInfo.lte.signalStrengthLte.rssnr);
                printf("cqi:%d ",pos->cellinfo.CellInfo.lte.signalStrengthLte.cqi);
                printf("timingAdvance:%d\n",pos->cellinfo.CellInfo.lte.signalStrengthLte.timingAdvance);
            }
            break;
            case RIL_CELL_INFO_TYPE_WCDMA:
            {
                printf("mcc:%d ",pos->cellinfo.CellInfo.wcdma.cellIdentityWcdma.mcc);
                printf("mnc:%d ",pos->cellinfo.CellInfo.wcdma.cellIdentityWcdma.mnc);
                printf("lac:%d ",pos->cellinfo.CellInfo.wcdma.cellIdentityWcdma.lac);
                printf("cid:%d ",pos->cellinfo.CellInfo.wcdma.cellIdentityWcdma.cid);
                printf("psc:%d ",pos->cellinfo.CellInfo.wcdma.cellIdentityWcdma.psc);
                printf("signalStrength:%d ",pos->cellinfo.CellInfo.wcdma.signalStrengthWcdma.signalStrength);
                printf("bitErrorRate:%d\n",pos->cellinfo.CellInfo.wcdma.signalStrengthWcdma.bitErrorRate);
            }
            break;
            case RIL_CELL_INFO_TYPE_TD_SCDMA:
            {
                printf("mcc:%d ",pos->cellinfo.CellInfo.tdscdma.cellIdentityTdscdma.mcc);
                printf("mnc:%d ",pos->cellinfo.CellInfo.tdscdma.cellIdentityTdscdma.mnc);
                printf("lac:%d ",pos->cellinfo.CellInfo.tdscdma.cellIdentityTdscdma.lac);
                printf("cid:%d ",pos->cellinfo.CellInfo.tdscdma.cellIdentityTdscdma.cid);
                printf("cpid:%d ",pos->cellinfo.CellInfo.tdscdma.cellIdentityTdscdma.cpid);
                printf("rscp:%d\n",pos->cellinfo.CellInfo.tdscdma.signalStrengthTdscdma.rscp);
            }
            break;
            default:
            {
                printf("error type");
            }
            break;
        }
        printf("====================\n");
        pos = pos->next;
    }
}

#if 0
static int cdma_convert_to_csq(int rssi)
{
    int csqval;
    if(rssi <= -125)
    {
        csqval = 0;
    }
    else if(rssi >= -75)
    {
        csqval = 31;
    }
    else
    {
        //csqval = floor((125.0 + rssi) * 31.0 / 50);
    }
    return csqval;
}

static int threegpp_convert_to_csq(int rssi)
{
    int csqval;
    if(rssi <= -113)
    {
        csqval = 0;
    }
    else if(rssi >= -51)
    {
        csqval = 31;
    }
    else
    {
        csqval = floor((182.26 + 100.0/62 * rssi) * 31.0 / 100 + 0.5);
    }    
    return csqval;
}


static int tds_convert_to_csq(int rssi)
{
    int csqval;
    if(rssi <= -116)
    {
        csqval = 0;
    }
    else if(rssi >= -25)
    {
        csqval = 31;
    }
    else
    {
       //csqval = floor((113.0 + 100.0/191 * rssi) * 31 / 100 + 0.5);
    }    
    return csqval;
}


static int get_csq_val(int radio_if, int rssi)
{
    int csqval = 0;
    if(radio_if ==  RADIO_IF_NO_SVC)
    {
        return csqval;
    }
    //printf("get_csq_val radio_if = %d, rssi = %d\n", radio_if, rssi);
    switch(radio_if)
    {
        case RADIO_IF_NO_SVC:
            csqval = 0;
            break;
        case RADIO_IF_CDMA_1X:
        case RADIO_IF_CDMA_1XEVDO:
            csqval = cdma_convert_to_csq(rssi);
            break;
        case RADIO_IF_GSM:
        case RADIO_IF_UMTS:
        case RADIO_IF_LTE:
        case RADIO_IF_NR5G:
            csqval = threegpp_convert_to_csq(rssi);
            break;
        case RADIO_IF_TDSCDMA:
            csqval = tds_convert_to_csq(rssi);
            break;
        default:
            break;
    }
   
    return csqval;  
}
#endif
static void simcom_nas_test()
{
    const char *options_list[] =
        {
            "1. get serving system",
            "2. get signal info",
            "3. get network preference mode",
            "4. set network preference mode",
            "6. get extended signal info",
            "7. get cfun mode",
            "8. set cfun mode",
            "9. get all cell info",
            "10. get ims registation state",
            "11. get register plmn",
            "12. search network",
            "13. select network",
            "99. back",
        };
    int ret = 0;
    int opt = 0;
    char scan_string[100] = {0};
    int index;
    int level, mode;
    int nw_mode;
    SIM_NetState registrationState;
    CellInfo_Node *head = NULL;

    while (1)
    {
        printf("\nPlease select an option to test from the items listed below.\n");
        print_option_menu(options_list, sizeof(options_list) / sizeof(options_list[0]));
        printf("Option > ");
        memset(scan_string, 0, sizeof(scan_string));
        if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
        {
            continue;
        }
        
        /* Convert the option to an integer, and switch on the option entered. */
        opt = atoi(scan_string);
        switch (opt)
        {
        case 1:
        {
            memset(&registrationState, 0x00, sizeof(SIM_NetState));
            get_NetworkType(&registrationState);
            app_nas_printf_serving_system(registrationState);
        }
        break;
        case 2:
        {
            SIM_sigInfo sigInfo = get_SignalStrength();
            if (sigInfo.lteSigValid)
            {
                printf("LTE rssi:%d", sigInfo.rssi);
            }

            if (sigInfo.gsmSigValid)
            {
                printf("GSM rssi:%d", sigInfo.rssi);
            }
        }
        break;
        case 3:
        {
            ret = get_NetworkPreferredMode(&nw_mode);
            printf("nw mode:%d\n",nw_mode);
        }
        break;
        case 4:
        {
            printf("nw_mode>");
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
            {
                continue;
            }
            nw_mode = atoi(scan_string);
            ret = set_NetworkPreferredMode(nw_mode);
            if(ret != 0)
            {
                printf("error!");
            }
        }   
        break;
        case 6:
        {
            SIM_ExtSigInfo siginfo;
            ret = get_ExtSignalInfo(&siginfo);
            if (siginfo.lte_siginfo.SigType == 1)
            {
                printf("LTE info:%d,%d,%d,%d", siginfo.lte_siginfo.rssi,siginfo.lte_siginfo.rsrp,siginfo.lte_siginfo.rsrq,siginfo.lte_siginfo.sinr);
            }

            if (siginfo.gsm_siginfo.SigType == 0)
            {
                printf("GSM info:%d,%d,%d", siginfo.gsm_siginfo.rssi,siginfo.gsm_siginfo.rscp,siginfo.gsm_siginfo.ecio);
            }
        }
        break;
        case 7:
        {
            ret = get_PhoneMode(&mode);
            if(ret == 0)
            {
                app_nas_printf_phonemode(mode);            
            }
            else
            {
                printf("error!");
            }
        }
        break;
        case 8:
        {
            printf("0:online\n1:low power\n2:factory online\n3:persistent low power\n4:offline\n5:reset(reboot after set)\n");
            printf("phone mode>");
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
            {
                continue;
            }
            mode = atoi(scan_string);
            ret = set_PhoneMode(mode);
            if(ret != 0)
            {
                printf("error!");
            }
        }
        break;
        case 9:
        {
            ret = get_CellInfo(&head);
            if(!ret)
            {
                app_nas_printf_cell_info(head);
            }
            free_cellinfo_list(head);
            head = NULL;
        }
        break;

        case 10:
        {
            int ims_registration_state;
            ret = get_ImsRegistrationState(&ims_registration_state);
            if(ret == EXIT_SUCCESS)
            {
                printf("ims registration state=%d\n", ims_registration_state);
            }
            else
            {
                printf("ims registration state=%d\n", -1);
            }
        }
        break;

        case 11:
        {
            char long_formal_alphanumeric[MAX_SIM_INFO_STR_LEN];
            char short_formal_alphanumeric[MAX_SIM_INFO_STR_LEN];
            char numeric[MAX_SIM_INFO_STR_LEN];
            ret = get_register_plmn( long_formal_alphanumeric, short_formal_alphanumeric, numeric);
            if (ret != 0)
            {
                printf("error!\n");
            }
            else
            {
                printf(" long_formal_alphanumeric:%s\n short_formal_alphanumeric:%s\n numeric_formal:%s\n", long_formal_alphanumeric, short_formal_alphanumeric, numeric);
            }
        }
        break;
        
        case 12:
        {
            Network_list *tmp =NULL;
            Network_list *network_tmp =NULL;
            Network_list *network_head =NULL;
            ret = search_network(&network_head);
            if(network_head == NULL)
            {
                printf("Failed to receive data!\n");
            }
            if (ret == 0)
            {
                if(network_head == NULL)
                {
                    printf("Failed to receive data!\n");
                }
                else
                {
                    tmp = network_head;
                    for (;;)
                    {
                        if (NULL != tmp)
                        {
                            printf("id:%d\n", tmp->id);
                            printf("isp_name:%s\n", tmp->isp_name);
                            printf("plmn:%s\n", tmp->plmn); 
                            printf("status:%s\n", tmp->status);
                            printf("act:%s\n\n", tmp->act);
                            tmp = tmp->next;
                        }
                        else
                        {
                            break;
                        }
                    }
                    tmp = network_head;
                    for (;;)
                    {
                        if (NULL != tmp)
                        {
                            network_tmp = tmp->next;
                            free(tmp);
                            tmp = network_tmp;
                        }
                        else
                        {
                            break;
                        }
                    }
                }
            }
            else
            {
                printf("error!\n");
            }
        }
        break;

        case 13:
        {
            char plmn[MAX_NETWORK_INFO_STR_LEN]={0};
            int act = -1;
            int i = 0;
            while (1)
            {
                printf("Please input the plmn which you want to select\n");
                printf("eg: 46000 (according to the result of search network!)\n");
                printf(">");
                memset(plmn, 0, sizeof(plmn));
                if (fgets(plmn, sizeof(plmn), stdin) == NULL)
                {
                    continue;
                }
                plmn[strlen(plmn) - 1] = 0;
                if (0 == strlen(plmn))
                {
                    continue;
                }
                break;
            }
            while (1)
            {
                printf("Please input the act which you want to select\n");
                printf("act and plmn must match according to the search network!\n");
                printf("0 means GSM, 1 means GSM_COMPACT, 2 means UTRAN, 3 means GSM_wEGPRS, 4 means UTRAN_wHSDPA\n");
                printf("5 means UTRAN_wHSUPA, 6 means UTRAN_wHSDPA_HSUPA,7 means E_UTRAN, 8 means UTRAN_HSPA+\n");
                printf(">");
                memset(scan_string, 0, sizeof(scan_string));
                if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                {
                    continue;
                }
                scan_string[strlen(scan_string) - 1] = 0;
                for (i = 0; i < strlen(scan_string); i++)
                {
                    if ((scan_string[i] >= '0') && (scan_string[i] <= '9'))
                    {
                        break;
                    }
                }
                if (i == strlen(scan_string))
                {
                    continue;
                }
                act = atoi(scan_string);
                break;
            }
            ret = select_network(plmn, act);
            if (ret == 0)
            {
                printf("Select succeed!\n");
            }
            else
            {
                printf("error!\n");
            }
        }
        break;
        
        default:
            break;
        }

        if (opt == 99)
        {
            break;
        }
    }
}
#endif

#ifdef FEATURE_DEMO_WIFI
static boolean select_ap_index(ap_index_type *ap_index)
{
    char ap_index_str[64] = {0};
    wifi_mode_type mode = get_wifi_mode();
    int ap_index_val;
    if (mode == APAP_MODE)
    {
        printf("Please input the ap_index: ");
        fgets(ap_index_str, sizeof(ap_index_str), stdin);

        //ap_index_val == atoi(ap_index_str);
        if (strlen(ap_index_str) != 2)
        {
            return FALSE;
        }

        switch (ap_index_str[0])
        {
        case '0':
            ap_index_val = AP_INDEX_0;
            break;

        case '1':
            ap_index_val = AP_INDEX_1;
            break;

        default:
            ap_index_val = 2;
            break;
        }

        if (ap_index_val != AP_INDEX_0 && ap_index_val != AP_INDEX_1)
        {
            printf("ap_index input wrong!\r\n");
            return FALSE;
        }
        else
        {
            *ap_index = (ap_index_type)ap_index_val;
        }
    }
    else if (mode == APSTA_MODE)
    {
        *ap_index = AP_INDEX_STA;
    }
    else if (mode == AP_MODE)
    {
        *ap_index = AP_INDEX_0;
    }
    else
    {
        printf("ap mode is invalid!\r\n");
        return FALSE;
    }

    return TRUE;
}

static int is_ascii_key_string(char *str, int len)
{
    int i;
    char c;
    int ret = TRUE;
    for (i = 0; i < len; i++)
    {
        c = *(str + i);
        if (c < 0x20 || c > 0x7F)
        {
            ret = FALSE;
        }
    }

    return ret;
}

static int simcom_check_wep_password(char *pwstr)
{
    int ret = 0;
    int len = 0;
    int i = 0;

    if (pwstr == NULL)
    {
        return -1;
    }

    len = strlen(pwstr);
    if (len < 5 || len > 26)
    {
        return -1;
    }

    if (len == 5 || len == 13)
    {
        if (!is_ascii_key_string(pwstr, len))
        {
            return -1;
        }
    }
    else if (len == 10 || len == 23)
    {
        for (i = 0; i < len; i++)
        {
            if (!isxdigit(pwstr[i]))
                ;
            {
                return -1;
            }
        }
    }
    return 0;
}
static int simcom_check_wpa_password(char *pwstr)
{
    int ret = 0;
    int len = 0;
    int i = 0;

    if (pwstr == NULL)
    {
        return -1;
    }

    len = strlen(pwstr);
    if (len < 8 || len > 64)
    {
        return -1;
    }

    if (len == 64)
    {
        for (i = 0; i < len; i++)
        {
            if (!isxdigit(pwstr[i]))
                ;
            {
                return -1;
            }
        }
    }
    else
    {
        if (!is_ascii_key_string(pwstr, len))
        {
            return -1;
        }
    }
    return 0;
}

static void simcom_wifi_test()
{
    char scan_string[130] = {0};
    int opt = 0;
    boolean ret = FALSE;
    int i = 0;
    int j = 0;
    /* Arry for wifi_daemon options, Should always have and even number of elements */
    const char *options_list[] = {
        "1. SET SSID",
        "2. GET SSID",
        "3. SET AUTH",
        "4. GET AUTH",
        "5. SET BCAST",
        "6. GET BCAST",
        "7. SET ISOLATE",
        "8. GET ISOLATE",
#ifdef FEATURE_WIFI_CLIENT /*SIMCOM zhangwei @20210725 add for wifi support*/
        "9. GET DHCP",
        "10. GET CLIENT_COUNT",
        "11. OPEN/CLOSE WIFI",
        "12. GET WIFI_STATUS",
        "13. GET NET_STATUS",
        "14. SET USER_INFO",
        "15. GET USER_INFO",
        "16. GET MAC_ADDR",
        "17. GET AP_CFG(for W58)",
        "18. SET AP_CFG(for W58)",
        "19. GET STA_IP",
        "20. SET STA_CFG",
        "21. GET STA_CFG",
        "22. STA SCAN",
        "23. SET_SSID_AND_AUTH",
        "24. OPEN/CLOSE STA MODE(for W58L)",
        "25. GET STA_STATE(for W58L)",
        "26. CONNECT/DISCONNECT THE NET",
        "27. GET THE NET STATUS",
#endif /*SIMCOM zhangwei @20210725 add for wifi support*/
        "28. RESTORE",
        "29. GET OPERATION RESULT CODE",
        "30. SET CHANNEL",
        "31. GET CHANNEL",
        "32. SET NETMODE",
        "33. GET NETMODE",
        "34. SET BAND",
        "35. GET BAND",
        "36. GET ALL_CLIENT_INFO",
        "37. GET CONN_CLIENTS_INFO",
        "38. CONNECT_NUM_CONFIG",
        "39. ENABLE WPS",
        "40. GET WPS_STATE",
        "41. CALL WPS_PBC",
        "42. CALL WPS_PIN",
        "43. CALL WPS_CANCEL",
        "44. SET WIFI MAC filter Mode",
        "45. GET WIFI MAC filter INFO",
        "46. ADD MAC Address of allow",
        "47. ADD MAC Address of deny",
        "48. Delete MAC Address of allow",
        "49. Delete MAC Address of deny",
        "50. GET WPS_PBC_STATE",
        "51. GET WIFI_MODE",
        "52. STA_SCAN",
        "53. STA_CONNECT",
        "54. SET WIFI_MODE",
        "55. GET STA_STATE",
        "56. GET STA_CFG",
        "57. GET STA_IP",
        "58. STA_DISABLE_NETWORK",
        "59. STA_ENABLE_NETWORK",
        "60. CHANGE WIFI FREQ",
        "99. BACK",
    };

    boolean be_exit = FALSE;

    while (TRUE)
    {
        /* Display menu of options. */
        printf("\n@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
        printf("\nPlease select an option to test from the items listed below.\n");
        print_option_menu(options_list, sizeof(options_list) / sizeof(options_list[0]));
        printf("Option > ");

        /* Read the option from the standard input. */
        memset(scan_string, 0, sizeof(scan_string));
        if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
            continue;

        /* Convert the option to an integer, and switch on the option entered. */
        opt = atoi(scan_string);
        switch (opt)
        {
        case 1:
        {
            char ssid[65] = {0};
            ap_index_type ap_index;
            wifi_mode_type mode;
            wifi_freq freq;

            if (simcom_if_support_5G() == 1)
            {
                printf(" Please input the freq 0~2.4G  1~5G: ");
                memset(scan_string, 0, sizeof(scan_string));
                if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                    continue;
                freq = atoi(scan_string);

                printf(" Please input the SSID: ");
                memset(scan_string, 0, sizeof(scan_string));
                if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                    continue;
                memcpy(ssid, scan_string, strlen(scan_string) - 1);
                ap_index  = AP_INDEX_0;
            }
            else
            {
                if (!select_ap_index(&ap_index))
                {
                    break;
                }
                printf(" Please input the SSID: ");
                memset(scan_string, 0, sizeof(scan_string));
                if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                    continue;
                memcpy(ssid, scan_string, strlen(scan_string) - 1);
                char ssid_cmp[65] = {0};
                if (strcmp(ssid,ssid_cmp) == 0)
                {
                    printf("\nset SSID failed!\n");
                    break;
                }
                freq = FREQ_2_4G;
            }

            ret = set_ssid_plus(ssid, ap_index, freq);
            if (ret)
            {
                printf("\nset SSID success!\n");
            }
            else
            {
                printf("\nset SSID failed!\n");
            }
        }
        break;

        case 2:
        {
            char ssid[69] = {0};
            ap_index_type ap_index;
            wifi_freq freq;

            if (simcom_if_support_5G() == 1)
            {
                printf("   Please input the freq 0~2.4G  1~5G: ");
                memset(scan_string, 0, sizeof(scan_string));
                if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                    continue;
                freq = atoi(scan_string);
                ap_index  = AP_INDEX_0;
            }
            else
            {
                if (!select_ap_index(&ap_index))
                {
                    break;
                }
                freq = FREQ_2_4G;
            }

            ret = get_ssid_plus(ssid, ap_index, freq);
            if (ret)
            {
                printf("\nSSID: %s\n", ssid);
            }
            else
            {
                printf("\nget SSID failed!\n");
            }
        }
        break;

        case 3:
        {
            /*SIMCOM zhangwei @20210725 add for wifi support begin*/
            int auth_type = 0;
            char str_encrypt_mode[8] = {0};
            int encrypt_mode = 0;
            char pwd_str[65] = {0};
            ap_index_type ap_index;
            wifi_freq freq;

            if (simcom_if_support_5G() == 1)
            {
                printf(" Please input the freq 0~2.4G  1~5G: ");
                memset(scan_string, 0, sizeof(scan_string));
                if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                    continue;
                freq = atoi(scan_string);
                ap_index  = AP_INDEX_0;
            }
            else
            {
                if (!select_ap_index(&ap_index))
                {
                    break;
                }
                freq = FREQ_2_4G;
            }

            printf(" Please input the auth type(%d-%d): ", AUTH_MIN, AUTH_MAX);
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                continue;
            auth_type = atoi(scan_string);
            if (auth_type < AUTH_MIN || auth_type > AUTH_MAX)
            {
                printf(" Input the auth type invalid\n");
                break;
            }

            if (auth_type != 0)
            {
                if (auth_type == AUTH_WPA) // wpa
                {
                    printf("   Please input the encrypt mode(%d-%d): ", E_TKIP, E_AES_TKIP);
                }
                else if (auth_type == AUTH_WEP)
                {
                    printf("   Please input the encrypt mode(%d,%d): ", E_WEP_SHARED, E_WEP_OPEN);
                }
                else
                {
                    printf("   Please input the encrypt mode(%d,%d): ", E_AES, E_AES_TKIP);
                }
                memset(scan_string, 0, sizeof(scan_string));
                if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                    continue;

                simcom_handle_input_string(scan_string, strlen(scan_string), str_encrypt_mode, 7, 0);

                encrypt_mode = atoi(str_encrypt_mode);

                //check  encrypt_mode value
                if (auth_type == AUTH_WPA) // wpa
                {
                    if ((encrypt_mode < E_TKIP) || (encrypt_mode > E_AES_TKIP))
                    {
                        printf("   Input the encrypt mode failed!\n");
                        break;
                    }
                }
                else if (auth_type == AUTH_WEP)
                {
                    if ((encrypt_mode != E_WEP_SHARED) && (encrypt_mode != E_WEP_OPEN))
                    {
                        printf("   Input the encrypt mode failed!\n");
                        break;
                    }
                }
                else if (auth_type == AUTH_WPAWPA2 || auth_type == AUTH_WPA2)
                {
                    if ((encrypt_mode != E_AES) && (encrypt_mode != E_AES_TKIP))
                    {
                        printf("   Input the encrypt mode failed!\n");
                        break;
                    }
                }

                /* share mode need set wep key */
                if (auth_type == AUTH_WEP)
                {
                    printf("   Please input the password(ASC 5 or 13/hex 10 or 26): ");
                    memset(scan_string, 0, sizeof(scan_string));
                    if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                        continue;
                    memcpy(pwd_str, scan_string, strlen(scan_string) - 1);
                }
                else
                {
                    printf("   Please input the password(ASC 8-63) ");
                    memset(scan_string, 0, sizeof(scan_string));
                    if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                        continue;
                    memcpy(pwd_str, scan_string, strlen(scan_string) - 1);
                }
            }

            ret = set_auth_plus(pwd_str, auth_type, encrypt_mode, ap_index,freq);
            if (ret)
            {
                printf("\nset AUTH success!\n");
            }
            else
            {
                printf("\nset AUTH failed!\n");
            }
        }
        /*SIMCOM zhangwei @20210725 add for wifi support end*/
        break;

        case 4:
        {
            int auth_type = 0;
            int encrypt_mode = 0;
            char pwd_str[65] = {0};
            ap_index_type ap_index;
            wifi_freq freq;

            if (simcom_if_support_5G() == 1)
            {
                printf(" Please input the freq 0~2.4G  1~5G: ");
                memset(scan_string, 0, sizeof(scan_string));
                if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                    continue;
                freq = atoi(scan_string);
                ap_index  = AP_INDEX_0;
            }
            else
            {
                if (!select_ap_index(&ap_index))
                {
                    break;
                }
                freq = FREQ_2_4G;
            }
            ret = get_auth_plus(&auth_type, &encrypt_mode, pwd_str, ap_index,freq);
            if (ret)
            {
                printf("\nAUTH: %d,%d,\"%s\"\n", auth_type, encrypt_mode, pwd_str);
            }
            else
            {
                printf("\nget AUTH failed!\n");
            }
        }
        break;

        case 5:
        {
            int broadcast = 0;
            ap_index_type ap_index;
            wifi_freq freq;

            if (simcom_if_support_5G() == 1)
            {
                printf(" Please input the freq 0~2.4G  1~5G: ");
                memset(scan_string, 0, sizeof(scan_string));
                if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                    continue;
                freq = atoi(scan_string);
                ap_index  = AP_INDEX_0;
            }
            else
            {
                if (!select_ap_index(&ap_index))
                {
                    break;
                }
                freq = FREQ_2_4G;
            }

            printf(" Please input the broadcast(0,1): ");
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                continue;

            broadcast = atoi(scan_string);

            ret = set_bcast_plus(broadcast, ap_index,freq);
            if (ret)
            {
                printf("\nset set_bcast success!\n");
            }
            else
            {
                printf("\nset set_bcast failed!\n");
            }
        }
        break;

        case 6:
        {
            int broadcast = 0;
            ap_index_type ap_index;
            wifi_freq freq;

            if (simcom_if_support_5G() == 1)
            {
                printf(" Please input the freq 0~2.4G  1~5G: ");
                memset(scan_string, 0, sizeof(scan_string));
                if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                    continue;
                freq = atoi(scan_string);
                ap_index  = AP_INDEX_0;
            }
            else
            {
                if (!select_ap_index(&ap_index))
                {
                    break;
                }
                freq = FREQ_2_4G;
            }

            ret = get_bcast_plus(&broadcast, ap_index,freq);
            if (ret)
            {
                printf("\nbroadcast: %d\n", broadcast);
            }
            else
            {
                printf("\nget broadcast failed!\n");
            }
        }
        break;
#if 0
            case 7:
                {
                    int channel = 0;
                    int mode = 0;
                    ap_index_type ap_index;
                    break;
                    if(!select_ap_index(&ap_index))
                    {
                        break;
                    }                    
                    printf("   Please input the channel: ");
                    memset(scan_string, 0, sizeof(scan_string));
                    if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                        continue;
                    channel = atoi(scan_string);

                    
                    printf("   Please input the mode: ");
                    memset(scan_string, 0, sizeof(scan_string));
                    if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                        continue;
                    mode = atoi(scan_string);

                    ret = set_channel_mode(channel, mode, ap_index);
                    if(ret)
                    {
                        printf("\nset set_channel_mode success!\n");
                    }
                    else
                    {
                        printf("\nset set_channel_mode failed!\n");
                    }
                }
                break;

            case 8:
                {
                    char channel_str[8] = {0};
                    int mode = 0;
                    ap_index_type ap_index;
                    break;
                    if(!select_ap_index(&ap_index))
                    {
                        break;
                    }

                    ret = get_channel_mode(channel_str, &mode, ap_index);
                    if(ret)
                    {
                        printf("\n channel_mode: %s,%d\n", channel_str, mode);
                    }
                    else
                    {
                        printf("\n get_channel_mode failed!\n");
                    }
                }
                break;
#endif
        case 7:
        {
            int isolate = 0;
            ap_index_type ap_index;
            wifi_freq freq;

            if (simcom_if_support_5G() == 1)
            {
                printf(" Please input the freq 0~2.4G  1~5G: ");
                memset(scan_string, 0, sizeof(scan_string));
                if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                    continue;
                freq = atoi(scan_string);
                ap_index  = AP_INDEX_0;
            }
            else
            {
                if (!select_ap_index(&ap_index))
                {
                    break;
                }
                freq = FREQ_2_4G;
            }

            printf(" Please input the isolate(0,1): ");
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                continue;

            isolate = atoi(scan_string);

            ret = set_isolate_plus(isolate, ap_index,freq);
            if (ret)
            {
                printf("\nset set_isolate success!\n");
            }
            else
            {
                printf("\nset set_isolate failed!\n");
            }
        }
        break;
        case 8:
        {
            int isolate = 0;
            ap_index_type ap_index;
            wifi_freq freq;

            if (simcom_if_support_5G() == 1)
            {
                printf(" Please input the freq 0~2.4G  1~5G: ");
                memset(scan_string, 0, sizeof(scan_string));
                if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                    continue;
                freq = atoi(scan_string);
                ap_index  = AP_INDEX_0;
            }
            else
            {
                if (!select_ap_index(&ap_index))
                {
                    break;
                }
                freq = FREQ_2_4G;
            }

            ret = get_isolate_plus(&isolate, ap_index,freq);
            if (ret)
            {
                printf("\nisolate: %d\n", isolate);
            }
            else
            {
                printf("\nget broadcast failed!\n");
            }
        }
        break;

#ifdef FEATURE_WIFI_CLIENT /*SIMCOM zhangwei @20210725 add for wifi support*/

        case 9:
        {
            char host_ip_str[16] = {0};
            char start_ip_str[16] = {0};
            char end_ip_str[16] = {0};
            char time_str[16] = {0};

            ret = get_dhcp(host_ip_str, start_ip_str, end_ip_str, time_str);
            if (ret)
            {
                printf("\n DHCP: \"%s\",\"%s\",\"%s\",%s\n", host_ip_str, start_ip_str, end_ip_str, time_str);
            }
            else
            {
                printf("\n get_dhcp failed!\n");
            }
        }
        break;

        case 10:
        {
            ap_index_type ap_index;
            int count1, count2;
            wifi_mode_type mode = get_wifi_mode();
            if (mode == AP_MODE)
            {
                count1 = get_client_count(AP_INDEX_0);
                printf("\n count: %d\n", count1);
            }
            else if (mode == APSTA_MODE)
            {
                count1 = get_client_count(AP_INDEX_STA);
                printf("\n count: %d\n", count1);
            }
            else
            {
                count1 = get_client_count(AP_INDEX_0);
                count2 = get_client_count(AP_INDEX_1);
                printf("\n AP1_count: %d  AP2_count: %d\n", count1, count2);
            }
        }
        break;

        case 11:
        {
            int flag = 0;
            printf("   Please input the operation( 0-Disable AP; 1-Enable AP): ");
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                continue;
            flag = atoi(scan_string);
            ret = wifi_power(flag);
            if (ret)
            {
                printf("\n set_map success!\n");
            }
            else
            {
                printf("\n set_map failed!\n");
            }
        }
        break;

        case 12:
        {
            uint8 flag = 0;

            ret = get_wifi_status(&flag);
            if (ret)
            {
                printf("\n wifi status: %d\n", flag);
            }
            else
            {
                printf("\n wifi status: 0\n");
            }
        }
        break;

        case 13:
        {
            char net_enable_str[sizeof(int) + 1] = {0};
            int save_flag = 0;
            ret = get_net_status(net_enable_str, &save_flag);
            if (ret)
            {
                printf("\n NET_STATUS: %s,%d\n", net_enable_str, save_flag);
            }
            else
            {
                printf("\n get_net_status failed!\n");
            }
        }
        break;

        case 14:
        {
            char sz_usrname[128] = {0};
            char sz_usrpwd[128] = {0};
            char ch;
            int i = 0, j = 0;
            int nret = 0;

            printf("   Please input the user name(length 1-127): ");

            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                continue;

            nret = simcom_handle_input_string(scan_string, strlen(scan_string), sz_usrname, 127, 1);
            if (nret < 0)
            {
                printf("\n user name is too short!\n");
                break;
            }
            else if (nret == 1)
            {
                printf("\n user name is too long!\n");
                break;
            }
            else if (nret == 2)
            {
                printf("\n user name is too long!\n");
                memset(scan_string, 0, sizeof(scan_string));
                fgets(scan_string, sizeof(scan_string), stdin);
                break;
            }

            printf("   Please input the password(length 1-127): ");
            memset(scan_string, 0, sizeof(scan_string));

            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                continue;

            nret = simcom_handle_input_string(scan_string, strlen(scan_string), sz_usrpwd, 127, 1);
            if (nret < 0)
            {
                printf("\n user pwd is too short!\n");
                break;
            }
            else if (nret == 1)
            {
                printf("\n user pwd is too long!\n");
                break;
            }
            else if (nret == 2)
            {
                printf("\n user pwd is too long!\n");
                memset(scan_string, 0, sizeof(scan_string));
                fgets(scan_string, sizeof(scan_string), stdin);
                break;
            }

            ret = set_user_name_pwd(sz_usrname, sz_usrpwd);
            if (ret)
            {
                printf("\n set_user_name_pwd success!\n");
            }
            else
            {
                printf("\n set_user_name_pwd failed!\n");
            }
        }
        break;

        case 15:
        {
            char sz_usrname[128] = {0};
            char sz_usrpwd[128] = {0};

            ret = get_user_name_pwd(sz_usrname, sizeof(sz_usrname), sz_usrpwd, sizeof(sz_usrpwd));
            if (ret)
            {
                printf("\n USER NAME:%s    PASSWORD:%s\n", sz_usrname, sz_usrpwd);
            }
            else
            {
                printf("\n get_user_name_pwd failed!\n");
            }
        }
        break;

        case 16:
        {
            char mac_addr[1024] = {0};
            ap_index_type ap_index;
            if (!select_ap_index(&ap_index))
            {
                break;
            }

            ret = get_mac_addr(mac_addr, ap_index);
            if (ret)
            {
                printf("\n MAC_ADDR: %s\n", mac_addr);
            }
            else
            {
                printf("\n get_mac_addr failed!\n");
            }
        }
        break;

        case 17:
        {
            wifi_mode_type mode = get_wifi_mode();
            printf("\n wifi mode: %d\n", mode);
        }
        break;

        case 18:
        {
            int i = 0;

            int mode;

            printf("   Please input the wifi mode (0: AP  1:AP-AP  2:AP-STA) >");
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                continue;

            mode = atoi(scan_string);
            if (mode < 0 || mode > 2)
            {
                printf("input mode wrong");
                break;
            }

            ret = set_wifi_mode((wifi_mode_type)mode);
            if (ret)
            {
                printf("\n set_wifi_mode success!\n");
            }
            else
            {
                printf("\n set_wifi_mode failed!\n");
            }
        }
        break;

        case 19:
        {
            char ip_addr[30] = {0};
            int status = 0;

            if (!wifi_is_sta_mode())
            {
                printf("mode is not AP-STA\r\n");
                break;
            }

            ret = get_sta_ip(ip_addr, sizeof(ip_addr));
            if (ret)
            {
                printf("\n STA_IP: %s\n", ip_addr);
            }
            else
            {
                printf("\n get_sta_ip failed!\n");
            }
        }
        break;

        case 20:
        {
            char ssid[65] = {0};
            char psk_value[65] = {0};
            int status = 0;
            boolean is_chines = FALSE;
            int i = 0;
            if (!wifi_is_sta_mode())
            {
                printf("mode is not AP-STA\r\n");
                break;
            }
            printf("   Please input the ssid string(1-32): ");
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                continue;

            if (strlen(scan_string) > 32 + 1)
            {
                printf("\n ssid is too long!\n");
                break;
            }

            //ssid[0] = '\"';
            strncpy(ssid, scan_string, strlen(scan_string) - 1);
            //ssid[strlen(ssid)] = '\"';
            printf("\n ssid[%s]\n", ssid);

            printf("   Please input the password of external wireless network(8-64): ");
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                continue;

            if (strlen(scan_string) > 64 + 1)
            {
                printf("\n password is too long!\n");
                break;
            }
            while (i < strlen(scan_string))
            {
                if (scan_string[i] >= 0xb0 && scan_string[i] <= 0xf7 && scan_string[i + 1] >= 0xa1 && scan_string[i + 1] <= 0xfe)
                {
                    is_chines = true;
                    i += 2;
                }
                else
                {
                    i++;
                }
            }
            if (is_chines)
            {
                printf("input Format err!\n");
                break;
            }

            //psk_value[0] = '\"';
            strncpy(psk_value, scan_string, strlen(scan_string) - 1);
            //psk_value[strlen(psk_value)] = '\"';
            //printf("\n psk_value[%s]\n", psk_value);

            ret = set_sta_cfg(ssid, psk_value);
            if (ret)
            {
                printf("\n set_sta_cfg success!\n");
            }
            else
            {
                printf("\n set_sta_cfg failed!\n");
            }
        }
        break;

        case 21:
        {
            char ssid_str[69] = {0};
            char psk_value[69] = {0};
            int status = 0;

            if (!wifi_is_sta_mode())
            {
                printf("mode is not AP-STA\r\n");
                break;
            }
            ret = get_sta_cfg(ssid_str, psk_value);
            if (ret)
            {
                printf("\n STA_CFG: %s, %s\n", ssid_str, psk_value);
            }
            else
            {
                printf("\n get_sta_cfg failed!\n");
            }
        }
        break;

        case 22:
        {
            char list_str[300] = {0};
            int status = 0;

            if (!wifi_is_sta_mode())
            {
                printf("mode is not AP-STA\r\n");
                break;
            }

            ret = sta_scan(list_str);
            if (ret)
            {
                printf("\n STA_SCAN LIST: %s\n", list_str);
            }
            else
            {
                printf("\n sta_scan failed!\n");
            }
        }
        break;

        case 23:
        {
            char ssid[66] = {0};
            int auth_type = 0;
            int encrypt_mode = 0;
            char pwd_str[128] = {0};
            int nret;
            ap_index_type ap_index;
            wifi_mode_type mode;
            printf("   Please input the SSID: ");
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                continue;

            memcpy(ssid, scan_string, strlen(scan_string) - 1);

            mode = get_wifi_mode();
            if (mode == APAP_MODE)
            {
                printf("   Please input the index: ");
                memset(scan_string, 0, sizeof(scan_string));
                if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                    continue;
                //ap_index == atoi(scan_string);
                if (strlen(scan_string) != 2)
                {
                    break;
                }

                switch (scan_string[0])
                {
                case '0':
                    ap_index = AP_INDEX_0;
                    break;

                case '1':
                    ap_index = AP_INDEX_1;
                    break;

                default:
                    ap_index = AP_INDEX_STA;
                    break;
                }

                if (ap_index != AP_INDEX_0 && ap_index != AP_INDEX_1)
                {
                    // error index
                    break;
                }
            }
            else if (mode == AP_MODE)
            {
                ap_index = AP_INDEX_0;
            }
            else
            {
                ap_index = AP_INDEX_STA;
            }

            if (!select_ap_index(&ap_index))
            {
                break;
            }
            printf("Please input the auth type(%d-%d): ", AUTH_MIN, AUTH_MAX);
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                continue;
            auth_type = atoi(scan_string);

            if (auth_type > AUTH_MAX || auth_type < AUTH_MIN)
            {
                printf(" auth type wrong\n");
                continue;
            }
            if (auth_type >= AUTH_WPA) // wpa
            {
                printf("   Please input the encrypt mode(%d-%d): ", E_TKIP, E_AES_TKIP);
            }
            else
            {
                printf("   Please input the encrypt mode(%d-%d): ", E_NULL, E_WEP);
            }
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                continue;
            encrypt_mode = atoi(scan_string);
            if (auth_type <= AUTH_OPEN)
            {
                if (encrypt_mode < E_NULL || encrypt_mode > E_WEP)
                {
                    printf("encrypt mode wrong");
                    continue;
                }
            }
            else if (auth_type == AUTH_SHARE)
            {
                if (encrypt_mode != E_WEP)
                {
                    printf("encrypt mode wrong");
                    continue;
                }
            }
            else if (auth_type >= AUTH_WPA)
            {
                if (encrypt_mode < E_TKIP || encrypt_mode > E_AES_TKIP)
                {
                    printf("encrypt mode wrong");
                    continue;
                }
            }

            if (encrypt_mode == E_WEP)
            {
                printf("   Please input the wep password\n ");
                printf("format:\n");
                printf(" 5  ASCII character      example: sim12 \n ");
                printf(" 10 hexadecimal number   example: 12345abcdf \n ");
                printf(" 13 ASCII character      example: simcom12345678 \n ");
                printf(" 26 hexadecimal number   example: abcdef1234567abcdef1234567 \n ");
                printf("\ninput:");
                memset(scan_string, 0, sizeof(scan_string));
                if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                    continue;
                if (strlen(scan_string) >= sizeof(pwd_str) - 1)
                {
                    printf("wep format wrong!\n");
                    continue;
                }
                memcpy(pwd_str, scan_string, strlen(scan_string) - 1);
                nret = simcom_check_wep_password(pwd_str);
                if (nret < 0)
                {
                    printf("wep format wrong!\n");
                    continue;
                }
            }
            else if (encrypt_mode >= E_TKIP)
            {
                printf("   Please input the wpa password\n ");
                printf("format:\n");
                printf(" 8~63  ASCII character   example: simcom12 /simcom123456***** \n ");
                printf(" 64 bits    example: 12345abcdf \n ");

                printf("\ninput:");
                memset(scan_string, 0, sizeof(scan_string));
                if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                    continue;
                if (strlen(scan_string) >= sizeof(pwd_str) - 1)
                {
                    printf("wpa format wrong!\n");
                    continue;
                }
                memcpy(pwd_str, scan_string, strlen(scan_string) - 1);

                nret = simcom_check_wpa_password(pwd_str);
                if (nret < 0)
                {
                    printf("wpa format wrong!\n");
                    continue;
                }
            }

            ret = set_ssid_and_auth(ssid, pwd_str, auth_type, encrypt_mode, ap_index);
            if (ret)
            {
                printf("\nset SSID_AND_AUTH success!\n");
            }
            else
            {
                printf("\nset SSID_AND_AUTH failed!\n");
            }
        }
        break;

        case 24:
        {
            int op;
            printf("   Please input the operation( 0-Close STA mode; 1-Open STA mode): >");
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                continue;

            op = atoi(scan_string);
            if (op < 0 || op > 1)
            {
                printf("input wrong operation!\n");
                break;
            }

            ret = sta_init(op);
            if (ret)
            {
                printf("\n STA operation success!\n");
            }
            else
            {
                printf("\n STA operation failed!\n");
            }
        }
        break;

        case 25:
        {
            uint8 status = 0;
            ret = get_sta_status(&status);
            if (ret)
            {
                printf("\n STA_STATE: %d\n", status);
            }
            else
            {
                printf("\n get_sta_status failed!\n");
            }
        }
        break;

        case 26:
        {
            int act = -1;
            int save_flag;

            printf("   Please input the operation (0: DISCONNECT  1:CONNECT) >");
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                continue;

            if (scan_string[0] == '0')
            {
                act = 0;
            }
            else if (scan_string[0] == '1')
            {
                act = 1;
            }
            //act = atoi(scan_string);
            printf("\nact=[%d]\n", act);
            if (act < 0 || act > 1)
            {
                printf("input operation wrong");
                break;
            }

            printf("   Please input the save_flag (0: not save  1:save) >");
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                continue;

            save_flag = atoi(scan_string);
            if (save_flag < 0 || save_flag > 1)
            {
                printf("input save_flag wrong");
                break;
            }

            ret = wifi_net_cnct(act, save_flag);
            if (ret)
            {
                printf("\n wifi_net_cnct success!\n");
            }
            else
            {
                printf("\n wifi_net_cnct failed!\n");
            }
        }
        break;

        case 27:
        {
            int status = 0;
            ret = wifi_get_net_status(&status);
            if (ret)
            {
                printf("\n NET_STATE: %d\n", status);
            }
            else
            {
                printf("\n wifi_get_net_status failed!\n");
            }
        }
        break;
#endif /*SIMCOM zhangwei @20210725 add for wifi support*/
        case 28:
        {
            printf("\nPlease wait to restore...\n");
            restore_wifi();
            printf("\nshut down...\n");
        }
        break;

        case 29:
        {
            uint8 ret_code = wifi_get_err_code();
            printf("\n RESULT CODE:%d\n", ret_code);
        }
        break;

        case 30:
        {
            int channel;
            ap_index_type ap_index;
            wifi_freq freq;
            if (simcom_if_support_5G() == 1)
            {
                printf(" Please input the freq 0~2.4G  1~5G: ");
                memset(scan_string, 0, sizeof(scan_string));
                if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                    continue;
                freq = atoi(scan_string);
                ap_index  = AP_INDEX_0;
            }
            else
            {
                if (!select_ap_index(&ap_index))
                {
                    break;
                }
                freq = FREQ_2_4G;
            }

            if (freq == FREQ_2_4G)
            {
                printf("Please input the channel(0~14) (12~14 Associated with the country code): ");
            }
            else if(freq == FREQ_5G)
            {
                printf("Please input the channel(36 40 44 48 52 56 60 64 149 153 157 161) : ");
            }

            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                continue;
            channel = atoi(scan_string);
            ret = set_channel_plus(channel, ap_index,freq);
            if (ret)
            {
                printf("\nset set_channel success!\n");
            }
            else
            {
                printf("\nset set_channel failed!\n");
            }
        }
        break;

        case 31:
        {
            int  channel=0;
            ap_index_type ap_index;
            wifi_freq freq;

            if (simcom_if_support_5G() == 1)
            {
                printf(" Please input the freq 0~2.4G  1~5G: ");
                memset(scan_string, 0, sizeof(scan_string));
                if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                    continue;
                freq = atoi(scan_string);
                ap_index  = AP_INDEX_0;
            }
            else
            {
                if (!select_ap_index(&ap_index))
                {
                    break;
                }
                freq = FREQ_2_4G;
            }

            ret = get_channel_plus(&channel,ap_index,freq);
            if (ret)
            {
                printf("\n channel: %d\n", channel);
            }
            else
            {
                printf("\n get_channel failed!\n");
            }
        }
        break;

        case 32:
        {
            int netmode;
            ap_index_type ap_index;
            wifi_freq freq;

            if (simcom_if_support_5G() == 1)
            {
                printf(" Please input the freq 0~2.4G  1~5G: ");
                memset(scan_string, 0, sizeof(scan_string));
                if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                    continue;
                freq = atoi(scan_string);
                ap_index  = AP_INDEX_0;
            }
            else
            {
                if (!select_ap_index(&ap_index))
                {
                    break;
                }
                freq = FREQ_2_4G;
            }

            if (freq == FREQ_2_4G)
            {
                printf(" Please input the netmode(E_11B---1  E_11G---2  E_11BG---3  E_11N---4): ");
            }
            else if(freq == FREQ_5G)
            {
                //E_11AC-- 7 not supported now
                printf(" The E_11AC---7 mode is not supported !\n");
                printf(" Please input the netmode(E_11A---5  E_11NA---6 ): ");

            }
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                continue;
            netmode = atoi(scan_string);

            ret = set_netmode_plus(netmode, ap_index,freq);
            if (ret)
            {
                printf("\nset set_netmode success!\n");
            }
            else
            {
                printf("\nset set_netmode failed!\n");
            }
        }
        break;

        case 33:
        {
            int  netmode=0;
            ap_index_type ap_index;
            wifi_freq freq;

            if (simcom_if_support_5G() == 1)
            {
                printf(" Please input the freq 0~2.4G  1~5G: ");
                memset(scan_string, 0, sizeof(scan_string));
                if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                    continue;
                freq = atoi(scan_string);
                ap_index  = AP_INDEX_0;
            }
            else
            {
                if (!select_ap_index(&ap_index))
                {
                    break;
                }
                freq = FREQ_2_4G;
            }

            ret = get_netmode_plus(&netmode,ap_index,freq);
            if (ret)
            {
                printf("\n netmode: %d\n", netmode);
            }
            else
            {
                printf("\n netmode failed!\n");
            }
        }
        break;

        case 34:
        {
            int band ;
            ap_index_type ap_index;
            wifi_freq freq;
            int  netmode=0;
            if (simcom_if_support_5G() == 1)
            {
                printf(" Please input the freq 0~2.4G  1~5G: ");
                memset(scan_string, 0, sizeof(scan_string));
                if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                    continue;
                freq = atoi(scan_string);
                ap_index  = AP_INDEX_0;
            }
            else
            {
                if (!select_ap_index(&ap_index))
                {
                    break;
                }
                freq = FREQ_2_4G;
            }

            ret = get_netmode_plus(&netmode,ap_index,freq);
            if(netmode != E_11N && netmode != E_11NA)
            {
                printf("Set Band fail!\n Maybe you should set netmode to E_11N or E_11NA\n");
                break;
            }
            if (freq == FREQ_2_4G)
            {
                printf(" Please input the band(0~1 0~20M 1~40M): ");
            }
            else if (freq == FREQ_5G)
            {
                printf(" The E_11NA mode can be 20M or 40M \n");
                printf(" Please input the band(0~2 0~20M 1~40M ): ");
            }

            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                continue;
            band = atoi(scan_string);
            ret = set_band_plus(band, ap_index,freq);
            if (ret)
            {
                printf("\nset set_band success!\n");
            }
            else
            {
                printf("\nset set_band failed!\n");
            }
        }
        break;

        case 35:
        {
            int  band=0;
            ap_index_type ap_index;
            wifi_freq freq;

            if (simcom_if_support_5G() == 1)
            {
                printf(" Please input the freq 0~2.4G  1~5G: ");
                memset(scan_string, 0, sizeof(scan_string));
                if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                    continue;
                freq = atoi(scan_string);
                ap_index  = AP_INDEX_0;
            }
            else
            {
                if (!select_ap_index(&ap_index))
                {
                    break;
                }
                freq = FREQ_2_4G;
            }

            ret = get_band_plus(&band,ap_index,freq);
            if (ret)
            {
                printf("\n get_band: %d\n", band);
            }
            else
            {
                printf("\n get_band failed! Maybe you should set netmode to E_11N\n");
            }
        }
        break;

        case 36:
        {    
            client_info_node_ptr head;
            ret= get_clients_info(GET_ALL_CLIENTS_STAT,&head);
            client_info_list_display(head);
            client_info_list_destroy(head);
        }
        break;

        case 37:
        {
            client_info_node_ptr head;
            ret= get_clients_info(GET_CONN_CLIENTS,&head);
            client_info_list_display(head);
            client_info_list_destroy(head);
        }
        break;

        case 38:
        {
            int num = 0;
            printf("Please input the connect_num(5~32): ");
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                continue;
            num = atoi(scan_string);
            ret =   wifi_connect_num_config(num);
            if (ret)
            {
                printf("\nThe number of wifi connections is configured successfully!\n");
            }
            else
            {
                printf("\nFailed to configure the number of wifi connections!\n");
            }
        }
        break;

        case 39:
        {
            int opt = 0;
            printf("Please input the opt(0~1) 1==enable wps: ");
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                continue;
            opt = atoi(scan_string);
            ret =   enable_wps_pushbutton(opt);
            if (ret)
            {
                printf("\nenable wps success!\n");
            }
            else
            {
                printf("\nenable wps failed!\n");
            }
        }
        break;

        case 40:
        {
            int  state=0;
            ret = get_wps_pushbutton_state(&state);
            if (ret)
            {
                printf("\n  get_wps_pushbutton_state: %d\n", state);
            }
            else
            {
                printf("\n  get_wps_pushbutton_state failed!\n");
            }
        }
        break;

        case 41:
        {
            ret = wifi_call_wps_pbc();
            if (ret)
            {
                printf("\n call_wps_pbc success!\n");
            }
            else
            {
                printf("\n call_wps_pbc failed!\n");
            }
        }
        break;

        case 42:
        {
            printf("Please input the pin_code: ");
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                continue;
            //check input param
            ret = wifi_call_wps_pin(scan_string);
            if (ret)
            {
                printf("\n call_wps_pin success!\n");
            }
            else
            {
                printf("\n call_wps_pin failed!\n");
            }
        }
        break;

        case 43:
        {
            ret = wifi_call_wps_cancel();
            if (ret)
            {
                printf("\n call_wps_cancel success!\n");
            }
            else
            {
                printf("\n call_wps_cancel failed!\n");
            }
        }
        break;

        case 44:
        {
            int ret = 0;
            char filter[64] = {0};
            printf("please input mac filter\n");
            printf("For example:disable,allow,deny\n");
            printf("please input>");
            memset(filter, 0, sizeof(filter));
            if (fgets(filter, sizeof(filter), stdin) == NULL)
            continue;
            filter[strlen(filter) - 1] = 0;
            ret= wifi_set_mac_filter_mode(filter);
            if (ret == -1)
            {
                printf("Set up the faild!\n");
            }
            else
            {
                printf("Set up the success!\n");
            }
         }
        break;

        case 45:
        {
            int ret= 0;
            maclist_info_t mac_info;
            ret = wifi_get_mac_filter_info(&mac_info);
            if (ret == -1)
            {
                printf("GET mac address info faild! \n");
            }
            else
            {
                printf("mac mode is %s\n",mac_info.filtermode);
                if (mac_info.allow_flag == 0)
                {
                    printf("allowlist = NULL\n");
                }
                else
                {
                    for(i=0;i<mac_info.allownum;i++)
                    {
                        printf("allowlist = %s\n",mac_info.allowlist[i]);
                        free(mac_info.allowlist[i]);
                    }
                    free(mac_info.allowlist);
                }
                if (mac_info.deny_flag == 0)
                {
                    printf("denylist = NULL\n");
                }
                else
                {
                    for(i=0;i<mac_info.denynum;i++)
                    {
                        printf("denylist = %s\n",mac_info.denylist[i]);
                        free(mac_info.denylist[i]);
                    }
                    free(mac_info.denylist);
                }
            }
        }
        break;

        case 46:
        {
            char scan_string[1024] = {0};
            char filter[256] = {0};                      //A maximum of 14 MAC addresses can be entered
            int ret = 0;
            printf("please add allow mac address filter\n");
            printf("For example:XX:XX:XX:XX:XX:XX\n");
            printf("please input>");
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
            continue;

            if (strlen(scan_string) > 256)
            {
                printf("Enter more than 14 MAC addresses,Input error!\n");
            }
            else
            {
                memset(filter, 0, sizeof(filter));
                memcpy(filter, scan_string, strlen(scan_string) - 1);
                ret = wifi_add_allow_mac_address_filter(filter);
                if (ret == -1)
                {
                    printf("ADD allow mac address faild! Please Enter a correct MAC address!\n");
                }
                else
                {
                    printf("ADD allow mac address Success!\n");
                }
             }
        }
        break;

        case 47:
        {
            char scan_string[1024] = {0};
            char filter[256] = {0};
            int ret = 0;
            printf("please add deny mac address filter\n");
            printf("For example:XX:XX:XX:XX:XX:XX\n");
            printf("please input>");
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
            continue;
            if (strlen(scan_string) > 256)
            {
                printf("Enter more than 14 MAC addresses,Input error!\n");
            }
            else
            {
                memset(filter, 0, sizeof(filter));
                memcpy(filter, scan_string, strlen(scan_string) - 1);
                ret = wifi_add_deny_mac_address_filter(filter);
                if (ret == -1)
                {
                    printf("ADD deny mac address faild! Please Enter a correct MAC address!\n");
                }
                else
                {
                    printf("ADD deny mac address Success!\n");
                }
            }
        }
        break;

        case 48:
        {
            char scan_string[1024] = {0};
            char filter[256] = {0};
            int ret = 0;
            printf("please delete allow mac address filter\n");
            printf("For example:XX:XX:XX:XX:XX:XX\n");
            printf("please input>");
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
            continue;
            if (strlen(scan_string) > 256)
            {
                printf("Enter more than 14 MAC addresses,Input error!\n");
            }

            else
            {
                memset(filter, 0, sizeof(filter));
                memcpy(filter, scan_string, strlen(scan_string) - 1);
                ret = wifi_del_allow_mac_address_filter(filter);
                if (ret == -1)
                {
                    printf("Delete allow mac address faild! Please Enter a correct MAC address!\n");
                }
                else
                {
                    printf("Delete allow mac address Success\n");
                }
            }
        }
        break;

        case 49:
        {
            char scan_string[1024] = {0};
            char filter[256] = {0};
            int ret = 0;
            printf("please delete deny mac address filter\n");
            printf("For example:XX:XX:XX:XX:XX:XX\n");
            printf("please input>");
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
            continue;
            if (strlen(scan_string) > 256)
            {
                printf("Enter more than 14 MAC addresses,Input error!\n");
            }

            else
            {
                memset(filter, 0, sizeof(filter));
                memcpy(filter, scan_string, strlen(scan_string) - 1);
                ret = wifi_del_deny_mac_address_filter(filter);
                if (ret == -1)
                {
                    printf("Delete deny mac address faild! Please Enter a correct MAC address!\n");
                }
                else
                {
                    printf("Delete deny mac address Success\n");
                }
            }
        }
        break;

        case 50:
        {
            int pbc_state = 0;
            ret = get_wps_pbc_state(&pbc_state);
            if (ret)
            {
                if (pbc_state == 1)
                {
                    printf("\n wps_pbc state is Active\n");
                }
                else if (pbc_state == 2)
                {
                    printf("\n wps_pbc state is Disabled\n");
                }
                else if (pbc_state == 3)
                {
                    printf("\n wps_pbc state is Timed-out\n");
                }
                else
                {
                    printf("\n get_wps_pbc_state failed!\n");
                }
            }
            else
            {
                printf("\n get_wps_pbc_state failed!\n");
            }
        }
        break;

        case 51:
        {
            int ret_value = -1;
            ret_value =  get_wifi_mode();
            printf("\n ret_value %d \n",ret_value);
            if (ret_value == AP_MODE)
            {
                printf("\n The current mode is single AP ! \n");
            }
            else if(ret_value == STA_MODE)
            {
                printf("\n The current mode is single-station mode ! \n");
            }
            else if(ret_value == APAP_MODE)
            {
                printf("\n The current mode is APAP_MODE ! \n");
            }
            else if(ret_value == APSTA_MODE)
            {
                printf("\n The current mode is APSTA_MODE ! \n");
            }
            else if(ret_value == UNKNOW_MODE)
            {
                printf("\n Current mode unknown! \n");
            }
            else
            {
                 printf("\n get_wifi_mode failed! \n");
            }
        }
        break;

        
        case 52:
        {
            sta_scan_info_node_ptr head;
            int scan_ret=sta_scan(&head);
            if (scan_ret != -1)
            {
                sta_scan_info_list_display(head);
                sta_scan_info_list_destroy(head);
            }
            else
            {
                printf("\n sta_scan failed! \n");
            }
        }
        break;
        
        case 53:
        {
            char in_ssid[32] = {0};
            char in_psk[64] = {0};
            int ret = 0;
            int lenth=0;
            printf("please input ssid:\n");
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
            {
                 continue;
            }
            lenth = strlen(scan_string);
            scan_string[lenth-1] = '\0';
            strncpy(in_ssid,scan_string,32);
            printf("please input password:\n");
            
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
            {
                 continue;
            }
            lenth = strlen(scan_string);
            scan_string[lenth-1] = '\0';
            strncpy(in_psk,scan_string,64);

            ret = sta_connect(in_ssid,in_psk);
            if (ret == -1)
            {
                printf("sta_connect fail! \n");
            }
            else
            {
                printf("sta_connect Execute successfully\n");
            }
        }
        break;

        case 54:
        {
            int ret = 0;
            wifi_mode_type mode;
            printf("please input wifi_mode 0 -- ap , 1 -- sta, 2 -- APAP , 3 -- APSTA: ");
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
            {
                 continue;
            }
            mode = atoi(scan_string);

            ret = set_wifi_mode(mode);
            if (ret)
            {
                printf("set_wifi_mode success! \n");
            }
            else
            {
                printf("set_wifi_mode fail! \n");
            }
        }
        break;


        case 55:
        {
            int sta_state = 0;
            ret = get_sta_status(&sta_state);
            if (ret)
            {
                if (sta_state == 1)
                {
                    printf("\n sta state is SCANNING\n");
                }
                else if (sta_state == 2)
                {
                    printf("\n sta state is ASSOCIATING\n");
                }
                else if (sta_state == 3)
                {
                    printf("\n sta state is COMPLETED\n");
                }
                else
                {
                    printf("\n get_sta_status failed!\n");
                }
            }
            else
            {
                printf("\n get_sta_status failed!\n");
            }
        }
        break;

        case 56:
        {
            boolean ret = false;
            char ssid_str[32] = {0};
            char psk_value[64] = {0};
            ret = get_sta_cfg(ssid_str,psk_value);
            if (ret)
            {
                printf("\nssid_str: %s\npsk_value: %s\n",ssid_str,psk_value);
            }
            else
            {
                printf("\n get_sta_cfg failed!\n");
            }
        }
        break;

        case 57:
        {
            boolean ret = false;
            char ip_str[16] = {0};
            ret = get_sta_ip(ip_str);
            if (ret)
            {
                printf("ip_str: %s\n",ip_str);
            }
            else
            {
                printf("\n get_sta_ip failed!\n");
            }
        }
        break;


        case 58:
        {
            boolean ret = false;
            ret = sta_disable_network();
            if (ret)
            {
                printf("\nsta_disable_network success\n");
            }
            else
            {
                printf("\nsta_disable_network failed!\n");
            }
        }
        break;
        
        case 59:
        {
            boolean ret = false;
            ret = sta_enable_network();
            if (ret)
            {
                printf("\n sta_enable_network success\n");
            }
            else
            {
                printf("\n sta_enable_network failed!\n");
            }

        }
        break;

        case 60:
        {
            int act = 0;
            wifi_freq freq = FREQ_2_4G;
            ap_index_type ap_index;

            if (simcom_if_support_5G() == 1)
            {
                 ap_index = AP_INDEX_0;
                 printf("   Please input the freq  0~FREQ_2_4G 1~FREQ_5G : ");
                 memset(scan_string, 0, sizeof(scan_string));
                 if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                     continue;
                 freq = atoi(scan_string);
                 printf("   Please input the act  0~disable wifi  1~enable wifi : ");
                 memset(scan_string, 0, sizeof(scan_string));
                 if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                     continue;
                 act = atoi(scan_string);
                // change wifi freq  2.4  ~ 5g
            }
            else
            {
                printf("   Please input the act  0~disable wifi  1~enable wifi : ");
                memset(scan_string, 0, sizeof(scan_string));
                if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                continue;
                act = atoi(scan_string);
           }
            ret = wifi_change_freq(ap_index,freq,act);
            if (ret)
            {
                printf("\n set success\n");
            }
            else
            {
                printf("\n set failed!\n");
            }
        }
        break;

        case 99:
        {
            be_exit = TRUE;
            printf("\nexit\n");
        }
        break;

        default:
            printf("\ninvalid selection!\n");
            break;
        };

        if (be_exit)
        {
            break;
        }
    };

    printf("\n");
}
#endif

static char CHAR_TO_OCTET(char ch)
{
    char rc;

    if (ch >= '0' && ch <= '9') //0-9
    {
        rc = ch - '0';
    }
    else if (ch >= 'A' && ch <= 'F') //A-F
    {
        rc = ch - 'A' + 10;
    }
    else if (ch >= 'a' && ch <= 'f') //a-f
    {
        rc = ch - 'a' + 10;
    }
    else
    {
        rc = -1;
    }

    return rc;
}

static int hex_to_bin(
    const unsigned char *in_str,
    const int in_str_len,
    unsigned char *out_str,
    int out_max_len)
{
    int i, j;

    if (in_str_len % 2 != 0)
    {
        return -1;
    }

    if (out_max_len < in_str_len / 2)
    {
        return -1;
    }

    for (i = 0, j = 0; i < in_str_len; i += 2)
    {
        if ((CHAR_TO_OCTET(in_str[i]) == 0xFF) && (CHAR_TO_OCTET(in_str[i + 1]) == 0xFF))
        {
            return -1;
        }
        out_str[j++] = CHAR_TO_OCTET(in_str[i]) * 16 + CHAR_TO_OCTET(in_str[i + 1]);
    }
    return j;
}

static void simcom_sms_test()
{
    const char *options_list[] =
        {
            "1. Send SMS",
            "2. Get SMSC Address",
            "3. Test Decode PDU",
            "4. Set SMSC Address",
            "99. Back",
        };

    int opt = 0;
    int i, j;
    char scan_string[500] = {0};

    if(sms_init((sms_ind_cb_fcn)process_simcom_ind_message) != 0)
    {
        return;
    }

    while (1)
    {
        printf("\nPlease select an option to test from the items listed below.\n");
        print_option_menu(options_list, sizeof(options_list) / sizeof(options_list[0]));
        printf("Option > ");

        memset(scan_string, 0, sizeof(scan_string));
        if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
        {
            continue;
        }
        /* Convert the option to an integer, and switch on the option entered. */
        opt = atoi(scan_string);
        switch (opt)
        {
        case 1: //ASCII
        {
            char phone_num[25] = {0};
            while (1)
            {
                printf("\nPlease input phone num >");
                memset(scan_string, 0, sizeof(scan_string));
                if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                {
                    continue;
                }
                strcpy(phone_num, scan_string);
                phone_num[strlen(phone_num) - 1] = 0;
                if (0 == strlen(phone_num))
                {
                    continue;
                }
                i = 0;
                if ('+' == phone_num[0])
                {
                    i = 1;
                }
                for (; i < strlen(phone_num); i++)
                {
                    if (!isdigit(phone_num[i]))
                    {
                        break;
                    }
                }
                if (i == strlen(phone_num))
                {
                    break;
                }
                else
                {
                    printf("Input err!\n ");
                    continue;
                }
            }
            while (1)
            {
                printf("\nPlease input message content>");
                memset(scan_string, 0, sizeof(scan_string));
                if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                {
                    continue;
                }
                scan_string[strlen(scan_string) - 1] = 0;
                if (0 == strlen(scan_string))
                {
                    continue;
                }
                if (send_message(phone_num, (unsigned char *)scan_string, strlen(scan_string)) == 0)
                {
                    printf("send sms successful!\n");
                }
                else
                {
                    printf("send sms failed!\n");
                }
                break;
            }
        }
        break;

        case 2:
        {
            char smsc_addr[SMS_SMSC_ADDRESS_LENGTH] = {0};
            if (get_smsc_addr(smsc_addr) == 0)
            {
                printf("smsc_addr=%s\n", smsc_addr);
            }
            else
            {
                printf("get smsc failed!\n");
            }
        }
        break;
        case 3:
        {
             //char *pdu = "099164000339044202F0240BA13100382098F800001211011161632304F4F29C0E";
            char *pdu = "099164000339044202F0640BA13100382098F8000812111101348423340500030F02027CFB597D50CF6D696D6983618361597D597D7684776189C94E0D95194E0D95195C31662F5C315B9E8BDD5B9E8BF4";
            sms_message_gsm * sms_message = NULL;

            sms_message = create_from_pdu(pdu);
            if (sms_message != NULL)
            {
                printf("dcs_type=%d\n", sms_message->dcs_type);
                printf("smsc_address=%s\n", sms_message->smsc_address);
                printf("number_address=%s\n", sms_message->number_address);
                printf("time_stamp=%s\n", sms_message->time_stamp);
                printf("message_content=%s\n", sms_message->message_content);
                printf("message_pdu=%s\n", sms_message->message_pdu);
                printf("is_concate_sms=%d\n", sms_message->is_concate_sms);
                if (sms_message->is_concate_sms == 1)
                {
                    printf("sms_header.ref_number=%d\n", sms_message->sms_header.ref_number);
                    printf("sms_header.msg_count=%d\n", sms_message->sms_header.msg_count);
                    printf("sms_header.seq_number=%d\n", sms_message->sms_header.seq_number);
                }
                free(sms_message);
            }
            else 
            {
                printf("create from pdu failed\n");
            }
        }
            break;
        case 4:
        {
            char smsc_addr[SMS_SMSC_ADDRESS_LENGTH] = {0};
            while (1)
            {
                printf("Please input the number which you want to set for SMSC Address\n");
                printf("eg:+8613334262200 Chongqing telecom in China!\n");
                printf(">");
                memset(smsc_addr, 0, sizeof(smsc_addr));
                if (fgets(smsc_addr, sizeof(smsc_addr), stdin) == NULL)
                {
                    continue;
                }
                smsc_addr[strlen(smsc_addr) - 1] = 0;
                if (0 == strlen(smsc_addr))
                {
                    continue;
                }
                if (set_smsc_addr(smsc_addr) == 0)
                {
                    printf("Set successful!\n");
                }
                else
                {
                    printf("Set failed!\n");
                }
                break;
            }
        }
        break;
        default:
            break;
        }

        if (opt == 99)
        {
            break;
        }
    }

    sms_deinit();
}

#ifdef FEATURE_DEMO_FOTA
static void simcom_ota_test()
{
    const char *options_list[] = {
        "1. Local Update",
        "2. HTTP Update",
        "3. Query Update state",
        "99. Back"};

    int opt = 0;
    int i, j;
    char scan_string[512] = {0};

    while (1)
    {
        printf("\nPlease select an option to test from the items listed below.\n");
        print_option_menu(options_list, sizeof(options_list) / sizeof(options_list[0]));
        printf("Option > ");

        memset(scan_string, 0, sizeof(scan_string));
        if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
            continue;

        /* Convert the option to an integer, and switch on the option entered. */
        opt = atoi(scan_string);
        switch (opt)
        {
        case 1:
        {
            int ret;
            char url[64];
            printf("\nPlease input url >");
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                continue;
        
            memset(url, 0, sizeof(url));
            if (strlen(scan_string) > 0 && strlen(scan_string) < 64)
            {
                strncpy(url, scan_string, strlen(scan_string) - 1);
            }
            else
            {
                strncpy(url, scan_string, 64 - 1);
            }
            ret = upgrade_through_sd(url);
            if (ret == 0)
            {
                printf("\nUpgrade successfully");
            }
            else if(ret == -1)
            {
                printf("\nUpgrade faild");
            }
            else
            {
                printf("\nCat't get upgrade package,please Check whether the path is correct");
                continue;
            }
        }
        break;
        case 2:
        {
            int ret;
            char url[512] = {0};
            printf("\nPlease input url >");
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                continue;
            
            memset(url, 0, sizeof(url));
            strncpy(url, scan_string, strlen(scan_string) - 1);
            ret = upgrade_through_http(url);

            if (ret == 0)
            {
                printf("\nUpgrade successfully");
            }
            else
            {
                printf("\nUpgrade faild");
            }
        }
        break;
        case 3:
        {
            int ret;
            UPDATE_STATE state;
            ret = query_update_state(&state);
            if (ret = 1)
            {
                printf("%d",state);
            }
        }
        break;
        default:
            break;
        }

        if (opt == 99)
        {
            break;
        }
    }
}
#endif

#ifdef FEATURE_DEMO_ADC
static void simcom_adc_test()
{
    const char *options_list[] =
        {
            "1. ADC value(voltage)",
            "2. VBAT value(voltage)",
            "99. Back"
        };

    int opt = 0;
    int i, j;
    int mVal;
    char scan_string[128] = {0};
    int voltage;
    while (1)
    {
        printf("\nPlease select an option to test from the items listed below.\n");
        print_option_menu(options_list, sizeof(options_list) / sizeof(options_list[0]));
        printf("Option > ");

        memset(scan_string, 0, sizeof(scan_string));
        if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
        {
            continue;
        }

        /* Convert the option to an integer, and switch on the option entered. */
        opt = atoi(scan_string);
        switch (opt)
        {
        case 1:
        {
            mVal = get_adc_voltage(&voltage);
            if (mVal < 0)
            {
                printf("ADC READ Failed\r\n");
            }
            else
            {
                printf("ADC(micro voltage): %d\r\n", voltage);
            }
        }
        break;
        case 2:
        {
            mVal = get_vbat_voltage(&voltage);
            if (mVal < 0)
            {
                printf("VBAT READ Failed\r\n");
            }
            else
            {
                printf("VBAT(micro voltage): %d\r\n", voltage);
            }
        }
        break;
        case 99:
            return;
        default:
            printf("ADC Channel Error\r\n");
            break;
        }
    }
}
#endif

#ifdef FEATURE_DEMO_I2C
static void simcom_i2c_test()
{
    const char *options_list[] =
        {
            "0. write slave device",
            "1. read from slave device",
            "99. Back"
        };

    int opt = 0;
    int i, j;
    char scan_string[100] = {0};

    while (1)
    {
        printf("\nPlease select an option to test from the items listed below.\n");
        print_option_menu(options_list, sizeof(options_list) / sizeof(options_list[0]));
        printf("Option > ");

        memset(scan_string, 0, sizeof(scan_string));
        if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
        {
            continue;
        }
        if (!isdigit(scan_string[0]))
        {
            continue;
        }
        /* Convert the option to an integer, and switch on the option entered. */
        opt = atoi(scan_string);
        switch (opt)
        {
        case 0:
        {
            uint16_t slave_addr;
            uint16_t reg;
            uint8_t *buf;
            int len;

            printf("\nPlease input slave addr format 0x36>"); //input the addr
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
            {
                continue;
            }

            sscanf(scan_string, "%x", &slave_addr);
            printf("slave_addr = 0x%x\n", slave_addr);
            printf("\nPlease input slave device register format 0x64>"); //input the addr
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
            {
                continue;
            }

            sscanf(scan_string, "%x", &reg);
            printf("reg = 0x%x\n", reg);
            printf("\nPlease input value format 0x3000>"); //input the addr
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL || strlen(scan_string) < 3)
            {
                continue;
            }

            buf = (uint8_t *)malloc((strlen(scan_string) - 2) / 2);
            sscanf(scan_string, "%x", buf);
            for (i = 0; i < (strlen(scan_string) - 2) / 2; i++)
            {
                printf("buf[%d]=0x%x\n", i, buf[i]);
            }
            printf("\nPlease input write len 2>"); //input the addr
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
            {
                continue;
            }

            sscanf(scan_string, "%d", &len);
            printf("len=%d\n", len);
            if (sim_i2c_write(slave_addr, reg, buf, len))
            {
                printf("i2c send failed\r\n");
            }
            free(buf);
        }
        break;
        case 1:
        {
            uint8_t slave_addr;
            uint16_t reg;
            uint8_t *buf;
            int len;
            printf("\nPlease input slave addr format 0x36>"); //input the addr
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
            {
                continue;
            }

            sscanf(scan_string, "%x", &slave_addr);
            printf("slave_addr = 0x%x\n", slave_addr);
            printf("\nPlease input slave device register format 0x64>"); //input the addr
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
            {
                continue;
            }

            sscanf(scan_string, "%x", &reg);
            printf("reg = 0x%x\n", reg);
            printf("\nPlease input read len 2>"); //input the addr
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
            {
                continue;
            }

            sscanf(scan_string, "%d", &len);
            printf("len=%d\n", len);
            buf = (uint8_t *)malloc(len);
            if (sim_i2c_read(slave_addr, reg, buf, len))
            {
                printf("i2c read failed\r\n");
            }
            else
            {
                for (i = 0; i < len; i++)
                {
                    printf("buf[%d] = 0x%x\n", i, buf[i]);
                }
            }
            free(buf);
        }
        break;
        case 99:
            return;
        default:
            break;
        }
    }
}
#endif

static void simcom_at_test()
{
    const char *options_list[] =
        {
            "1. get Module Version",
            "2. get CSQ",
            "3. get CREG",
            "4. get ICCID",
            "5. get IMEI",
            "6. get CIMI",
            "7. enable/disable volte",
            "8. Manual search network(mcc/mnc,rssi)",
            "9. Set USB AT Switch(default on)",
            "10. Get USB AT Switch",
            "11. get band",
            "12. set band",
            "13. reset band",
            "14. get lock info",
            "15. lock cell",
            "16. lock freq",
            "17. unlock",
            "18. set_gain_NITZtime_enable",
            "19. query_gain_NITZtime_enable",
            "20. query regisetd network info",
            "21. query SIM phonenumber"
            "99. back"};

    int opt = 0;
    int i, j;
    int ret;
    char scan_string[100] = {0};
    char buff[100];

    ret = atctrl_init();
    if (ret != 0)
    {
        return;
    }

    while (1)
    {
        printf("\nPlease select an option to test from the items listed below.\n");
        print_option_menu(options_list, sizeof(options_list) / sizeof(options_list[0]));
        printf("Option > ");

        memset(scan_string, 0, sizeof(scan_string));
        if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
        {
            continue;
        }

        /* Convert the option to an integer, and switch on the option entered. */
        opt = atoi(scan_string);
        switch (opt)
        {
        case 1:
        {
            int ret;
            memset(buff, 0, sizeof(buff));
            ret = getModuleRevision(buff, sizeof(buff));
            printf("version: %s\n", (buff));
        }
        break;
        case 2:
        {
            int ret;
            uint8_t rssi;
            ret = getCSQ(&rssi);
            printf("getCSQ: %d\n", rssi);
        }
        break;
        case 3:
        {
            int reg_val;
            reg_val = getCREG();
            printf("getCREG: %d\n", reg_val);
        }
        break;
        case 4:
        {
            char iccid_buf[ICCID_LENGTH + 1];
            memset(iccid_buf, 0, sizeof(iccid_buf));
            getICCID(iccid_buf, sizeof(iccid_buf));
            printf("getICCID: %s\n", (iccid_buf));
        }
        break;
        case 5:
        {
            char imei_buf[IMEI_LENGTH + 1];
            memset(imei_buf, 0, sizeof(imei_buf));
            getIMEI(imei_buf, sizeof(imei_buf));
            printf("getIMEI: %s\n", (imei_buf));
        }
        break;
        case 6:
        {
            char cimi_buf[CIMI_LENGTH + 1];
            memset(cimi_buf, 0, sizeof(cimi_buf));
            getCIMI(cimi_buf, sizeof(cimi_buf));
            printf("getCIMI: %s\n", (cimi_buf));
        }
        break;

        case 7:
        {
            int enable_value;
            printf("\nPlease input set value[0:disable VoLTE  1:enable VoLTE] >");
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
            {
                continue;
            }
            if(input_digitalized_check(scan_string, strlen(scan_string) - 1) == -1)
            {
                printf("error input");
                continue;
            }
            enable_value = atoi(scan_string);
            if (enable_value == 0 || enable_value == 1)
            {
                ret = enable_or_disable_volte(enable_value);
                if (ret == 0)
                {
                    printf("set volte successfully\n");
                }
                else
                {
                    printf("set volte failed\n");
                }
            }
            else
            {
                printf("error param\n");
                continue;
            }
        }
        break;

        case 8:
        {
            struct simcom_search_net_info *net_info_list = NULL;
            struct simcom_search_net_info *tmp = NULL;
            
            printf("\nManual searching network,Please wait...\n");
            ret = manual_search_network(&net_info_list);
            if (ret == 0)
            {
                printf("manual search network successfully\n");
                while(net_info_list != NULL)
                {
                    tmp = net_info_list;
                    printf("MCC=%d,MNC=%d,RSSI=%d,NET_TYPE=%d\n", tmp->mcc, tmp->mnc, tmp->rssi, tmp->net);                    
                    if(tmp->next != NULL)
                    {
                        net_info_list = tmp->next;
                    }
                    else
                    {
                        net_info_list = NULL;
                    }

                    free(tmp);
                    tmp = NULL;
                }
            }
            else
            {
                printf("manual search network failed\n");
            }
        }
        break;

        case 9:
        {
            int onoff;
            printf("\nPlease input set value[0:close usb at  1:open usb at] >");
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
            {
                continue;
            }

            onoff = atoi(scan_string);
            if (onoff == 0 || onoff == 1)
            {
                ret = set_usb_at_switch(onoff);
                if (ret == 0)
                {
                    printf("set usb at switch successfully\n");
                }
                else
                {
                    printf("set usb at switch failed\n");
                }
            }
            else
            {
                printf("error param\n");
                continue;
            }
        }
        break;

        case 10:
        {
            int onoff;

            ret = get_usb_at_switch(&onoff);
            if (ret == 0)
            {
                if (onoff == 0)
                {
                    printf("usb at state:close\n");
                }
                else
                {
                    printf("usb at state:open\n");
                }
            }
            else
            {
                printf("get usb at switch failed\n");
            }
        }
        break;
        case 11:
        {
            int GSM_band, UMTS_band, LTEH_band, LTEL_band;

            ret = get_current_band(&GSM_band, &UMTS_band, &LTEH_band, &LTEL_band);
            if (ret == 0)
            {
                printf("GSM:%d,UMTS:%d,LTEH:%d,LTEL:%d\n",GSM_band,UMTS_band,LTEH_band,LTEL_band);
            }
            else
            {
                printf("error!\n");
                continue;
            }
        }
        break;

        case 12:
        {
            int GSM_band, UMTS_band, LTEH_band, LTEL_band;
            printf("\ninput GSM_band(0 for no change) >");
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
            {
                continue;
            }
            GSM_band = atoi(scan_string);
            printf("\ninput UMTS_band(0 for no change) >");
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
            {
                continue;
            }
            UMTS_band = atoi(scan_string);
            printf("\ninput LTEH_band(0 for no change) >");
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
            {
                continue;
            }
            LTEH_band = atoi(scan_string);
            printf("\ninput LTEL_band(0 for no change) >");
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
            {
                continue;
            }
            LTEL_band = atoi(scan_string);
            ret = lock_band(GSM_band, UMTS_band, LTEH_band, LTEL_band);
            if (ret == 0)
            {
                printf("sucess!\n");
            }
            else
            {
                printf("error!\n");
                continue;
            }
        }
        break;

        case 13:
        {
            ret = reset_band();
            if (ret == 0)
            {
                printf("success\n");
            }
            else
            {
                printf("error!\n");
                continue;
            }
        }
        break;

        case 14:
        {
            int mode, act, freq, cellid = 0;
            ret = get_frequency_cell_lock_info(&mode, &act, &freq, &cellid);
            if (ret == 0)
            {
                printf("mode:%d, act:%d, freq:%d, cellid:%d\n",mode, act, freq, cellid);
            }
            else
            {
                printf("error!\n");
                continue;
            }
        }
        break;

        case 15:
        {
            int act, freq, cellid = 0;
            printf("\ninput act(0:GSM, 1:UMTS-TD, 2:UMTS_WB, 3:LTE) >");
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
            {
                continue;
            }
            act = atoi(scan_string);
            printf("\ninput freq >");
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
            {
                continue;
            }
            freq = atoi(scan_string);
            printf("\ninput cellid >");
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
            {
                continue;
            }
            cellid = atoi(scan_string);
            ret = lock_cell(act, freq, cellid);
            if (ret == 0)
            {
                printf("success\n");
            }
            else
            {
                printf("error!\n");
                continue;
            }
        }
        break;

        case 16:
        {
            int act, freq = 0;
            printf("\ninput act(0:GSM, 1:UMTS-TD, 2:UMTS_WB, 3:LTE) >");
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
            {
                continue;
            }
            act = atoi(scan_string);
            printf("\ninput freq >");
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
            {
                continue;
            }
            freq = atoi(scan_string);
            ret = lock_frequency(act, freq);
            if (ret == 0)
            {
                printf("success\n");
            }
            else
            {
                printf("error!\n");
                continue;
            }
        }
        break;

        case 17:
        {
            ret = close_frequency_cell_lock();
            if (ret == 0)
            {
                printf("success\n");
            }
            else
            {
                printf("error!\n");
                continue;
            }
        }
        break;
        
        case 18:
        {   
            int act;
            printf("\nPlease set value (0:close CTZU 1:open CTZU) >");
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
            {
                continue;
            }
            act = atoi(scan_string);
            ret =set_gain_NITZtime_enable(act);
            if (ret == 0)
            {
                printf("success\n");
            }
            else
            {
                printf("error!\n");
                continue;
            }
        }
        break;

        case 19:
        {
            int onoff;
            memset(buff, 0, sizeof(buff));
            ret =query_gain_NITZtime_enable(buff,sizeof(buff));
            if (ret == 0)
            {
                printf("CTZU:%s\n",buff);
            }
            else
            {
                printf("error!\n");
                continue;
            }
        }
        break;

        case 20:
        {
            registed_network_info info;
            memset(&info, 0, sizeof(info));
            ret = query_registe_network_information(&info);
            if (ret == 0)
            {
                //printf("success\n");
                printf("======================================\n");
                printf("act:%d,mcc:%d,mnc:%d,area_code:%x,cellid:%d,band:%d,earfcn:%d\n",
                info.access_tech, info.mcc, info.mnc, info.area_code, info.cell_id, info.freq_band, info.earfcn);
                printf("======================================\n");
            }
            else
            {
                printf("error!\n");
                continue;
            }
        }
        break;

        case 21:
        {
            char number[MAX_SIM_INFO_STR_LEN];
            memset(number, 0, sizeof(number));
            ret = query_SIM_number(number);
            if (ret == 0)
            {
                //printf("success\n");
                printf("======================================\n");
                printf("number:%s\n", number);
                printf("======================================\n");
            }
            else
            {
                printf("error!\n");
                continue;
            }
        }
        break;

        default:
            break;
        }

        if (opt == 99)
        {
            break;
        }
    }

    atctrl_deinit();
}

extern int modem_ri_notify_mcu(void);
#define FEATURE_DEMO_GPIO
#ifdef FEATURE_DEMO_GPIO
static void simcom_gpio_test()
{
    const char *options_list[] = {
        "1. Open DTR(PIN72) Interrupt",
        "2. Close DTR(PIN72) Interrupt",
        "3. RI(PIN69) wakeup host",
        "4. get gpio direction",
        "5. get gpio value",
        "6. set gpio isr",
        "99. Back"};
    int opt = 0;
    int ret = 0;
    int *tret = NULL;
    char scan_string[100] = {0};
    int wakeFds[2] = {-1, -1};
    pthread_t g_gpiodtr_thr;

    while (1)
    {
        printf("\nPlease select an option to test from the items listed below.\n");
        print_option_menu(options_list, sizeof(options_list) / sizeof(options_list[0]));
        printf("Option > ");

        memset(scan_string, 0, sizeof(scan_string));
        if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
            continue;

        /* Convert the option to an integer, and switch on the option entered. */
        opt = atoi(scan_string);
        switch (opt)
        {
        case 1:
            if (wakeFds[1] == -1)
            {
                pipe(wakeFds);
                fcntl(wakeFds[0], F_SETFL, O_NONBLOCK);
                fcntl(wakeFds[1], F_SETFL, O_NONBLOCK);
                printf("wakeFds[0]=%d, wakeFds[1]=%d\n", wakeFds[0], wakeFds[1]);
                if ((ret = pthread_create(&g_gpiodtr_thr, NULL, gipo_wakeup_init, &wakeFds[0])) != 0)
                {
                    printf("gpio pthread_create error %d!\n", ret);
                }
            }
            break;
        case 2:
            printf("close DTR interrupt, wakeFds[1]=%d\n", wakeFds[1]);
            gpio_dtr_deinit();
            if (wakeFds[1] > 0)
            {
                int *pret = NULL;
                write(wakeFds[1], "exit", 5);
                pthread_join(g_gpiodtr_thr, (void **)&pret);
                close(wakeFds[0]);
                close(wakeFds[1]);
                wakeFds[0] = -1;
                wakeFds[1] = -1;
            }
            break;
        case 3:
            modem_ri_notify_mcu();
            break;
        case 4:
            {
                int gpio;
                int ret;
                memset(scan_string, 0, sizeof(scan_string));
                printf("\nPlease input which gpio you want to get :"); //input the addr
                if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                {
                    continue;
                }
                gpio = atoi(scan_string);
                ret = gpio_get_dir(gpio);
                if (ret == 0)
                {
                    printf("The direction of gpio%d is in!\n", gpio);
                }
                else if (ret == 1)
                {
                    printf("The direction of gpio%d is out!\n", gpio);
                }
                else
                {
                    printf ("Get the direction of gpio%d failed!\n", gpio);
                }
            }
            break;
        case 5:
            {
                int gpio;
                int ret;
                memset(scan_string, 0, sizeof(scan_string));
                printf("\nPlease input which gpio you want to get :"); //input the addr
                if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                {
                    continue;
                }
                gpio = atoi(scan_string);
                ret = gpio_get_value(gpio);
                if (ret == 0)
                {
                    printf("gpio%d is low level!\n", gpio);
                }
                else if (ret == 1)
                {
                    printf("gpio%d is high level!\n", gpio);
                }
                else
                {
                    printf ("Get the value of gpio%d failed!\n", gpio);
                }
            }
            break;
        case 6:
            {
                int gpio;
                int ret;
                gpio_edge_type edge;
                memset(scan_string, 0, sizeof(scan_string));
                printf("\nPlease input which gpio you want to set:"); //input the addr
                if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                {
                    continue;
                }
                gpio = atoi(scan_string);


                memset(scan_string, 0, sizeof(scan_string));
                printf("\nPlease input which edge mode you want to set\n");
                printf("(0 means none, 1 means rising, 2 means falling, 3 means both):");
                if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                {
                    continue;
                }
                edge = atoi(scan_string);
                
                ret = gpio_set_isr(gpio,edge);
                if (ret == 0)
                {
                    printf("Set gpio%d isr succeed!\n", gpio);
                }
                else
                {
                    printf("Set gpio%d isr failed!\n", gpio);
                }
            }
            break;

        default:
            break;
        }

        if (opt == 99)
        {
            if (wakeFds[1] > 0)
            {
                int *pret = NULL;
                write(wakeFds[1], "exit", 5);
                pthread_join(g_gpiodtr_thr, (void **)&pret);
                close(wakeFds[0]);
                close(wakeFds[1]);
                wakeFds[0] = -1;
                wakeFds[1] = -1;
            }
            break;
        }
    }
}
#endif

#ifdef FEATURE_DEMO_UART
extern void uart_main(void);
static void simcom_uart_test()
{
    uart_main();
}
#endif

#ifdef FEATURE_DEMO_SPI

//extern int sim_ssp_bytes(SSPCONTROL_S *ctl, uint8_t *send, uint8_t *recv, uint32_t len);
static void simcom_spi_test()
{
    const char *options_list[] = {
        "0. spi data transfer",
        "99. Back"};

    int opt = 0;
    int i, j;
    char scan_string[128] = {0};

    while (1)
    {
        printf("\nPlease select an option to test from the items listed below.\n");
        print_option_menu(options_list, sizeof(options_list) / sizeof(options_list[0]));
        printf("Option > ");

        memset(scan_string, 0, sizeof(scan_string));
        if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
            continue;

        /* Convert the option to an integer, and switch on the option entered. */
        opt = atoi(scan_string);
        switch (opt)
        {
        case 0:
        {
            SSPCONTROL_S ssp = {
                .umode = {0},
                .bits = 8,
                .speed = 500000,
                .delay = 0,
            };
            uint8_t *send;
            uint8_t *recv;
            int len;

            memset(scan_string, 0, sizeof(scan_string));
            printf("\nPlease input value format 0x30000000>"); //input the addr
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                continue;

            send = (uint8_t *)malloc(strlen(scan_string));
            memset(send, 0x00, strlen(scan_string));
            recv = (uint8_t *)malloc(strlen(scan_string));
            memset(recv, 0x00, strlen(scan_string));

            sscanf(scan_string, "%x", send);
            len = strlen(send);
            //printf("malloc ok %d %s\r\n",len,send );
            for (i = 0; i < len; i++)
                printf("send[%d]=0x%x\n", i, send[i]);

            if (sim_ssp_bytes(&ssp, send, recv, len))
            {
                printf("SPI DATA Transfer failed\r\n");
            }
            free(send);
            free(recv);
        }
        break;
        case 99:
            return;
        default:
            break;
        }
    }
}
#endif

#ifdef FEATURE_DEMO_GPS

void simcom_event_loc_ind(simcom_event_e event_id, void *data)
{
    switch (event_id)
    {
    case SIMCOM_EVENT_LOC_INFO_IND:
    {
        simcom_loc_info *loc_info = (simcom_loc_info *)data;
        printf("\nLATITUDE:%f  LONGITUDE: %f  ALTITUDE: %f  SPEED: %f  TIME: %02d-%02d-%02d %02d:%02d:%02d\n",
               loc_info->latitude, loc_info->longitude, loc_info->altitude, loc_info->speed, loc_info->time[0], loc_info->time[1], loc_info->time[2], loc_info->time[3], loc_info->time[4], loc_info->time[5]);
    }
    break;
    case SIMCOM_EVENT_LOC_NMEA_IND:
        printf("%s", (char *)data);
        break;
    case SIMCOM_EVENT_LOC_NMEA_BY_ROW_IND:
    {
        NMEA_LINE *len_data = (NMEA_LINE *)data;
        printf("len_data->data %s\n",len_data->data);
        printf("len_data->len %d\n",len_data->length);
    }
        break;
    default:
        break;
    }
}

static void simcom_gps_test()
{
    char scan_string[20] = {0};
    uint8 type;
    int loop = 1;
    int opt;
    int ret = 0;
    char data[512] = {0};
    char gpsfirminfo[100] = {0};
    int temp_ret;
    
    const char *options_list[] = {
        "1. GET Location Info",
        "2. Cold Start",
        "3. Hot Start",
        "4. Close GPS",
        "5. AGNSS enable",
        "6. AGNSS disable",
        "7. Power off GPS",
        "8. Set Dynamic Download flag",
        "9. Get GPS Version Info",
        "10.Get Dynamic_status ",
        "11.Send GNSS CMD",
        "12.Switch GNSS MODE",
        "13.GNSS INIT ",
        "14.GNSS OPEN",
        "15 GNSS DEINIT",
        "16 OUTPUT PROT SWITCH",
        "17 GNSS STOP",
        "18 GNSS GET AGPS",
        "19 OUTPUT DATA SWITCH",
        "99. Back"};

#if (defined __SIMCOM_PROJ_A7600C_LXBD__)
    if (simcom_loc_init(simcom_event_loc_ind, SIMCOM_GNSS_GPIO) != EXIT_SUCCESS)
    {
        printf("simcom loc init failed");
        return;
    }
#elif (defined __SIMCOM_PROJ_A7605CR2_LXBD__)
    if (simcom_loc_init(simcom_event_loc_ind, LONGSHANG_GNSS_GPIO) != EXIT_SUCCESS)
    {
        printf("simcom loc init failed");
        return;
    }
#endif
    while (loop)
    {
        /* Display menu of options. */
        printf("Please select an option to test from the items listed below.\n");
        print_option_menu(options_list, sizeof(options_list) / sizeof(options_list[0]));
        printf("Option > ");

        /* Read the option from the standard input. */
        if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
            continue;
        opt = atoi(scan_string);
        switch (opt)
        {
        /*=======================================================================================*/
        case 1:
        {
                if (simcom_loc_track_start() != EXIT_SUCCESS)
            {
                printf("Location track Failed!\n");
            }
        }
        break;
        /*=======================================================================================*/
        case 2:
        {
            if (simcom_loc_coldstart() != EXIT_SUCCESS)
            {
                printf("Location ColdStart Failed!\n");
            }
        }
        break;
        /*=======================================================================================*/
        case 3:
        {
            if (simcom_loc_hotstart() != EXIT_SUCCESS)
            {
                printf("Location hotStart Failed!\n");
            }
        }
        break;
        /*=======================================================================================*/
        case 4:
        {
            if (simcom_loc_stop() != EXIT_SUCCESS)
            {
                printf("Location stop Failed!\n");
            }
        }
        break;
        /*=======================================================================================*/
        case 5:
        {
              simcom_loc_agnss_enable();
              printf("Location agnss enable succeed!\n");
        }
        break;
        /*=======================================================================================*/
        case 6:
        {
              simcom_loc_agnss_disable();
              printf("Location agnss disable succeed!\n");
        }
        break;
        /*=======================================================================================*/
        case 7:
        {
              if(simcom_loc_poweroff() != EXIT_SUCCESS)
              {
                printf("Location poweroff Failed!\n");
              }
              else
              {
                printf("Location poweroff succeed!\n");
              }
        }
        break;
        /*=======================================================================================*/
        case 8:
        {
              simcom_loc_dynamic_upgrade_disable();
              printf("Location dynamic upgrade disable succeed!\n");
        }
        break;
        /*=======================================================================================*/
        case 9:
        {
            memset(gpsfirminfo,0,sizeof(gpsfirminfo));
            temp_ret = simcom_loc_get_version_info(gpsfirminfo);
            if (temp_ret == 0)
            {
                printf("%s\n",gpsfirminfo);
            }
            else
            {
                printf("temp_ret: %d\n",temp_ret);
            }
        }
        break;
        
        case 10:
        {
            temp_ret = get_dynamic_status();
            printf("%d\n",temp_ret);
        }
        break;
        
        case 11:
        {
            temp_ret = simcom_loc_send_gnss_cmd("$CFGSYS,H01");
            sleep(1);
            simcom_loc_send_gnss_cmd("$CFGMSG,0,5,1");
            simcom_loc_send_gnss_cmd("$CFGNMEA,h30");
            printf("%d\n",temp_ret);
        }
        break;
        
        case 12:
        {
            GNSS_MODE mode;
#if (defined __SIMCOM_ASR_5311__)
            printf("\nPlease input GNSS mode(1-7):");
#else
            printf("\nPlease input GNSS mode(1-4):");
#endif
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
            {
                continue;
            }
            mode = atoi(scan_string);
            Sunsea_Gnss_SetPositionMode(mode);
        }
        break;
        case 13:
        {
            Sunsea_Gnss_Init(simcom_event_loc_ind, SIMCOM_GNSS_GPIO);
        }
        break;
        case 14:
        {
            Sunsea_Gnss_Start();
        }
        break;
        case 15:
        {
            Sunsea_Gnss_Deinit();
        }
        break;
        case 16:
        {
            NMEA_DATA_OUTPUT_PORT port;
            printf("\nPlease input GNSS port(0-cb,1-nmea_prot):");
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
            {
                continue;
            }
            port = atoi(scan_string);
            Sunsea_Gnss_Output_Channel_Switch(port);
        }
        break;
        case 17:
        {
            Sunsea_Gnss_Stop();
        }
        break;
        case 18:
        {
            Sunsea_Gnss_AgpsReqData();
        }
        break;
        case 19:
        {
            NMEA_LOC_INFO_SWITCH data_output_mode;
            printf("\nPlease input GNSS port(0-cb,1-nmea_prot):");
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
            {
                continue;
            }
            data_output_mode = atoi(scan_string);
            Sunsea_Gnss_Nmea_Info_Switch(data_output_mode);
        }
        break;
        case 99:
        {
            simcom_loc_stop();
            loop = 0;
        }
        break;
        default:
            break;
        }
    }
    simcom_loc_deinit();
}
#endif

#ifdef FEATURE_DEMO_BT
/**********************************************************************************
         Bluetooth 
************************************************************************************/
static void bt_setTimespecRelative(struct timespec *p_ts, long long msec)
{
    struct timeval tv;
    gettimeofday(&tv, (struct timezone *)NULL);
    p_ts->tv_sec = tv.tv_sec + (msec / 1000);
    p_ts->tv_nsec = (tv.tv_usec + (msec % 1000) * 1000L) * 1000L;
}

static void bt_wait_response(int timeout)
{
    struct timespec ts;
    bt_setTimespecRelative(&ts, timeout * 1000 + 2000);
    pthread_cond_timedwait(&s_btcond, &s_btmutex, &ts);
}
static void bt_stop_wait_response(void)
{
    pthread_cond_signal(&s_btcond);
}
#if 0
void simcom_process_response(bt_msg *recv_msg)
{
    //int result = 0;
    int i;
    char response[512] = {0};
    switch (recv_msg->command)
    {
    case BT_SEARCH_COMMAND:
        /* ------------------------------------
            output scan result
            param1: search status: search end
            param2: search result index
            param3: rssi;
            address: device mac address
            data_len:  data name length
            data:   device name
            ---------------------------------------*/
        if (recv_msg->param1 == 0)
        {
            printf("BTSCAN: end\r\n"); // scan end
        }
        else if (recv_msg->param1 == 1)
        {
            printf("BTSCAN:  %d, %s, %02X:%02X:%02X:%02X:%02X:%02X, %d\r\n",
                   recv_msg->param2,
                   (recv_msg->data_len > 0) ? (char *)recv_msg->data : "unknow",
                   (recv_msg->address.nap >> 8) & 0xFF,
                   recv_msg->address.nap & 0xFF,
                   recv_msg->address.uap,
                   (recv_msg->address.lap >> 16) & 0xFF,
                   (recv_msg->address.lap >> 8) & 0xFF,
                   recv_msg->address.lap & 0xFF,
                   recv_msg->param3);
        }
        break;

    case BT_BOND_COMMAND:
    {
        /* -----------------------------------
                match scan result
                param1: bond result 0: FAILED  1:SUCCESS
                address: device mac address
                data_len  device name length
                data:    if data_len > 0  deviceName
                ---------------------------------------*/

        if (recv_msg->param1 == BT_BOND_SUCCESS)
        {
            printf("+BTPAIR: %s, %02X:%02X:%02X:%02X:%02X:%02X\r\n",
                   (recv_msg->data_len > 0) ? (char *)recv_msg->data : "unknow",
                   (recv_msg->address.nap >> 8) & 0xFF,
                   recv_msg->address.nap & 0xFF,
                   recv_msg->address.uap,
                   (recv_msg->address.lap >> 16) & 0xFF,
                   (recv_msg->address.lap >> 8) & 0xFF,
                   recv_msg->address.lap & 0xFF);
        }
        else if (recv_msg->param1 == BT_BOND_FAILED)
        {
            /* -----------bond fail end ---------*/
            /*  --- result code -----
                    recv_msg->param2 is result code: 
                    CSR_BT_RESULT_CODE_SC_SUCCESS                (0x0000)
                    CSR_BT_RESULT_CODE_SC_BONDING_FAILED         (0x0001)
                    CSR_BT_RESULT_CODE_SC_BONDING_CANCELLED      (0x0002)
                    CSR_BT_RESULT_CODE_SC_SECURITY_FAIL          (0x0003)
                    CSR_BT_RESULT_CODE_SC_SECURITY_MODE_NOT_SET  (0x0004)
                    CSR_BT_RESULT_CODE_SC_INTERNAL_ERROR         (0x0005)
                    CSR_BT_RESULT_CODE_SC_ERROR_UNSPECIFIED      (0x0006)
                    CSR_BT_RESULT_CODE_SC_NO_SUCH_DEVICE         (0x0007)
                    CSR_BT_RESULT_CODE_SC_BONDING_ACCEPT_TIMEOUT (0x0008) */
            printf("+BTPAIR:  pair fail\r\n");
        }
    }
    break;
    case BT_ACCEPT_COMMAND:
    {
        /*--------------------------------------------
                tip user to check 
                param1:  accept mode
                param2:  if ((param1 == ACCEPT_MODE_NOTIFICATION) || 
                             (param1 == ACCEPT_MODE_COMPARE) ||
                             (param1 == ACCEPT_MODE_PASSKEY))   
                          param2 is passkey value
                         else 
                            ignore
                            
                address: device address
                data_len:   data length
                data:    device name  (maybe null)
                --------------------------------------------*/
        if ((recv_msg->param1 == ACCEPT_MODE_NOTIFICATION) ||
            (recv_msg->param1 == ACCEPT_MODE_COMPARE) ||
            (recv_msg->param1 == ACCEPT_MODE_PASSKEY))
        {
            printf("+BTPAIRING: %d, %s, %02X:%02X:%02X:%02X:%02X:%02X, %d\r\n",
                   recv_msg->param1,
                   (recv_msg->data_len > 0) ? (char *)recv_msg->data : "unknow",
                   (recv_msg->address.nap >> 8) & 0xFF,
                   recv_msg->address.nap & 0xFF,
                   recv_msg->address.uap,
                   (recv_msg->address.lap >> 16) & 0xFF,
                   (recv_msg->address.lap >> 8) & 0xFF,
                   recv_msg->address.lap & 0xFF,
                   recv_msg->param2);
        }
        else
        {
            printf("+BTPAIRING: %d, %s, %02X:%02X:%02X:%02X:%02X:%02X\r\n",
                   recv_msg->param1,
                   (recv_msg->data_len > 0) ? (char *)recv_msg->data : "unknow",
                   (recv_msg->address.nap >> 8) & 0xFF,
                   recv_msg->address.nap & 0xFF,
                   recv_msg->address.uap,
                   (recv_msg->address.lap >> 16) & 0xFF,
                   (recv_msg->address.lap >> 8) & 0xFF,
                   recv_msg->address.lap & 0xFF);
        }

        if (recv_msg->param1 == ACCEPT_MODE_NOTIFICATION)
        {
            /* just notify user the bonding information, please wait next msg */
            break;
        }

        /*----- need  accept/reject/passkey  ----- */

        // because the item_test fgets will block the keyboard input.
        // goto main thread(item_test) to get keyboard input

        //  tip: memcpy(&s_bt_msg, recv_msg, sizeof(bt_msg));
        printf("If accept bond? (Y / N):\r\n");
        break;
    }

    case BT_GET_BONDED_COMMAND:

        /*---------------------------------------------
             * get match list
             * param1 = total bonded number
             * param2 = current index;
             * param3 :  if total bonded unmber more than 0,  1: last one
             * address = devices;
             *-------------------------------------------------*/
        if (recv_msg->param1 == 0)
        {
            printf("+BTPAIRED: 0\r\n");
        }
        else
        {
            printf("+BTPAIRED: %d, %d, %s, %02X:%02X:%02X:%02X:%02X:%02X\r\n",
                   recv_msg->param1,
                   recv_msg->param2,
                   (recv_msg->data_len > 0) ? (char *)recv_msg->data : (char *)"unknow",
                   (recv_msg->address.nap >> 8) & 0xFF,
                   recv_msg->address.nap & 0xFF,
                   recv_msg->address.uap,
                   (recv_msg->address.lap >> 16) & 0xFF,
                   (recv_msg->address.lap >> 8) & 0xFF,
                   recv_msg->address.lap & 0xFF);
        }
        break;

    case BT_CONNECT_COMMAND:
        if (recv_msg->param1 == 0)
        {
            printf("spp connected fail!\n");
        }
        else
        {
            printf("spp connected success!  %02X:%02X:%02X:%02X:%02X:%02X\n",
                   (recv_msg->address.nap >> 8) & 0xFF,
                   recv_msg->address.nap & 0xFF,
                   recv_msg->address.uap,
                   (recv_msg->address.lap >> 16) & 0xFF,
                   (recv_msg->address.lap >> 8) & 0xFF,
                   recv_msg->address.lap & 0xFF);
            printf("spp max data len: %d\n", recv_msg->param2);
        }

        break;

    case BT_DISCONNECT_COMMAND:
        printf("spp disconnected!\n");
        break;

    case BT_SPP_RECV_COMMAND:
        /*---------------------------------------------
             * SPP
             * data_len:   receive data len
             * data:       receive data
             * -------------------------------------------------*/
        printf("recv data len: %d\r\n", recv_msg->data_len);
        printf("recv data:%s\r\n", recv_msg->data);
        break;

    case BT_GATT_CONNECT_COMMAND:
        if (recv_msg->param1 == 1)
        {
            printf("gatt connected.\r\n");
        }
        else
        {
            printf("gatt disconnected.\r\n");
        }
        break;

    case BT_GATT_READ_IND_COMMAND:
        /* other devies want to read module data by BT */
        printf("+BT_GATT_READ_IND:handle:%d\r\n", recv_msg->attrHandle);
        memcpy(&s_bt_msg, recv_msg, sizeof(bt_msg));
        printf("IF Accept? Y/N\r\n");
        break;

    case BT_GATT_WRITE_IND_COMMAND:
        /* other devies want to read module data by BT */
        printf("+BT_GATT_WRITE_IND:handle:%d,", recv_msg->attrHandle);
        for (i = 0; i < recv_msg->data_len; i++)
        {
            printf("%2X", recv_msg->data[i]);
        }
        memcpy(&s_bt_msg, recv_msg, sizeof(bt_msg));
        printf("\r\nIF Accept? Y/N\r\n");
        break;

    case BT_GATT_NOTIFY_COMMAND:
        /*---------------------------------------------
             * NOTIFY
             * -------------------------------------------------*/
        printf("gatt notify has been sent.\n");
        break;

    case BT_GATT_INDICATION_COMMAND:
        /*---------------------------------------------
             * ind
             * -------------------------------------------------*/
        printf("gatt indication has been sent.\n");
        break;

    default:
        break;
    }

    //return result;
}

#endif
void ble_scan_info_list_display(ble_scan_info_node_ptr head)
{
    ble_scan_info_node_ptr p;
    if(head)
    {
      p = head->next;
    }
    else
    {
        return;
    }

    while (p != NULL)
    {
       printf("%-20s",p->addr);
       printf("%s\n",p->name);
       p = p->next;
    }
}
void ble_scan_info_list_destroy(ble_scan_info_node_ptr head)
{
    ble_scan_info_node_ptr p = head;
    while (p)
    {
        head = head->next;
        free(p);
        //delete(p);
        p = head;
    }
}


static void simcom_bt_test()
{
    const char *options_list[] =
        {
            "1. Scan",
            "2. Connect",
            "3. Get Characteristic",
            "4. Write value",
            "5. Read Value",
            "6. Disconnect",
            /*" 1. Power",        //because of the gpio port is not immobilization so power mannger development is on hold
            " 2. Gatt Register",
            " 3. Gatt Create DataBase",
            " 4. Create Service",
            " 5. Create Characteristic",
            " 6. Create Descriptor",
            " 7. Add Srvice to DataBase",
            " 8. Gatt Active",
            " 9. Gatt send notify",
            " 10. Gatt send indication",
            " 11. Get scan enable",
            " 12. Set scan enable",
            " 13. Host",
            " 14. SPP",*/
            "99. Back"};
    int opt = 0;
    int ret = 0;
    char scan_string[100] = {0};
    int scan_state = 0;
            
    while (1)
    {
        printf("\nPlease select an option to test from the items listed below.\n");
        print_option_menu(options_list, sizeof(options_list) / sizeof(options_list[0]));
        printf("Option > ");

        memset(scan_string, 0, sizeof(scan_string));
        if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
            continue;
            
        /* Convert the option to an integer, and switch on the option entered. */
        opt = atoi(scan_string);
        switch (opt)
        {
        case 1:
            {
                ble_scan_info_node_ptr head;
                printf("LE Scan start \n");
                ret = ble_scan(&head);
                if (ret != -1)
                {
                    ble_scan_info_list_display(head);
                    ble_scan_info_list_destroy(head);
                }
                else
                {
                    printf("\n sta_scan failed! \n");
                }
            }
            break;
        case 2:
            {
                int sec;
                int dst_type;
                char dst_addr[100]={0};
                connect_init(&sec,&dst_type,dst_addr);
                bt_connect(sec,dst_type,dst_addr);
            }
            break;
        case 3:
            {
                get_characteristic();
            }
            break;
        case 4:
            {
                char handle[16] = {0};
                char value[32] = {0};
                printf("Please enter the handle you want to operate on\n");
                memset(handle, 0, sizeof(handle));
                fgets(handle, sizeof(handle), stdin);
                handle[strlen(handle) - 1] = '\0';
                printf("Please enter the value you want to write\n");
                memset(value, 0, sizeof(value));
                fgets(value, sizeof(value), stdin);
                char_write_cmd(handle,value);
            }
            break;
        case 5:
            {
                int length,ret,i;
                char buf[128] = {0};
                ret = read_value(buf,&length);
                if (ret == 0)
                {
                    printf("value = ");
                    for (i = 0;i < length;i++)
                    {
                        printf("%02x",buf[i]);
                    }
                }
            }
            break;
        case 6:
            {
                ble_disconnect();
            }
        default:
            break;
        }
        if (opt == 99)
        {
            break;
        }
    }
}

    
#if 0
    int array_size;
    char scan_string[40];
    int opt, i;
    int bAccept = 0;
    int ret;
    int inquiry_scan_status;
    int page_scan_status;

    printf("bt_item_test\r\n");

    array_size = sizeof(options_list) / sizeof(options_list[0]);
    ret = bt_init((bt_ind_cb_fcn)simcom_process_response);
    if (ret != 0)
    {
        return;
    }

    while (TRUE)
    {
        /* Display menu of options. */
        printf("\n@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
        printf("\nPlease select an option to test from the items listed below.\n");
        print_option_menu(options_list, sizeof(options_list) / sizeof(options_list[0]));
        printf("Option > ");

        if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
        {
            continue;
        }

        if ((strncmp(scan_string, "y", 1) == 0) || (strncmp(scan_string, "Y", 1) == 0))
        {
            bAccept = 1;
        }
        else if ((strncmp(scan_string, "n", 1) == 0) || (strncmp(scan_string, "N", 1) == 0))
        {
            bAccept = 0;
        }
        else
        {
            bAccept = -1;
            if ((scan_string[0] < '0') || (scan_string[0] > '9'))
            {
                continue;
            }
        }

        if (bAccept >= 0)
        {
            /*-----------------------process bond accept or reject -------------------------------*/
            bt_msg send_msg;
            memset(&send_msg, 0, sizeof(bt_msg));

            if (s_bt_msg.command == BT_ACCEPT_COMMAND)
            {
                if (bAccept == 1 && s_bt_msg.param1 == ACCEPT_MODE_PASSKEY)
                {
                    int passkey = 0;

                    printf("\r\n please enter the passkey:");
                    memset(scan_string, 0, sizeof(scan_string));
                    if (fgets(scan_string, sizeof(scan_string), stdin) != NULL)
                    {
                        passkey = atoi(scan_string);
                    }
                    send_msg.param3 = passkey;
                }

                else if (bAccept == 1 && s_bt_msg.param1 == ACCEPT_MODE_PINCODE)
                {
                    printf("\r\n please enter pincode the passkey(max len 16):");
                    memset(scan_string, 0, sizeof(scan_string));
                    fgets(scan_string, sizeof(scan_string), stdin);
                    strncpy((char *)send_msg.data, scan_string, strlen(scan_string) - 1);
                }

                accept_bond(bAccept, (accept_mode)s_bt_msg.param1);
            }
            if (s_bt_msg.command == BT_GATT_READ_IND_COMMAND)
            {
                char data[50];
                int i;
                if (bAccept == 1)
                {
                    //CSR_BT_GATT_ACCESS_RES_SUCCESS  0
                    printf("\r\n please enter data to be read:");
                    memset(data, 0, sizeof(data));
                    fgets(data, sizeof(data), stdin);
                    for (i = 0; i < sizeof(data); i++)
                    {
                        if (0x0a == data[i])
                            data[i] = '\0';
                    }
                    bt_gatt_read_cfm(s_bt_msg.attrHandle, 0, (uint8 *)data, strlen(data));
                }
                else
                {
                    //#define CSR_BT_GATT_ACCESS_RES_READ_NOT_PERMITTED   2
                    bt_gatt_read_cfm(s_bt_msg.attrHandle, 2, NULL, 0);
                }
            }
            if (s_bt_msg.command == BT_GATT_WRITE_IND_COMMAND)
            {
                if (bAccept == 1)
                {
                    bt_gatt_write_cfm(s_bt_msg.attrHandle, 3);
                }
                else
                {
                    //CSR_BT_GATT_ACCESS_RES_WRITE_NOT_PERMITTED   3
                    bt_gatt_write_cfm(s_bt_msg.attrHandle, 3);
                }
            }

            continue;
        }

        opt = atoi(scan_string);
        switch (opt)
        {
        case 1:
        {
            int bPower;
            printf("\r\n please select power status(1: power on 0: power off):");
            fgets(scan_string, sizeof(scan_string), stdin);
            bPower = atoi(scan_string);
            simcom_bt_power_on(bPower); 
        }
        break;
#if 0
            case BT_SEARCH_COMMAND:
                {
                    int timeout = 0;
                    int mode;
                    int bsubcmd;          
                    printf("\r\n please search cmd: 1: start  0: stop \r\n");
                    fgets(scan_string, sizeof(scan_string), stdin);
                    bsubcmd = atoi(scan_string);
                    if(bsubcmd > 1 || bsubcmd < 0)
                    {
                        continue;
                    }
                    if(bsubcmd)
                    {
                        printf("\r\n search mode: 0: normal serach  1: hide paired:\r\n");
                        fgets(scan_string, sizeof(scan_string), stdin);
                        
                        mode = atoi(scan_string);
                        
                        if(mode > 1 || mode < 0)  
                        {
                            continue;
                        }

                        printf("\r\n search timeout:\r\n");
                        fgets(scan_string, sizeof(scan_string), stdin);
                        
                        timeout = atoi(scan_string);
                        
                        if(timeout > 60 || timeout < 0)
                        {
                            continue;
                        }
                    }
                    search_devices(bsubcmd,mode,timeout);
                    printf("\r\n");
                }
                break;

            case BT_IOCAP_COMMAND:
                {
                    int mode;
                    printf("\r\n please select io cap mode(0~3):");
                    fgets(scan_string, sizeof(scan_string), stdin);
                    mode = atoi(scan_string);
                    if(mode > IO_CAP_NO_INPUT_NO_OUTPUT || mode < IO_CAP_DISPLAY_ONLY)
                    {
                        continue;
                    }   
                    ret = set_iocap_device(mode);
                    if(ret == 0)
                    {
                        printf("set io cap success!\n");
                    }
                    else
                    {
                        printf("set io cap fail!\n");
                    }
                }
                break;
                
            case BT_BOND_COMMAND:
                {
                    int index;
                    printf("\r\n please select bond id:");
                    fgets(scan_string, sizeof(scan_string), stdin);
                    index = atoi(scan_string);
                    ret = bond_device(index);
                    if(ret < 0)
                    {
                        printf("bond_device fail!\n");
                    }
                }
                break;     

            case BT_DEBOND_COMMAND:
                {
                    int index;                    
                    printf("\r\n please select debond id:");
                    fgets(scan_string, sizeof(scan_string), stdin);
                    index = atoi(scan_string);
                    ret = debond_device(index);
                    if(ret < 0)
                    {
                        printf("debond_device fail!\n");
                    }
                    else
                    {
                        printf("debond_device success!\n");
                    }
                }
                break;
                
            case BT_GET_BONDED_COMMAND:
                {
                    ret = get_bonded_device();
                    if(ret < 0)
                    {
                        printf("get_bonded_device fail!\n");
                    }                    
                }
                break;
            case BT_PIN_READ_COMMAND:
                {
                    int bsubcmd;       
                    char pin[50]={'\0'};
                    printf("\r\n please choose cmd: 1: set pin  0: read pin \r\n");

                    fgets(scan_string, sizeof(scan_string), stdin);
                    
                    bsubcmd = atoi(scan_string);
                    
                    if(bsubcmd > 1 || bsubcmd < 0)
                    {
                        continue;
                    }

                    if(bsubcmd)
                    {
                        printf("\r\n please input pin(numberic 4-16):\r\n");
              			    memset(scan_string, 0, sizeof(scan_string));
              			    memset(pin, 0, sizeof(pin));
                        fgets(scan_string, sizeof(scan_string), stdin);	
              			    if((strlen(scan_string)>=5 )&&(strlen(scan_string)<=17))
              			    {
                			   	  scan_string[strlen(scan_string)-1]='\0';
                            strncpy(pin, scan_string,strlen(scan_string));
              			    }
              			   else
              			   {
                			 	    printf("the length of pin is wrong,exit!!!\n");
                			  	  break;
              			   }
                        ret = pin_set(pin);
                        if(ret < 0)
                        {
                            printf("set pin fail!\n");
                        }
                        else
                        {
                            printf("set pin success!\n");
                        }
                    }
                    else
                    {
                        char pincode[32];
                        ret = pin_read(pincode);
                        if(ret < 0)
                        {
                            printf("read pin fail!\n");
                        }
                        else
                        {
                            printf("read pincode = %s.\n",pincode);
                        }                        
                    }
                }
                break;
            case BT_SPP_SERVER_COMMAND:
                {
                    int bActive;
                    printf("\r\n please set mode: 1: active  0:deactive\r\n");
                    fgets(scan_string, sizeof(scan_string), stdin);
                    bActive = atoi(scan_string);
                    ret = spp_server(bActive);
                    if(ret < 0)
                    {
                        printf("spp_server fail!\n");
                    }
                    else
                    {
                        printf("spp_server success!\n");
                    }  
                }
                break;
            case BT_GET_PROFILE_COMMAND:
                {
                    int index;
                    int isSupportSPP;
                    printf("\r\n please select device id:\r\n");
                    fgets(scan_string, sizeof(scan_string), stdin);
                    index = atoi(scan_string);
                    ret = bt_get_profile(index, &isSupportSPP);
                    if(ret < 0)
                    {
                        printf("bt_get_profile fail!\n");
                    }
                    else
                    {
                        printf("isSupportSPP = %d.\n", isSupportSPP);
                    }
                }
                break;
            case BT_CONNECT_COMMAND:
                {
                    int index;
                    printf("\r\n please select device id:\r\n");
                    fgets(scan_string, sizeof(scan_string), stdin);
                    index = atoi(scan_string);
                    ret = spp_conncet_device(index);
                    if(ret < 0)
                    {
                        printf("spp_conncet_device fail!\n");
                    }
                    else
                    {
                        printf("spp_conncet_device success!\n");
                    } 
                }
                break;
            case BT_DISCONNECT_COMMAND:
                {
                    ret = spp_disconncet_device();
                    if(ret < 0)
                    {
                        printf("spp_disconncet_device fail!\n");
                    }
                    else
                    {
                        printf("spp_disconncet_device success!\n");
                    }                     
                }
                break;
            case BT_SPP_SEND_COMMAND:
                {
                    uint8_t data[2048] ={0};
                    uint16_t data_len;
                    printf("Please input the msg to send:");
                    if (fgets((char*)data, sizeof(data), stdin) != NULL)
                    {
                        data_len = strlen((char*)data) - 1;    //ret = spp_send_data(data, data_len);
                        if(ret < 0)
                        {
                            printf("spp_send_data fail!\r\n");
                        }
                        else
                        {
                            printf("spp_send_data success!\r\n");
                        }
                    }
                }
                break;
            case BT_SERVER_STATUS_COMMAND:
                {
                    int isActived;
                    ret = bt_get_server_status(&isActived);
                    if(ret < 0)
                    {
                        printf("bt_get_server_status fail!\n");
                    }
                    else
                    {
                        printf("spp server isActived =%d\n", isActived);
                    }
                }
                break;
            case BT_CONNECT_STATUS_COMMAND:
                {
                    int isConnected;
                    ret = bt_get_connect_status(&isConnected);
                    if(ret < 0)
                    {
                        printf("bt_get_connect_status fail!\n");
                    }
                    else
                    {
                        printf("spp  isConnected=%d\n", isConnected);
                    }
                }
                break;
#endif
        case 2:
        {
            int bRegister;
            printf("\r\n please select cmd: (1: register 0: unregister) >");
            fgets(scan_string, sizeof(scan_string), stdin);
            bRegister = atoi(scan_string);
            ret = bt_gatt_register(bRegister);
            if (ret < 0)
            {
                printf("bt gatt register fail!\r\n");
            }
            else
            {
                printf("bt gatt register success!\r\n");
            }
        }
        break;
        case BT_GATT_ADVERTISE_COMMAND:
            //bt_gatt_advertise(1);
            break;
        case 3:
            ret = bt_gatt_createdatabase(1);
            if (ret < 0)
            {
                printf("create database fail!\r\n");
            }
            else
            {
                printf("create database success!\r\n");
            }
            break;
        case 4:
        {
            int uuid, len;
            unsigned char uuid128[16] = {0};
            char uuidtmp[2] = {0};
            printf("\r\n please select character uuid(hex,2bytes or 16bytes):>");
            fgets(scan_string, sizeof(scan_string), stdin);
            len = strlen(scan_string) - 1;
            if (len == 4)
            {
                uuid = strtoul(scan_string, NULL, 16);
            }
            else if (len == 32)
            {
                printf("len=32\n");
                for (i = 0; i < 16; i++)
                {
                    strncpy(uuidtmp, scan_string + 2 * i, 2);
                    uuid128[15 - i] = strtoul(uuidtmp, NULL, 16);
                }
            }
            else
            {
                printf("the length is incorrect!\r\n");
                continue;
            }
            if (len == 4)
            {
                ret = bt_gatt_create_service_uuid16(uuid);
            }
            else
            {
                ret = bt_gatt_create_service_uuid128(uuid128);
            }
            if (ret < 0)
            {
                printf("bt gatt create service fail!\r\n");
            }
            else
            {
                printf("bt gatt create service success!\r\n");
            }
        }
        break;
        case 5:
        {
            int pro, permission, uuid, len;
            int pro_tmp, permission_tmp;
            uint32_t attrHandle;
            unsigned char uuid128[16];
            printf("\r\n please select character uuid(hex,2bytes or 16bytes):>");
            fgets(scan_string, sizeof(scan_string), stdin);
            len = strlen(scan_string) - 1;
            if (len == 4)
            {
                uuid = strtoul(scan_string, NULL, 16);
            }
            else if (len == 32)
            {
                char uuidtmp[2];
                for (i = 0; i < 16; i++)
                {
                    strncpy(uuidtmp, scan_string + 2 * i, 2);
                    uuid128[15 - i] = strtoul(uuidtmp, NULL, 16);
                }
            }
            else
            {
                continue;
            }
#if 0
                    printf("\r\n please select character property: \n(1:broadcast,2:read,4:write without respose8:write,16:notify,32:indicate,64:signed write,128:extended propertiers) >");
                    fgets(scan_string, sizeof(scan_string), stdin);
                    pro = atoi(scan_string);
                    printf("\r\n please select character permission:\n (0:none,16:read,32:write,64:read encryption,128:read auth) >");
                    fgets(scan_string, sizeof(scan_string), stdin);
                    permission = atoi(scan_string);
#else
            pro = 0;
            printf("\n please select character property:broadcast (0:  disable    1: enable)>");
            fgets(scan_string, sizeof(scan_string), stdin);
            pro_tmp = atoi(scan_string);
            if (pro_tmp)
            {
                pro = pro | 0x01;
            }

            printf("\n please select character property:read (0:  disable    1: enable)>");
            fgets(scan_string, sizeof(scan_string), stdin);
            pro_tmp = atoi(scan_string);
            if (pro_tmp)
            {
                pro = pro | 0x02;
            }

            printf("\n please select character property:write without respose (0:  disable    1: enable)>");
            fgets(scan_string, sizeof(scan_string), stdin);
            pro_tmp = atoi(scan_string);
            if (pro_tmp)
            {
                pro = pro | 0x04;
            }

            printf("\n please select character property:write (0:  disable    1: enable)>");
            fgets(scan_string, sizeof(scan_string), stdin);
            pro_tmp = atoi(scan_string);
            if (pro_tmp)
            {
                pro = pro | 0x08;
            }

            printf("\n please select character property:notify (0:  disable    1: enable)>");
            fgets(scan_string, sizeof(scan_string), stdin);
            pro_tmp = atoi(scan_string);
            if (pro_tmp)
            {
                pro = pro | 0x10;
            }

            printf("\n please select character property:indicate (0:  disable    1: enable)>");
            fgets(scan_string, sizeof(scan_string), stdin);
            pro_tmp = atoi(scan_string);
            if (pro_tmp)
            {
                pro = pro | 0x20;
            }

            permission_tmp = 0;
            // printf("\r\n please select character permission:\n (0:none,16:read,32:write,64:read encryption,128:read auth) >");
            printf("\n please select permission : (0: none  1: read  2: write  3: read and write)>");
            fgets(scan_string, sizeof(scan_string), stdin);
            permission_tmp = atoi(scan_string);
            if (permission_tmp == 0)
            {
                permission = 0;
            }
            else if (permission_tmp == 1)
            {
                permission = 0x10;
            }
            else if (permission_tmp == 2)
            {
                permission = 0x20;
            }
            else if (permission_tmp == 3)
            {
                permission = 0x10 | 0x20;
            }
#endif
            if (len == 4)
            {
                ret = bt_gatt_create_characteristic_uuid16(uuid, pro, permission, &attrHandle);
            }
            else
            {
                ret = bt_gatt_create_characteristic_uuid128(uuid128, pro, permission, &attrHandle);
            }

            if (ret < 0)
            {
                printf("bt gatt create characteristic fail!\r\n");
            }
            else
            {
                printf("bt gatt create characteristic success! attrHandle = %d.\n", attrHandle);
            }
        }
        break;
        case 6:
            ret = bt_gatt_create_descriptor();
            if (ret < 0)
            {
                printf("bt gatt create descriptor fail!\r\n");
            }
            else
            {
                printf("bt gatt create descriptor success!\r\n");
            }

            break;
        case 7:
            ret = bt_gatt_add_service_2_db();
            if (ret < 0)
            {
                printf("bt gatt add service to database fail!\r\n");
            }
            else
            {
                printf("bt gatt add service to database success!\r\n");
            }

            break;
        case 8:
            ret = bt_gatt_active(1);
            if (ret < 0)
            {
                printf("bt gatt active fail!\r\n");
            }
            else
            {
                printf("bt gatt active success!\r\n");
            }
            break;
        case 9:
        {
            int attrhandle;
            char data[50];
            printf("\r\n please input attrhandle: >");
            fgets(scan_string, sizeof(scan_string), stdin);
            attrhandle = atoi(scan_string);
            printf("\r\n please input data to be notified: >");
            memset(data, 0, sizeof(data));
            fgets(data, sizeof(data), stdin);
            ret = bt_gatt_notification(attrhandle, data);
            if (ret < 0)
            {
                printf("bt gatt notification fail!\r\n");
            }
            else
            {
                printf("bt gatt notification success!\r\n");
            }
        }
        break;
        case 10:
        {
            int attrhandle;
            char data[50];
            printf("\r\n please input attrhandle: >");
            fgets(scan_string, sizeof(scan_string), stdin);
            attrhandle = atoi(scan_string);
            printf("\r\n please input data to be notified: >");
            memset(data, 0, sizeof(data));
            fgets(data, sizeof(data), stdin);
            ret = bt_gatt_indication(attrhandle, data);
            if (ret < 0)
            {
                printf("bt gatt indication fail!\r\n");
            }
            else
            {
                printf("bt gatt indication success!\r\n");
            }
        }
        break;
        case 11:
        {
            //int inquiry_scan_status;
            //int page_scan_status;
            ret = bt_get_scan_enable(&inquiry_scan_status, &page_scan_status);
            if (ret < 0)
            {
                printf("bt get scan enable fail!\r\n");
            }
            else
            {
                printf("inquiry scan status = %d,  page_scan_status = %d.\n", inquiry_scan_status, page_scan_status);
            }
        }
        break;
        case 12:
            //int inquiry_scan_status;
            //int page_scan_status;
            printf("\r\n please input inquiry scan status: 0 off  1 on >");
            fgets(scan_string, sizeof(scan_string), stdin);
            inquiry_scan_status = atoi(scan_string);

            printf("\r\n please input page scan status: 0 off  1 on >");
            fgets(scan_string, sizeof(scan_string), stdin);
            page_scan_status = atoi(scan_string);

            bt_set_scan_enable(inquiry_scan_status, page_scan_status);
            if (ret < 0)
            {
                printf("bt set scan enable fail!\r\n");
            }
            else
            {
                printf("bt set scan enable success!\r\n");
            }
            break;
        case 13:
        {
            int cmd;
            char devicename[128] = {0};
            SIMCOM_BT_ADDR_T addr;
            printf("\r\n get hostname mac (0) set hostname(1):");
            fgets(scan_string, sizeof(scan_string), stdin);
            cmd = atoi(scan_string);
            if (cmd == 1)
            {
                printf("please input new bt name:\r\n");
                fgets(devicename, sizeof(devicename), stdin);
                if ((strlen(devicename) <= 1) || (strlen(devicename) > 64))
                {
                    printf("error: bt name length must be (1-63)\r\n");
                    break;
                }

                devicename[strlen(devicename) - 1] = '\0';

                ret = host_device(devicename);
                if (ret < 0)
                {
                    printf("set bluetooth name fail!\n");
                }
                else
                {
                    printf("set bluetooth name success!\n");
                }
            }
            else
            {
                host_device_addr(devicename, &addr);
                if (ret < 0)
                {
                    printf("get bluetooth name and mac fail!\n");
                }
                else
                {
                    printf("name=%s, %02X:%02X:%02X:%02X:%02X:%02X\r\n",
                           devicename,
                           (addr.nap >> 8) & 0xFF,
                           addr.nap & 0xFF,
                           addr.uap,
                           (addr.lap >> 16) & 0xFF,
                           (addr.lap >> 8) & 0xFF,
                           addr.lap & 0xFF);
                }
            }
        }
        break;
        case 14:
            printf("Currently not open for use.\r\n");
            break;
#if 0
            case BT_SET_INQUIRY_SCAN_COMMAND:
                {
                    int interval,window;
                    printf("\r\n please input interval: (18-4096 1=0.625ms even only) >");
                    fgets(scan_string, sizeof(scan_string), stdin);
                    interval = atoi(scan_string);
                    printf("\r\n please input window: (17-4096 1=0.625ms) >");
                    fgets(scan_string, sizeof(scan_string), stdin);
                    window = atoi(scan_string);
                    bt_ctrl->bt_set_inquiry_scan(interval,window);  
                }
                break;
#endif
        case 99:
            break;

        default:
            break;
        }

        if (opt == 99)
            break;
    }
}
#endif
#endif
#ifndef _SIMCOM_WINDOWS_
static void simcom_timer_test()
{
    const char *options_list[] =
        {
            "1. set timer interval",
            "2. create timer",
            "99. back",
        };

    int opt = 0;
    int i, j;
    char scan_string[100] = {0};

    set_freq_nanosecs(1000000000); //default 1s

    while (1)
    {
        printf("\nPlease select an option to test from the items listed below.\n");
        print_option_menu(options_list, sizeof(options_list) / sizeof(options_list[0]));
        printf("Option > ");

        memset(scan_string, 0, sizeof(scan_string));
        if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
            continue;

        /* Convert the option to an integer, and switch on the option entered. */
        opt = atoi(scan_string);
        switch (opt)
        {
        case 1:
        {
            printf("\r\n please input timer interval(ns): >");
            fgets(scan_string, sizeof(scan_string), stdin);
            set_freq_nanosecs(atoll(scan_string));
        }
        break;
        case 2:
        {
            create_timer();
            char cmd;
            sigset_t mask;
            while (1)
            {
                scanf("%c", &cmd);
                switch (cmd)
                {
                case '1':
                    /* Block timer signal temporarily */

                    printf("Blocking signal %d\n", SIG);
                    sigemptyset(&mask);
                    sigaddset(&mask, SIG);
                    if (sigprocmask(SIG_SETMASK, &mask, NULL) == -1)
                    {
                        printf("sigprocmask fail!\n");
                    }
                    break;
                case '2':
                    /* Unlock the timer signal, so that timer notification
                                can be delivered */

                    printf("Unblocking signal %d\n", SIG);
                    if (sigprocmask(SIG_UNBLOCK, &mask, NULL) == -1)
                    {
                        printf("sigprocmask fail!\n");
                    }
                    break;
                default:
                    break;
                }
                if (cmd == 'q')
                {
                    destroy_timer();
                    scanf("%c", &cmd);
                    break;
                }
            }
        }
        case 99:
            break;
        default:
            break;
        }

        if (opt == 99)
        {
            break;
        }
    }
}
#else
timer_t timerID_1000 = 0;
timer_t timerID_10000 = 0;

void *app_timer_ind_cb_fcn_1000(union sigval v)
{
    DEBUGLOG("app_timer_ind_cb_fcn_1000()[%d][%d]\n", v.sival_int, timerID_1000);
}
void *app_timer_ind_cb_fcn_10000(union sigval v)
{
    DEBUGLOG("app_timer_ind_cb_fcn_10000()[%d][%d]\n", v.sival_int, timerID_10000);
    app_stopTimer(timerID_10000);
    timerID_10000 = 0;
}

static void simcom_timer_test()
{
    const char *options_list[] =
        {
            "1. create 1  second  timer",
            "2. stop   1  second  timer",
            "3. create 10 seconds timer",
            "99. back",
        };

    app_timer_init();

    int opt = 0;
    int i, j;
    char scan_string[100] = {0};

    while (1)
    {
        printf("\nPlease select an option to test from the items listed below.\n");
        print_option_menu(options_list, sizeof(options_list) / sizeof(options_list[0]));
        printf("Option > ");

        memset(scan_string, 0, sizeof(scan_string));
        if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
        {
            continue;
        }

        /* Convert the option to an integer, and switch on the option entered. */
        opt = atoi(scan_string);
        switch (opt)
        {
        case 1:
        {
            if (timerID_1000 == 0)
                timerID_1000 = app_startTimer(1000, (timer_ind_cb_fcn)app_timer_ind_cb_fcn_1000);
        }
        break;
        case 2:
        {
            if (timerID_1000 != 0)
            {
                app_stopTimer(timerID_1000);
                timerID_1000 = 0;
            }
        }
        break;
        case 3:
        {
            if (timerID_10000 == 0)
                timerID_10000 = app_startTimer(10000, (timer_ind_cb_fcn)app_timer_ind_cb_fcn_10000);
        }
        case 99:
            break;
        default:
            break;
        }

        if (opt == 99)
        {
            timerID_1000 = 0;
            timerID_10000 = 0;
            break;
        }
    }

    app_timer_deinit();
}
#endif

extern int TCPTest(char *serverIP, int serverPort, uint8_t *send_data, uint32_t send_len);
extern int app_udp_send_data(char *serverIP, int serverPort, uint8_t *send_data, uint32_t send_len);
static void simcom_tcp_test()
{
    char ip[20] = {0};
    int port;
    const char *options_list[] =
        {
            "1. send and recv tcp data",
            "2. send and recv udp data",
            "99. back",
        };

    int ret;
    int i;
    int opt = 0;
    char ch;
    char scan_string[1024 + 4]={0};
    char send_string[1024 + 4]={0};

    while (1)
    {
        printf("\nPlease select an option to test from the items listed below.\n");
        print_option_menu(options_list, sizeof(options_list) / sizeof(options_list[0]));
        printf("Option > ");

        memset(scan_string, 0, sizeof(scan_string));
        if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
        {
            continue;
        }
        /* Convert the option to an integer, and switch on the option entered. */
        opt = atoi(scan_string);
        switch (opt)
        {
        case 1:
        {
            printf("\nPlease input server ip>");
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
            {
                continue;
            }
            strncpy(ip, scan_string, (sizeof(ip) - 1));

            for (i = 0; i < strlen(ip) - 1; i++)
            {
                if (!isdigit(ip[i]) && ip[i] != '.')
                {
                    printf("invalid ip,i=%d,len=%d\n", i, strlen(ip));
                    break;
                }
            }
            printf("\nPlease input server port>");
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
            {
                continue;
            }
            port = atoi(scan_string);

            printf("\nPlease input send data(Maximum 1024)>");
            memset(scan_string, 0, sizeof(scan_string));
            memset(send_string, 0, sizeof(send_string));

            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
            {
                continue;
            }

            if (strlen(scan_string) >= 1027 )
            {
                while((ch = getchar()) != '\n' && ch != EOF)
                {
                    ;
                }
                printf("the intput data is too long.\n");
             }
            else
            {
                int nret = simcom_handle_input_string(scan_string, strlen(scan_string), send_string, 1024, 0);
                if (nret == 0)
                {
                    TCPTest(ip, port, (uint8_t *)send_string, strlen(send_string));
                }
                else if (nret == 1)
                {
                    printf("the intput data is too long.\n");
                }
                else
                {
                    printf("intput error!\n");
                }
            }
        }
        break;
        case 2:
        {
            printf("\nPlease input server ip>");
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
            {
                continue;
            }

            strncpy(ip, scan_string, (sizeof(ip) - 1));

            for (i = 0; i < strlen(ip) - 1; i++)
            {
                if (!isdigit(ip[i]) && ip[i] != '.')
                {
                    printf("invalid ip,i=%d,len=%d\n", i, strlen(ip));
                    break;
                }
            }
            printf("\nPlease input server port>");
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
            {
                continue;
            }

            port = atoi(scan_string);

            printf("\nPlease input send data(Maximum 1024)>");
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
            {
                continue;
            }

            memset(send_string, 0, sizeof(send_string));
            int nret = simcom_handle_input_string(scan_string, strlen(scan_string), send_string, 1024, 0);
            if (nret == 0)
            {
                app_udp_send_data(ip, port, (uint8_t *)send_string, strlen(send_string));
            }
            else if (nret == 1)
            {
                printf("intput data is too long.\n");
            }
            else
            {
                printf("intput error!\n");
            }
        }
        break;
        default:
            break;
        }

        if (opt == 99)
        {
            break;
        }
    }
}

#ifdef FEATURE_DEMO_UIM
static void simcom_uim_test()
{
    SimCard_Status_type sim_status;
    const char *options_list[] =
        {
            "1. get simcard Info",
            "2. get iccid",
            "3. get imsi",
            "4. enable pin",
            "5. disable pin",
            "6. enter pin",
            "7. enter puk",
            "8. change pin",
            "99. back",
        };

    int ret;
    int opt = 0;
    int i, j;
    char scan_string[100] = {0};

    if (uim_auto_report_init((uim_ind_cb_fcn)process_simcom_ind_message) != 0)
    {
        return;
    }

    while (1)
    {
        printf("\nPlease select an option to test from the items listed below.\n");
        print_option_menu(options_list, sizeof(options_list) / sizeof(options_list[0]));
        printf("Option > ");

        memset(scan_string, 0, sizeof(scan_string));
        if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
        {
            continue;
        }

        /* Convert the option to an integer, and switch on the option entered. */
        opt = atoi(scan_string);
        switch (opt)
        {
        case 1:
        {
            /* get sim card status*/

            memset(&sim_status, 0, sizeof(SimCard_Status_type));
            ret = getSimCardStatus(&sim_status);

            printf("card_status:%d   (0:not exist 1: Exist: 2: UNKNOW)\n", sim_status.card_status);
            printf("app_type:   %d   (0:UNKNOWN 1: SIM 2: USIM 3: RUIM 4: CSIM 5: ISIM)\n", sim_status.app_type);

            printf("\napp_state define: (0:unknown  1: detected      2: pin block          3: puk block)\n");
            printf("app_state define: (4: person check  5:Permanently blocked 6: illgeal 7: Ready)\n");
            printf("app_state:  %d   \n\n", sim_status.app_state);

            printf("\npin_state define: (0: unknown  1: Enabled and not verified 2: Enabled and verified)\n");
            printf("pin_state define: (3: Disabled 4: Blocked                  5:Permanently blocked)\n");
            printf("pin_state:  %d   \n\n", sim_status.pin.pin_state);

            printf("pin_retries:%d\n", sim_status.pin.pin_retries);
            printf("puk_retries:%d\n", sim_status.pin.puk_retries);
        }
        break;
        case 2:
        {
            char iccid[21] = {0};
            ret = get_iccid(iccid);
            if (ret != 0)
            {
                printf("get iccid fail!\n");
                break;
            }
            printf("iccid:%s\n", iccid);
        }
        break;
        case 3:
        {
            char imsi[33] = {0};
            ret = get_imsi(imsi);
            if (ret != 0)
            {
                printf("get imsi fail!\n");
                break;
            }
            printf("imsi:%s\n", imsi);
        }
        break;

        case 4:
        {
            int retries_remaining = 0;
            char pin_code[16] = {0};
        
            printf("\nPlease input pin code,enter 4-8 digits>");            
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
            {
                continue;
            }

            if ((strlen(scan_string) - 1) < 4 || (strlen(scan_string) - 1) > 8)
            {
                printf("pin code length error\n");
                continue;
            }

            memcpy(pin_code, scan_string, strlen(scan_string) - 1);
            
            ret = enable_pin(pin_code, &retries_remaining);
            printf("ret=%d,retries_remaining=%d\n", ret, retries_remaining);
        }
        break;

        case 5:
        {
            int retries_remaining = 0;
            char pin_code[16] = {0};
        
            printf("\nPlease input pin code,enter 4-8 digits>");            
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
            {
                continue;
            }

            if ((strlen(scan_string) - 1) < 4 || (strlen(scan_string) - 1) > 8)
            {
                printf("pin code length error\n");
                continue;
            }

            memcpy(pin_code, scan_string, strlen(scan_string) - 1);
            
            ret = disable_pin(pin_code, &retries_remaining);
            printf("ret=%d,retries_remaining=%d\n", ret, retries_remaining);
        }
        break;

        case 6:
        {
            int retries_remaining = 0;
            char pin_code[16] = {0};
        
            printf("\nPlease input pin code,enter 4-8 digits>");            
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
            {
                continue;
            }

            if ((strlen(scan_string) - 1) < 4 || (strlen(scan_string) - 1) > 8)
            {
                printf("pin code length error\n");
                continue;
            }

            memcpy(pin_code, scan_string, strlen(scan_string) - 1);
            
            ret = enter_sim_pin(pin_code, &retries_remaining);
            printf("ret=%d,retries_remaining=%d\n", ret, retries_remaining);
        }
        break;

        case 7:
        {
            int retries_remaining = 0;
            char puk_code[16] = {0};
            char new_pin_code[16] = {0};

            //input puk code
            printf("\nPlease input puk code,enter 8 digits>");            
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
            {
                continue;
            }

            if ((strlen(scan_string) - 1) != 8)
            {
                printf("puk code length error\n");
                continue;
            }
            memcpy(puk_code, scan_string, strlen(scan_string) - 1);

            //input new pin code
            printf("\nPlease input new pin code,enter 4-8 digits>");            
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
            {
                continue;
            }
            if ((strlen(scan_string) - 1) < 4 || (strlen(scan_string) - 1) > 8)
            {
                printf("pin code length error\n");
                continue;
            }

            memcpy(new_pin_code, scan_string, strlen(scan_string) - 1);
            
            ret = enter_sim_puk(puk_code, new_pin_code, &retries_remaining);
            printf("ret=%d,retries_remaining=%d\n", ret, retries_remaining);
        }
        break;

        case 8:
        {
            int retries_remaining = 0;
            char pin_code[16] = {0};
            char new_pin_code[16] = {0};

            //input old pin code
            printf("\nPlease input pin code,enter 4-8 digits>");            
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
            {
                continue;
            }

            if ((strlen(scan_string) - 1) < 4 || (strlen(scan_string) - 1) > 8)
            {
                printf("pin code length error\n");
                continue;
            }
            memcpy(pin_code, scan_string, strlen(scan_string) - 1);

            //input new pin code
            printf("\nPlease input new pin code,enter 4-8 digits>");            
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
            {
                continue;
            }
            if ((strlen(scan_string) - 1) < 4 || (strlen(scan_string) - 1) > 8)
            {
                printf("pin code length error\n");
                continue;
            }
            memcpy(new_pin_code, scan_string, strlen(scan_string) - 1);
            
            ret = change_pin(pin_code, new_pin_code, &retries_remaining);
            printf("ret=%d,retries_remaining=%d\n", ret, retries_remaining);
        }
        break;        
        
        default:
            break;
        }

        if (opt == 99)
        {
            break;
        }
    }

    uim_auto_report_deinit();
}
#endif

#ifdef FEATURE_DEMO_DMS
static void simcom_dms_test()
{
    const char *options_list[] =
        {
            "1. get imei",
            "2. get rev_id",
            "3. set operate mode",
            "4. get operate mode",
            "5. get current system time",
            "99. back",
        };

    int ret;
    int opt = 0;
    int i, j;
    char scan_string[100] = {0};

    if (dms_init() != 0)
    {
        return;
    }

    while (1)
    {
        printf("\nPlease select an option to test from the items listed below.\n");
        print_option_menu(options_list, sizeof(options_list) / sizeof(options_list[0]));
        printf("Option > ");

        memset(scan_string, 0, sizeof(scan_string));
        if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
        {
            continue;
        }

        /* Convert the option to an integer, and switch on the option entered. */
        opt = atoi(scan_string);
        switch (opt)
        {
        case 1:
        {
            /*IMEI*/
            char imei[33] = {0};
            ret = get_imei(imei);
            if (ret != 0)
            {
                printf("get imei fail!\n");
                break;
            }
            printf("IMEI:%s\n", imei);
        }
        break;

        case 2:
        {
            /* rev_id*/
            char revid[150] = {0};
            ret = get_rev_id(revid);
            if (ret != 0)
            {
                printf("get rev_id fail!\n");
                break;
            }
            printf("rev_id:%s\n", revid);
        }
        break;

        case 3:
        {
            /* operating_mode*/
            char mode = 0;
            printf("\nPlease input mode value\n");
            printf(" 0 Online      (AT+CFUN=1) \n");
            printf(" 1 Low power   (AT+CFUN=0) \n");
            printf(" 2 Factory Test mode\n 3 Persistent low power\n 4 Resetting\n 5 Shutting down\n 6 OffLine\n");

            printf("Option > ");
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
            {
                continue;
            }

            /* Convert the option to an integer, and switch on the option entered. */
            mode = atoi(scan_string);
            printf("mode > %d", mode);
            ret = dms_set_operating_mode(mode);
            if (ret != 0)
            {
                printf("set operate mode fail!\n");
                break;
            }
        }
        break;
        case 4:
        {
            /* operating_mode*/
            char mode = 0;

            ret = dms_get_operating_mode(&mode);
            if (ret != 0)
            {
                printf("get operate mode fail!\n");
                break;
            }
            printf("operate mode: %d\n", mode);
        }
        break;

        case 5:
        {
            /* get current system time*/
            uint64_t msecs;

            ret = time_get_modem_cclk(&msecs);
            if (ret != 0)
            {
                printf("get current system time fail!\n");
                break;
            }
            printf("current system msecs: %lld\n", msecs);
            printf("current system second: %lld\n", msecs/1000);
            printf("current system time usec: %lld\n", msecs%1000);
        }
        
        default:
            break;
        }

        if (opt == 99)
        {
            break;
        }
    }

    dms_deinit();
}
#endif

static void simcom_get_version()
{
    const char *options_list[] = {
        "1. get sdk lib version",
        "2. get demo version",
        "99. back",
    };

    int ret;
    int opt = 0;
    int i, j;
    char scan_string[100] = {0};

    while (1)
    {
        printf("\nPlease select an option to test from the items listed below.\n");
        print_option_menu(options_list, sizeof(options_list) / sizeof(options_list[0]));
        printf("Option > ");

        memset(scan_string, 0, sizeof(scan_string));
        if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
            continue;

        /* Convert the option to an integer, and switch on the option entered. */
        opt = atoi(scan_string);
        switch (opt)
        {
        case 1:
        {
            char *sdk_version;
            sdk_version = (char *)get_simcom_sdk_version();

            printf("sdk lib version: %s\n", sdk_version);
        }
        break;
        case 2:
        {
            char *demo_version;
            demo_version = (char *)get_simcom_demo_version();

            printf("demo version: %s\n", demo_version);
        }
        break;
        default:
            break;
        }

        if (opt == 99)
        {
            break;
        }
    }
}


#ifdef FEATURE_DEMO_FTP
static char ftp_dl_save_path[128];
static int ftp_dl_save_handle;
static long long recv_len = 0;
void process_ftp_dl_recv_data(void *data, int data_len, int index, long long total_size)
{
    int write_len;

    if (index == 1 && strlen(ftp_dl_save_path) > 0)
    {
        ftp_dl_save_handle = open(ftp_dl_save_path, O_WRONLY | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE);
        recv_len = data_len;
    }
    if (ftp_dl_save_handle < 0)
    {
        return;
    }

    if (total_size == 0)
    {
        ftp_dl_save_handle = open(ftp_dl_save_path, O_WRONLY | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE);
        recv_len = data_len;
    }

    if (index >= 1)
    {
        recv_len += data_len;
        printf("\r\033[k");
        printf("pack index: %d,  recv = %lld/%lld  %%%lld", index, recv_len, total_size, recv_len * 100 / total_size);
    }

    if (data_len > 0)
    {
        write_len = write(ftp_dl_save_handle, data, data_len);
    }

    if (index == 0 || data_len == 0) // recv end
    {
        close(ftp_dl_save_handle);
        printf("\nclose write file handle.\n");
    }
}

static void simcom_ftp_test()
{
    const char *options_list[] = {
        "1. Login",
        "2. Print work directory",
        "3. Change work directory",
        "4. Mkdir",
        "5. Rename or move",
        "6. Delete file",
        "7. Delete folder",
        "8. Get file size",
        "9. download file",
        "10. upload file",
        "11  Logout",
        "99. back",
    };

    int i;
    int ret;
    int ctrl_sock = -1;
    int bLoginSuccess = 0;
    int opt = 0;
    char ip[20] = {0};
    char scan_string[256] = {0};
    char command[128] = {0};
    int tmp_val;

    char host[128] = {0};
    char user[64] = {0};
    char password[64] = {0};

    while (1)
    {

        printf("\nPlease select an option to test from the items listed below.\n");
        print_option_menu(options_list, sizeof(options_list) / sizeof(options_list[0]));
        printf("Option > ");

        memset(scan_string, 0, sizeof(scan_string));
        if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
            continue;

        /* Convert the option to an integer, and switch on the option entered. */
        opt = atoi(scan_string);

        switch (opt)
        {
        case 1:
            {
                printf("please input IP address\n");
                printf("For example:xx.xx.xx.xx\n");
                printf("please input>");
                memset(host, 0, sizeof(host));
                if(fgets(host, sizeof(host), stdin)==NULL)
                {
                    continue;
                }

                printf("please input user name\n");
                printf("For example:xxx\n");
                printf("please input>");
                memset(user, 0, sizeof(user));
                if(fgets(user, sizeof(user), stdin)==NULL)
                {
                    continue;
                }

                printf("please input password\n");
                printf("For example:**********\n");
                printf("please input>");
                memset(password, 0, sizeof(password));
                if(fgets(password, sizeof(password), stdin)==NULL)
                {
                    continue;
                }

                ret = ftp_login(host, 21, user, password);
                if (ret == -1)
                {
                    printf("Login faild!\n");
                    break;
                }
            }
            break;
        case 2: //Print work directory
            {
                char path[256] = {0};

                ret = ftp_pwd(path);
                if (ret != 0)
                {
                    printf("Print work directory fail!\n");
                    break;
                }
                printf("Print work directory success!\n");
            }
            break;
        case 3: //change work dir
            {
                char path[1024] = {0};
                unsigned long long data_len;

                printf("\nPlease input directory\n");
                printf("  /  ------> prefix means absolute path, or else relative path\n");
                printf("For example:  /test    test    ../test \n");
                printf("please input>");
                memset(scan_string, 0, sizeof(scan_string));
                if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                continue;
                memcpy(path, scan_string, strlen(scan_string) - 1);

                ret = ftp_cwd(path);
                if (ret != 0)
                {
                    printf("change work directory fail!\n");
                }
                else
                {
                    printf("change work directory success!\n");
                }
            }
            break;
        case 4: //mkdir
            {
                char path[128] = {0};
                unsigned long long data_len;

                printf("Tips: input size less than 128!\n");
                printf("please input>");
                memset(scan_string, 0, sizeof(scan_string));
                if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                continue;

                ret = simcom_handle_input_string(scan_string, strlen(scan_string), path, 128, 1);
                if(0 != ret)
                {
                    if(-1 == ret)
                    {
                        printf("input length too short!\n");
                    }
                    else if(1 == ret)
                    {
                        printf("input length too long!\n");
                    }
                    else if(2 == ret)
                    {
                        printf("add \\n failed!\n");
                    }
                }
                
                ret = ftp_mkdir(path);
                if (ret != 0)
                {
                    printf("create fail!\n");
                }
                else
                {
                    printf("create success!\n");
                }
            }
            break;
        case 5: //rename move
            {
                char s_path[1024] = {0};
                char d_path[1024] = {0};
                unsigned long long data_len;

                printf("\nPlease input source directory or filename\n");
                printf("  /  ------> prefix means absolute path, or else relative path\n");
                printf("For example:  /test    test   test/test.ini  ../test \n");
                printf("please input>");
                memset(scan_string, 0, sizeof(scan_string));
                if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                    continue;
                memcpy(s_path, scan_string, strlen(scan_string) - 1);
                printf("\nPlease input destination directory or filename\n");
				printf("Tips: input size less than 128!\n");
                printf("please input>");
                memset(scan_string, 0, sizeof(scan_string));
                if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                    continue;
                //memcpy(d_path, scan_string, strlen(scan_string) - 1);
                ret = simcom_handle_input_string(scan_string, strlen(scan_string), d_path, 128, 1);
                if(0 != ret)
                {
                    if(-1 == ret)
                    {
                        printf("input length too short!\n");
                    }
                    else if(1 == ret)
                    {
                        printf("input length too long!\n");
                    }
                    else if(2 == ret)
                    {
                        printf("add \\n failed!\n");
                    }
                }

                ret = ftp_mv(s_path, d_path);
                if (ret != 0)
                {
                    printf("rename or move fail!\n");
                }
                else
                {
                    printf("rename or move success!\n");
                }
            }
            break;
        case 6: //delete file
            {
                char path[1024] = {0};
                unsigned long long data_len;

                printf("\nPlease input file\n");
                printf("  /  ------> prefix means absolute path, or else relative path\n");
                printf("For example:  /test/test.ini   test.ini  ../test/test.ini \n");
                printf("please input>");
                memset(scan_string, 0, sizeof(scan_string));
                if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                    continue;
                memcpy(path, scan_string, strlen(scan_string) - 1);

                ret = ftp_deltefile(path);
                if (ret != 0)
                {
                    printf("delete file fail!\n");
                }
                else
                {
                    printf("delete file success!\n");
                }
            }
            break;
        case 7: //ftp_delete folder
            {
                char path[1024] = {0};
                unsigned long long data_len;

                printf("\nPlease input file\n");
                printf("  /  ------> prefix means absolute path, or else relative path\n");
                printf("For example:  /test  test  ../test/test.ini \n");
                printf("please input>");
                memset(scan_string, 0, sizeof(scan_string));
                if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                    continue;
                memcpy(path, scan_string, strlen(scan_string) - 1);

                ret = ftp_deletefolder(path);
                if (ret != 0)
                {
                    printf("delete folder fail!\n");
                }
                else
                {
                    printf("delete folder success!\n");
                }
            }
            break;
        case 8: // get file size
            {
                char path[1024] = {0};
                long long size;

                printf("\nPlease input file path\n");
                printf("  /  ------> prefix means absolute path, or else relative path\n");
                printf("For example:  /test/test.ini    test.ini\n");
                printf("please input>");
                memset(scan_string, 0, sizeof(scan_string));
                if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                    continue;
                memcpy(path, scan_string, strlen(scan_string) - 1);

                ret = ftp_size(path, &size);
                if (ret != 0)
                {
                    printf("get size fail!\n");
                }
                else
                {
                    printf("size: %lld!\n", size);
                }
            }
            break;
        case 9: //download file
            {
                unsigned long long stor_size;
                char *p;
                char path[1024] = {0};

                printf("\nPlease input file path\n");
                printf("  /  ------> prefix means absolute path, or else relative path\n");
                printf("For example:  /test/test.ini    test.ini\n");
                printf("please input>");
                memset(scan_string, 0, sizeof(scan_string));
                if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                    continue;
                memcpy(path, scan_string, strlen(scan_string) - 1);
                p = strrchr(path, '/');
                if (p == NULL)
                {
                    sprintf(ftp_dl_save_path, "/data/%s", path);
                }
                else
                {
                    if (strlen(p + 1) > 0)
                    {
                        sprintf(ftp_dl_save_path, "/data/%s", p + 1);
                    }
                    else
                    {
                        sprintf(ftp_dl_save_path, "/data/%s", "unkonw_name");
                    }
                }

                ret = ftp_download(path, process_ftp_dl_recv_data);
                if (ret != 0)
                {
                    printf("download file fail!\n");
                }
                else
                {
                    printf("download file success!\n");
                }
            }
            break;
        case 10: //upload file
            {
                unsigned long long stor_size;
                char *p;
                char path[256] = {0};
                char local_path[256] = {0};

                printf("please input local file path(For example: /data/test.ini)>");
                memset(scan_string, 0, sizeof(scan_string));
                if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                    continue;
                memcpy(local_path, scan_string, strlen(scan_string) - 1);
                printf("\nPlease input file path\n");
                printf("  /  ------> prefix means absolute path, or else relative path\n");
                printf("For example:  /test/test.ini    test.ini\n");
                printf("please input>");
                memset(scan_string, 0, sizeof(scan_string));
                if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                    continue;
                memcpy(path, scan_string, strlen(scan_string) - 1);

                p = strrchr(path, '/');

                ret = ftp_upload(local_path, path);

                if (ret != 0)
                {
                    printf("upload file fail!\n");
                }
                else
                {
                    printf("upload file success!\n");
                }
            }
            break;
        case 11: //Logout
            {
                ret = ftp_logout();
                if (ret == -1)
                {
                    printf("No login, please log in first and then log out \n");
                    break;
                }
                    printf("ftp logout success!\n");
                }
            break;

        case 99:
            break;
        default:
            continue;
        }
        if (opt == 99)
        {
            break;
        }
    }
}
#endif

#ifdef FEATURE_DEMO_SSL
static void simcom_ssl_test()
{
    const char *options_list[] = {
        "1. Start SSL Client oneway",
        "2. Start SSL Client twoway",
        "3. Start SSL Server oneway",
        "4. Start SSL Server twoway",
        "99. back",
    };
    int opt = 0;
    char scan_string[100] = {0};

    while (1)
    {
        printf("\nPlease select an option to test from the items listed below.\n");
        print_option_menu(options_list, sizeof(options_list) / sizeof(options_list[0]));
        printf("Option > ");

        memset(scan_string, 0, sizeof(scan_string));
        if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
            continue;

        /* Convert the option to an integer, and switch on the option entered. */
        opt = atoi(scan_string);
        switch (opt)
        {
        case 1:
            ssl_client_oneway();
            break;
        case 2:
            ssl_client_twoway();
            break;
        case 3:
            ssl_server_oneway();
            break;
        case 4:
            ssl_server_twoway();
            break;
        default:
            break;
        }
        if (opt == 99)
        {
            break;
        }
    }
}
#endif

#ifdef FEATURE_DEMO_HTTPS
static void simcom_http_s_test()
{
    const char *options_list[] = {
        "1. HTTP",
        "2. HTTPS",
        "3. two-way https",
        "99. back",
    };
    int opt = 0;
    char scan_string[100] = {0};
    char url[100] = {0};
    char rename[32] = {0};
    char cainfo[50] = {0};
    int ca_flag = 0;
	char client_crt[50] = {0};
    char client_key[50] = {0};

    while (1)
    {
        printf("\nPlease select an option to test from the items listed below.\n");
        print_option_menu(options_list, sizeof(options_list) / sizeof(options_list[0]));
        printf("Option > ");

        memset(scan_string, 0, sizeof(scan_string));
        if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
            continue;

        /* Convert the option to an integer, and switch on the option entered. */
        opt = atoi(scan_string);
        switch (opt)
        {
        case 1:
            printf("\nPlease Input the url:\n>");
            memset(url, 0, sizeof(url));
            if (fgets(url, sizeof(url), stdin) == NULL)
                continue;
            url[strlen(url) - 1] = 0;

            printf("\nPlease Input the rename less than 30:\n>");
            memset(rename, 0, sizeof(rename));
            if (fgets(rename, sizeof(rename), stdin) == NULL)
                continue;
            rename[strlen(rename) - 1] = 0;
            http_get(url, rename);
            break;
        case 2:
            printf("\nPlease Input the url:\n>");
            memset(url, 0, sizeof(url));
            if (fgets(url, sizeof(url), stdin) == NULL)
                continue;
            url[strlen(url) - 1] = 0;

            printf("\nPlease Input the rename less than 30:\n>");
            memset(rename, 0, sizeof(rename));
            if (fgets(rename, sizeof(rename), stdin) == NULL)
                continue;
            rename[strlen(rename) - 1] = 0;

            printf("\nPlease Input the ca_flag(1,enable, 0,disable):\n>");
            memset(scan_string, 0, sizeof(scan_string));
            if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                continue;
            ca_flag = atoi(scan_string);

            if (ca_flag)
            {
                printf("\nPlease Input the ca name:\n>");
                memset(cainfo, 0, sizeof(cainfo));
                if (fgets(cainfo, sizeof(cainfo), stdin) == NULL)
                    continue;
                cainfo[strlen(cainfo) - 1] = 0;
            }
            https_get(url, rename, ca_flag, cainfo);
            break;

		case 3:

            printf("\nPlease Input the url:\n>");
            memset(url, 0, sizeof(url));
            if (fgets(url, sizeof(url), stdin) == NULL)
                continue;
            url[strlen(url) - 1] = 0;

            printf("\nPlease Input the save filename less than 30:\n>");
            memset(rename, 0, sizeof(rename));
            if (fgets(rename, sizeof(rename), stdin) == NULL)
                continue;
            rename[strlen(rename) - 1] = 0;

            printf("\nPlease Input the ca name:\n>");
            memset(cainfo, 0, sizeof(cainfo));
            if (fgets(cainfo, sizeof(cainfo), stdin) == NULL)
                continue;
            cainfo[strlen(cainfo) - 1] = 0;

            printf("\nPlease Input the client crt name:\n>");
            memset(client_crt, 0, sizeof(client_crt));
            if (fgets(client_crt, sizeof(client_crt), stdin) == NULL)
                continue;
            client_crt[strlen(client_crt) - 1] = 0;

            printf("\nPlease Input the client key name:\n>");
            memset(client_key, 0, sizeof(client_key));
            if (fgets(client_key, sizeof(client_key), stdin) == NULL)
                continue;
            client_key[strlen(client_key) - 1] = 0;

            https_get_binary(url, rename, cainfo, client_crt, client_key);
            break;
        default:
            break;
        }
        if (opt == 99)
        {
            break;
        }
    }
}
#endif

#ifdef FEATURE_DEMO_FTPS
static void simcom_ftp_s_test()
{
    const char *options_list[] = {
        "1. FTP",
        "2. FTPS",
        "99. back",
    };
    int opt = 0;
    char scan_string[100] = {0};
    char url[100] = {0};
    char rename[32] = {0};
    char usr[50] = {0};
    char pwd[20] = {0};
    int ret = 0;

    while (1)
    {
        printf("\nPlease select an option to test from the items listed below.\n");
        print_option_menu(options_list, sizeof(options_list) / sizeof(options_list[0]));
        printf("Option > ");

        memset(scan_string, 0, sizeof(scan_string));
        if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
            continue;

        /* Convert the option to an integer, and switch on the option entered. */
        opt = atoi(scan_string);
        switch (opt)
        {
        case 1:
            printf("\nPlease Input the url:\n>");
            memset(url, 0, sizeof(url));
            if (fgets(url, sizeof(url), stdin) == NULL)
                continue;
            url[strlen(url) - 1] = 0;

            printf("\nPlease Input the rename less than 30:\n>");
            memset(rename, 0, sizeof(rename));
            if (fgets(rename, sizeof(rename), stdin) == NULL)
                continue;
            rename[strlen(rename) - 1] = 0;

            printf("\nPlease Input the usr:\n>");
            memset(usr, 0, sizeof(usr));
            if (fgets(usr, sizeof(usr), stdin) == NULL)
                continue;
            usr[strlen(usr) - 1] = 0;

            printf("\nPlease Input the pwd:\n>");
            memset(pwd, 0, sizeof(pwd));
            if (fgets(pwd, sizeof(pwd), stdin) == NULL)
                continue;
            pwd[strlen(pwd) - 1] = 0;
            ret = ftp_get(url, rename, usr, pwd);
            if (ret == -1)
            {
                printf("FTP failure!\n");
            }
            break;
        case 2:
            printf("\nPlease Input the url:\n>");
            memset(url, 0, sizeof(url));
            if (fgets(url, sizeof(url), stdin) == NULL)
                continue;
            url[strlen(url) - 1] = 0;

            printf("\nPlease Input the rename:\n>");
            memset(rename, 0, sizeof(rename));
            if (fgets(rename, sizeof(rename), stdin) == NULL)
                continue;
            rename[strlen(rename) - 1] = 0;

            printf("\nPlease Input the usr:\n>");
            memset(usr, 0, sizeof(usr));
            if (fgets(usr, sizeof(usr), stdin) == NULL)
                continue;
            usr[strlen(usr) - 1] = 0;

            printf("\nPlease Input the pwd:\n>");
            memset(pwd, 0, sizeof(pwd));
            if (fgets(pwd, sizeof(pwd), stdin) == NULL)
                continue;
            pwd[strlen(pwd) - 1] = 0;
            ret = ftps_get(url, rename, usr, pwd);
            if (ret == -1)
            {
                printf("FTPS failure!\n");
            }
            break;
        default:
            break;
        }
        if (opt == 99)
        {
            break;
        }
    }
}
#endif

#ifdef FEATURE_DEMO_MQTTS
static void simcom_mqtt_s_test()
{
    const char *options_list[] = {
        "1. MQTT_SUB",
        "2. MQTT_PUB",
        "3. MQTTS_SUB",
        "4. MQTTS_PUB",
        "99. back",
    };
    int opt = 0;
    char scan_string[100] = {0};
    char host[100] = {0};
    char cafile[100] = {0};
    char certfile[100] = {0};
    char keyfile[100] = {0};
    int port = 0;

    while (1)
    {
        printf("\nPlease select an option to test from the items listed below.\n");
        print_option_menu(options_list, sizeof(options_list) / sizeof(options_list[0]));
        printf("Option > ");

        memset(scan_string, 0, sizeof(scan_string));
        if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
            continue;

        /* Convert the option to an integer, and switch on the option entered. */
        opt = atoi(scan_string);
        switch (opt)
        {
        case 1:
            while (1)
            {
                printf("Please input the server domain name or IP address!\n");
                memset(host, 0, sizeof(host));
                if (fgets(host, sizeof(scan_string), stdin) == NULL)
                {
                    continue;
                }
                else
                {
                    host[strlen(host)-1] = 0;
                    break;
                }
            }
            while (1)
            {
                printf("Please input the port!\n");
                memset(scan_string, 0, sizeof(scan_string));
                if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                {
                    continue;
                }
                else
                {
                    port = atoi(scan_string);
                    break;
                }
            }
            mqtt_sub(host, port);
            break;
        case 2:
            while (1)
            {
                printf("Please input the server domain name or IP address!\n");
                memset(host, 0, sizeof(host));
                if (fgets(host, sizeof(scan_string), stdin) == NULL)
                {
                    continue;
                }
                else
                {
                    host[strlen(host)-1] = 0;
                    break;
                }
            }
            while (1)
            {
                printf("Please input the port!\n");
                memset(scan_string, 0, sizeof(scan_string));
                if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                {
                    continue;
                }
                else
                {
                    port = atoi(scan_string);
                    break;
                }
            }
            mqtt_pub(host, port);
            break;
        case 3:
            while (1)
            {
                printf("Please input the server domain name or IP address!\n");
                memset(host, 0, sizeof(host));
                if (fgets(host, sizeof(scan_string), stdin) == NULL)
                {
                    continue;
                }
                else
                {
                    host[strlen(host)-1] = 0;
                    break;
                }
            }
            while (1)
            {
                printf("Please input the port!\n");
                memset(scan_string, 0, sizeof(scan_string));
                if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                {
                    continue;
                }
                else
                {
                    port = atoi(scan_string);
                    break;
                }
            }
            while (1)
            {
                printf("Please input the path and the name of ca file!\n");
                memset(cafile, 0, sizeof(cafile));
                if (fgets(cafile, sizeof(cafile), stdin) == NULL)
                {
                    continue;
                }
                else
                {
                    cafile[strlen(cafile)-1] = 0;
                    break;
                }
            }
            while (1)
            {
                printf("Please input the path and the name of Client Certificate file!\n");
                memset(certfile, 0, sizeof(certfile));
                if (fgets(certfile, sizeof(certfile), stdin) == NULL)
                {
                    continue;
                }
                else
                {
                    certfile[strlen(certfile)-1] = 0;
                    break;
                }
            }
            while (1)
            {
                printf("Please input the path and the name of Client key file!\n");
                memset(keyfile, 0, sizeof(keyfile));
                if (fgets(keyfile, sizeof(keyfile), stdin) == NULL)
                {
                    continue;
                }
                else
                {
                    keyfile[strlen(keyfile)-1] = 0;
                    break;
                }
            }

            mqtts_sub(host, port, cafile, certfile, keyfile);
            break;
        case 4:
            while (1)
            {
                printf("Please input the server domain name or IP address!\n");
                memset(host, 0, sizeof(host));
                if (fgets(host, sizeof(scan_string), stdin) == NULL)
                {
                    continue;
                }
                else
                {
                    host[strlen(host)-1] = 0;
                    break;
                }
            }
            while (1)
            {
                printf("Please input the port!\n");
                memset(scan_string, 0, sizeof(scan_string));
                if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                {
                    continue;
                }
                else
                {
                    port = atoi(scan_string);
                    break;
                }
            }
            while (1)
            {
                printf("Please input the path and the name of ca file!\n");
                memset(cafile, 0, sizeof(cafile));
                if (fgets(cafile, sizeof(cafile), stdin) == NULL)
                {
                    continue;
                }
                else
                {
                    cafile[strlen(cafile)-1] = 0;
                    break;
                }
            }
            while (1)
            {
                printf("Please input the path and the name of Client Certificate file!\n");
                memset(certfile, 0, sizeof(certfile));
                if (fgets(certfile, sizeof(certfile), stdin) == NULL)
                {
                    continue;
                }
                else
                {
                    certfile[strlen(certfile)-1] = 0;
                    break;
                }
            }
            while (1)
            {
                printf("Please input the path and the name of Client key file!\n");
                memset(keyfile, 0, sizeof(keyfile));
                if (fgets(keyfile, sizeof(keyfile), stdin) == NULL)
                {
                    continue;
                }
                else
                {
                    keyfile[strlen(keyfile)-1] = 0;
                    break;
                }
            }

            mqtts_pub(host, port, cafile, certfile, keyfile);
            break;
        default:
            break;
        }
        if (opt == 99)
        {
            break;
        }
    }
}
#endif

static void simcom_json_test()
{
    json_test1();
}
static void simcom_system_test()
{
    const char *options_list[] = 
    {
        "1. recover factory",
        "2. reboot",
        "3. set autosleep",
        "4. creat wake_lock",
        "5. remove wake_lock",
        "6. View wakeup lock",
        "7. get the CPU usage",
        "8. get the Memory usage",
        "9. set system timezone",
        "10. shutdown",
        "99. Back",
    };
    char scan_string[128] = {0};
    int opt = 0;
    int ret;
    int cpuusage;
    int memoryusage;
    while (1)
    {
        printf("\nPlease select an option to test from the items listed below.\n");
        print_option_menu(options_list, sizeof(options_list) / sizeof(options_list[0]));
        printf("Option > ");

        memset(scan_string, 0, sizeof(scan_string));
        if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
            continue;

        opt = atoi(scan_string);
        switch (opt)
        {
            case 1:
            {
                printf("Waiting...\n");
                ret = reset_factory();
                if (ret == 0)
                {
                    printf("recover factory faild\n");
                }
            }
                break;
            case 2:
            {
                printf("Waiting...\n");
                ret = reboot();
                if (ret == 0)
                {
                    printf("reboot faild\n");
                }
            }
                break;
            
            case 3:
            {
                printf("\nPlease input option 1 -- enable autosleep, 0 --disable autosleeep:\n");
                memset(scan_string, 0, sizeof(scan_string));
                if(fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                {
                    continue;
                }
                int choice = atoi(scan_string);
                ret = autosleep_enable(choice);
                
                if (ret)
                {
                    if(choice == DISABLE_AUTO_SLEEP)
                    {
                        printf("disable autosleep successful!\n");
                    }
                    else if(choice == ENABLE_AUTO_SLEEP)
                    {
                        printf("enable autosleep successful!\n");
                    }
                    break;
                }
                else
                {
                    printf("enable autosleep fail!\n");
                    break;
                }
            }
            break;

            case 4:
            {
                printf("\nPlease enter a wakeup lock name:");
                memset(scan_string, 0, sizeof(scan_string));
                if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                    continue;
                char lock_name[50] = {0};
                strncpy(lock_name, scan_string, strlen(scan_string) - 1);
                printf("\nSET hold time(ms) (0 :Not setting the time)\n");
                memset(scan_string, 0, sizeof(scan_string));
                if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                    continue;
                unsigned long time_out = atol(scan_string);
                if (creat_soure_lock(lock_name,time_out) == true)
                {
                    printf("creat successful!\n");
                }
                else
                {
                    printf("creat failed!\n");
                }
            }
            break;

            case 5:
            {
                printf("\nEnter the wakeup lock to remove\n");
                memset(scan_string, 0, sizeof(scan_string));
                if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                    continue;
                char unlock_name[50] = {0};
                strncpy(unlock_name,scan_string,strlen(scan_string)-1);
                remove_soure_lock(unlock_name);
                printf("remove successful!\n");
            }
            break;

            case 6:
            {
                int ret = view_soure_lock();
                if(!ret)
                {
                    printf("view fail!\n");
                }
            }
            break;

            case 7:
            {
                ret = get_cpu_usage(&cpuusage);
                if (!ret)
                {
                    printf("Faild to get cpu usage\n");
                }
                else
                {
                    printf("CPU USE(The percentage): %d\n",cpuusage);
                }
            }
            break;

            case 8:
            {
                ret = get_memory_usage(&memoryusage);
                if (!ret)
                {
                    printf("Faild to get cpu usage\n");
                }
                else
                {
                    printf("Memory Usage(The percentage): %d\n",memoryusage);
                }
             }
             break;

            case 9:
            {
                int timezone;
                printf("\nPlease input a corret timezone(0-24):");
                memset(scan_string, 0, sizeof(scan_string));
                if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                {
                    continue;
                }
                timezone = atoi(scan_string);
                ret = set_system_timezone(timezone);
                if (!ret)
                {
                    printf("Faild to set timezone\n");
                }
                else
                {
                    printf("Set Timezone successful\n");
                }
            }
              break;

            case 10:
            {
                printf("Waiting...\n");
                ret = power_off();
                if (ret == 0)
                {
                    printf("shutdown faild\n");
                }
            }
            break;

            default:
                break;
        }
        if (opt == 99)
        {
            break;
        }
    }
}

#ifdef FEATURE_DEMO_PHONEBOOK
static void simcom_phonebook_test()
{
    int ret;
    int opt = 0;
    int i, j;
    char scan_string[100] = {0};

    printf("enter phone book test\n");
    ret = pb_init();
    if (ret)
    {
        printf("pb init faild, ret:%d", ret);
        return;
    }

    const char *error_list[] =
    {
        "SUCCESS",
        "pb not init",
        "ril not init",
        "not find contact",
        "malloc faild",
        "error input",
        "storage full",
        "storage empty",
        "set sm faild",
        "set format faild",
        "unknown error",
    };

    const char *options_list[] = {
        "1. addnew pb",
        "2. delete pb",
        "3. edit contact",
        "4. get contact",
        "5. list contact",
        "6. get storage status",
        "99. back",
    };

    while (1)
    {
        printf("\nPlease select an option to test from the items listed below.\n");
        print_option_menu(options_list, sizeof(options_list) / sizeof(options_list[0]));
        printf("Option > ");

        memset(scan_string, 0, sizeof(scan_string));
        if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
            continue;

        opt = atoi(scan_string);
        switch (opt)
        {
            case 1:
            {
                contact_info info = {0};

                printf("input contact name:");
                memset(scan_string, 0, sizeof(scan_string));
                if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                    continue;
                strcpy(info.name, scan_string);
                info.name[strlen(info.name) - 1] = '\0';          //clear '\n'

                printf("input contact number:");
                memset(scan_string, 0, sizeof(scan_string));
                if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                    continue;
                strcpy(info.number, scan_string);
                info.number[strlen(info.number) - 1] = '\0';

                ret = add_new_contact(info, &(info.index));
                if(ret)
                {
                    printf("add contact faild: %s\n", error_list[ret]);
                }
                else
                {
                    printf("********************************************\n");
                    printf("index:%d\n", info.index);
                    printf("********************************************\n");
                }
            }
            break;

            case 2:
            {
                printf("input contact index:");
                memset(scan_string, 0, sizeof(scan_string));
                if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                    continue;

                int index = atoi(scan_string);
                ret = delete_contact(index);
                if(ret)
                {
                    printf("delete contact faild: %s\n", error_list[ret]);
                }
                else
                {
                    printf("delete success\n");
                }
            }
            break;

            case 3:
            {
                contact_info info = {0};

                printf("input contact index:");
                memset(scan_string, 0, sizeof(scan_string));
                if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                    continue;
                info.index = atoi(scan_string);

                printf("input contact name:");
                memset(scan_string, 0, sizeof(scan_string));
                if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                    continue;
                strcpy(info.name, scan_string);
                info.name[strlen(info.name) - 1] = '\0';

                printf("input contact number:");
                memset(scan_string, 0, sizeof(scan_string));
                if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                    continue;
                strcpy(info.number, scan_string);
                info.number[strlen(info.number) - 1] = '\0';

                ret = set_contact_info(info);
                if(ret)
                {
                    printf("edit contact faild: %s\n", error_list[ret]);
                }
                else
                {
                    printf("edit success\n");
                }
            }
            break;

            case 4:
            {
                contact_info *info = malloc(sizeof(contact_info));
                memset(info, 0, sizeof(contact_info));

                printf("input index:");
                memset(scan_string, 0, sizeof(scan_string));
                if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
                    continue;

                int index = atoi(scan_string);
                ret = get_contact_info(index, info);
                if(ret)
                {
                    printf("get contact info: %s\n", error_list[ret]);
                }
                else
                {
                    printf("********************************************\n");
                    printf("index: %d\nname:%s\nnumber:%s\n",info->index, info->name, info->number);
                    printf("********************************************\n");
                    free_contact_info(info);
                }
            }
            break;

            case 5:
            {
                contact_node *head= NULL;
                ret = get_contact_list(&head);
                if (ret)
                {
                    printf("get contact list faild: %s\n", error_list[ret]);
                }
                else
                {
                    contact_node *pos = head;
                    printf("********************************************\n");
                    while(pos)
                    {
                        printf("index=%d,number=%s,name=%s\n", pos->info->index,
                            pos->info->number ? pos->info->number : "", 
                            pos->info->name ? pos->info->name : "");
                        pos = pos->next;
                    }
                    printf("********************************************\n");
                    free_contact_list(&head);
                }
            }
            break;

            case 6:
            {
                int used = 0;
                int total = 0;
                ret = get_storage_status(&used, &total);
                if(ret)
                {
                   printf("get storage status faild: %s\n", error_list[ret]);
                }
                else
                {
                    printf("********************************************\n");
                    printf("used:%d, total:%d\n",used, total);
                    printf("********************************************\n");
                }
            }
            break;

            case 99:
            {
                break;
            }

            default:
            {
                printf("not support");
            }
            break;

        }
        if (opt == 99)
        {
            pb_deinit();
            break;
        }
    }
}
#endif

#ifdef FEATURE_DEMO_DMZ
static void simcom_dmz_test()
{
    int opt = 0;
    int ret = 0;
    char host[64] = {0};
    char scan_string[128] = {0};
    const char *options_list[] = {
        "1. set DMZ",
        "2. get DMZ",
        "3. close DMZ",
        "99. back"
        };

    while (1)
    {
        printf("\nPlease select an option to test from the items listed below.\n");
        print_option_menu(options_list, sizeof(options_list) / sizeof(options_list[0]));
        printf("Option > ");

        memset(scan_string, 0, sizeof(scan_string));
        if (fgets(scan_string, sizeof(scan_string), stdin) == NULL)
            continue;

        opt = atoi(scan_string);
        switch (opt)
        {
        case 1:
            {
                printf("please input IP address\n");
                printf("For example:xx.xx.xx.xx\n");
                printf("please input>");
                memset(host, 0, sizeof(host));
                if (fgets(host, sizeof(host), stdin) == NULL)
                continue;
                host[strlen(host) - 1] = 0;
                ret = set_dmz_ip(host);
                if (ret == -1)
                {
                    printf("Please enter the correct IP address\n");
                }
                else
                {
                    printf("Set up DMZ ip success\n");
                }
                
            }
            break;
        case 2:
            {
                memset(host, 0, sizeof(host));
                ret = get_dmz_ip(host);
                if (ret == -1)
                {
                    printf("Get ip is faild,Please set the IP address of the DMZ\n");
                }
                else
                {
                    printf("DMZ ip = %s\n", host);
                }
            }
            break;
        case 3:
            {
                ret = close_dmz();
                if (ret == -1)
                {
                    printf("close the dmz is faild\n");
                }
                else
                {
                    printf("The DMZ is shut down successfully, and the IP address of the DMZ is cleared\n");
                }
            }
        case 99:
            break;
        default:
            continue;
        }
        if (opt == 99)
        {
            break;
        }
    }
}
#endif

#ifdef FEATURE_DEMO_ECALL
void get_parameter_string(char *scan_string, unsigned int len)
{
    while (1)
    {
        printf(">");
        memset(scan_string, 0, len);
        if (fgets(scan_string, len, stdin)  == NULL || strlen(scan_string) <= 1)
        {
            continue;
        }
        if (scan_string[strlen(scan_string) - 1] == '\n')
        {
            scan_string[strlen(scan_string) - 1] = '\0';
        }
        return;
    }
}

int simcom_atoi(const char *str, unsigned int str_len)
{
    unsigned int i = 0;
    if (str_len == 0)
    {
        return SIMCOMDEMO_FALSE;
    }

    for (i = 0; i < str_len; i++)
    {
        if (str[i] < '0' || str[i] > '9')
        {
            break;
        }
    }

    if (i < str_len)
    {
        return SIMCOMDEMO_FALSE;
    }
    else
    {
        return atoi(str);
    }
}

unsigned int get_option(const char *options_list[], unsigned int size)
{
    int option = 0;
    char scan_string[MAX_SCAN_LENGTH] = {0};

    while (1)
    {
        printf("\nPlease select an option to test from the items listed below.\n");
        print_option_menu(options_list, size);
        printf("Option > ");

        memset(scan_string, 0, sizeof(scan_string));
        if (fgets(scan_string, sizeof(scan_string), stdin)  == NULL || strlen(scan_string) <= 1)
        {
            continue;
        }
        if (scan_string[strlen(scan_string) - 1] == '\n')
        {
            scan_string[strlen(scan_string) - 1] = '\0';
        }

        option = simcom_atoi((const char *)scan_string, (unsigned int)strlen(scan_string));
        if (option <= 0)
        {
            printf("input error or unable to match.\n");
            continue;
        }
        return (unsigned int)option;
    }
};

static void simcom_ecall_test()
{
    int opt = 0;
    const char *options_list[] = {
        "1. test ecall",
        "2. end",
        "3. generate msd",
        "4. push msd",
        "5. Manually initiated eCall",
        "6. Automatically initiated eCall",
        "99. back",
    };
        if(Ecall_init(process_simcom_ind_message) != 0)
        {
            printf("Ecall_init failed\n");
            return;
        }

    unsigned int list_size = sizeof(options_list) / sizeof(options_list[0]);
    while (1)
    {
        opt = get_option(options_list, list_size);
        switch (opt)
        {
            case 1:
            {
                char phone[MAX_SCAN_LENGTH] = {0};
                ecall_handle h_ecall;
                printf("please input the PSAP test number:\n");
                get_parameter_string(phone, sizeof(phone));

                if (Ecall_Start(phone, &h_ecall) != SIMCOMDEMO_TRUE)
                {
                    printf("ecall %s failed\n", phone);
                }
                else
                {
                    printf("ecall %s successful, h_ecall %d\n", phone, h_ecall);
                }
            }
            break;


            case 2:
            {
                char scan_string[MAX_SCAN_LENGTH] = {0};
                ecall_handle h_ecall = 0;
                printf("please enter ecall_handle:(greater than 0)\n");
                get_parameter_string(scan_string, sizeof(scan_string));
                h_ecall = simcom_atoi((const char *)scan_string, (unsigned int)strlen(scan_string));

                if (Ecall_End(h_ecall) != SIMCOMDEMO_TRUE)
                {
                    printf("end ecall_handle %d failed\n", h_ecall);
                }
                else
                {
                    printf("end ecall_handle %d successful\n", h_ecall);
                }
            }
            break;

            case 3:
            {
                ecall_msd_config_s_type msd_config;
                char scan_string[MAX_SCAN_LENGTH] = {0};
                printf("please input control info actType(0 or 1)\n");
                get_parameter_string(scan_string, sizeof(scan_string));
                msd_config.actType = simcom_atoi((const char *)scan_string, (unsigned int)strlen(scan_string));

                printf("please input control info callType(0 or 1)\n");
                get_parameter_string(scan_string, sizeof(scan_string));
                msd_config.callType = simcom_atoi((const char *)scan_string, (unsigned int)strlen(scan_string));

                printf("please input control info positionconfidence(0 or 1)\n");
                get_parameter_string(scan_string, sizeof(scan_string));
                msd_config.positionconfidence = simcom_atoi((const char *)scan_string, (unsigned int)strlen(scan_string));


                printf("please input control info vehicleType(1-13)\n");
                get_parameter_string(scan_string, sizeof(scan_string));
                msd_config.vehicleType = simcom_atoi((const char *)scan_string, (unsigned int)strlen(scan_string));


                printf("please input Isowmi_vin(0-9，A-H，J-N, P, R-Z less than 32)\n");
                get_parameter_string(scan_string, sizeof(scan_string));
                strncpy(msd_config.Isowmi_vin, scan_string,sizeof(msd_config.Isowmi_vin));

                printf("please input Isovds_vin(0-9，A-H，J-N, P, R-Z less than 32)\n");
                get_parameter_string(scan_string, sizeof(scan_string));
                strncpy(msd_config.Isovds_vin, scan_string,sizeof(msd_config.Isovds_vin));

                printf("please input isovisModelyear_vin(0-9，A-H，J-N, P, R-Z less than 32)\n");
                get_parameter_string(scan_string, sizeof(scan_string));
                strncpy(msd_config.isovisModelyear_vin, scan_string,sizeof(msd_config.isovisModelyear_vin));

                printf("please input isovisSeqPlant_vin(0-9，A-H，J-N, P, R-Z less than 32)\n");
                get_parameter_string(scan_string, sizeof(scan_string));
                strncpy(msd_config.isovisSeqPlant_vin, scan_string,sizeof(msd_config.isovisSeqPlant_vin));

                printf("please input gasolineTankPresent(0 or 1)\n");
                get_parameter_string(scan_string, sizeof(scan_string));
                msd_config.gasolineTankPresent = simcom_atoi((const char *)scan_string, (unsigned int)strlen(scan_string));


                printf("please input dieselTankPresent(0 or 1)\n");
                get_parameter_string(scan_string, sizeof(scan_string));
                msd_config.dieselTankPresent = simcom_atoi((const char *)scan_string, (unsigned int)strlen(scan_string));


                printf("please input compressedNaturalGas(0 or 1)\n");
                get_parameter_string(scan_string, sizeof(scan_string));
                msd_config.compressedNaturalGas = simcom_atoi((const char *)scan_string, (unsigned int)strlen(scan_string));


                printf("please input liquidPropaneGas(0 or 1)\n");
                get_parameter_string(scan_string, sizeof(scan_string));
                msd_config.liquidPropaneGas = simcom_atoi((const char *)scan_string, (unsigned int)strlen(scan_string));


                printf("please input electricEnergyStorage(0 or 1)\n");
                get_parameter_string(scan_string, sizeof(scan_string));
                msd_config.electricEnergyStorage = simcom_atoi((const char *)scan_string, (unsigned int)strlen(scan_string));


                printf("please input hydrogenStorage(0 or 1)\n");
                get_parameter_string(scan_string, sizeof(scan_string));
                msd_config.hydrogenStorage = simcom_atoi((const char *)scan_string, (unsigned int)strlen(scan_string));


                printf("please input otherStorage(0 or 1)\n");
                get_parameter_string(scan_string, sizeof(scan_string));
                msd_config.otherStorage = simcom_atoi((const char *)scan_string, (unsigned int)strlen(scan_string));


                printf("please input Position_latitude(-324000000 to 324000000)\n");
                get_parameter_string(scan_string, sizeof(scan_string));
                msd_config.Position_latitude = simcom_atoi((const char *)scan_string, (unsigned int)strlen(scan_string));


                printf("please input Position_longitude(-648000000 to 648000000)\n");
                get_parameter_string(scan_string, sizeof(scan_string));
                msd_config.Position_longitude = simcom_atoi((const char *)scan_string, (unsigned int)strlen(scan_string));


                printf("please input vehicleDirection(0 to 255)\n");
                get_parameter_string(scan_string, sizeof(scan_string));
                msd_config.vehicleDirection = simcom_atoi((const char *)scan_string, (unsigned int)strlen(scan_string));


                printf("please input VehicleLocationN1Delta(-512 to 511)\n");
                get_parameter_string(scan_string, sizeof(scan_string));
                msd_config.VehicleLocationN1Delta = simcom_atoi((const char *)scan_string, (unsigned int)strlen(scan_string));


                printf("please input VehicleLocationN2Delta(-512 to 511)\n");
                get_parameter_string(scan_string, sizeof(scan_string));
                msd_config.VehicleLocationN2Delta = simcom_atoi((const char *)scan_string, (unsigned int)strlen(scan_string));


                printf("please input numberOfPassengers(0 to 255)\n");
                get_parameter_string(scan_string, sizeof(scan_string));
                msd_config.numberOfPassengers = simcom_atoi((const char *)scan_string, (unsigned int)strlen(scan_string));


                if (Ecall_updateMSD(&msd_config) != SIMCOMDEMO_TRUE)
                {
                    printf("updateMSD msd failed\n");
                }
                else
                {
                    printf("updateMSD msd successful\n");
                }
            }
            break;

            case 4:
            {
                char scan_string[MAX_SCAN_LENGTH] = {0};
                ecall_handle h_ecall;
                printf("please enter ecall_handle:(greater than 0)\n");
                get_parameter_string(scan_string, sizeof(scan_string));
                h_ecall = simcom_atoi((const char *)scan_string, (unsigned int)strlen(scan_string));

                if (Ecall_pushMSD(h_ecall) != SIMCOMDEMO_TRUE)
                {
                    printf("push msd ecall_handle %d failed\n", h_ecall);
                }
                else
                {
                    printf("push msd ecall_handle %d successful\n", h_ecall);
                }
            }
            break;

			case 5:
            {
                char phone[MAX_SCAN_LENGTH] = {0};
                ecall_handle h_ecall;

                if (Ecall_Start_Manually(&h_ecall) != SIMCOMDEMO_TRUE)
                {
                    printf("Manually initiated eCall failed\n");
                }
                else
                {
                    printf("Manually initiated eCall successful, h_ecall %d\n",h_ecall);
                }
            }
            break;

			case 6:
            {
                char phone[MAX_SCAN_LENGTH] = {0};
                ecall_handle h_ecall;

                if (Ecall_Start_Automatically(&h_ecall) != SIMCOMDEMO_TRUE)
                {
                    printf("Automatically initiated eCall failed\n");
                }
                else
                {
                    printf("Automatically initiated eCall successful, h_ecall %d\n",h_ecall);
                }
            }
            break;

            default:
            break;
        }

        if (opt == 99)
        {
            break;
        }
    }

}
#endif
#endif /* SIMCOM_TEST_UI */
