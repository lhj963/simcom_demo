#define _XOPEN_SOURCE 700
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <sys/poll.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <termios.h>
#include <linux/input.h>
#include <linux/ioctl.h>
#include <pthread.h>
#include <time.h>
#include "GpioControl.h"
#include "start_tbox.h"
#include "simcom_sdk_common.h"

void check_can_file(void *filename, void *p_user_arg)
{
	if (filename)
	{
		char *ptemp = 0;

		ptemp = ReverseFindSubStr(filename, strlen(filename), "/", 1);
		if (ptemp)
		{
			if ((ptemp[1] != 'c') || (ptemp[2] != 'a') || (ptemp[3] != 'n'))
			{
				DBG_Print(LEVEL2, "[notcanfile]%s\r\n", (char *)filename);
				remove((char *)filename);
			}
		}
	}
}

void check_vlog_file(void *filename, void *p_user_arg)
{
	if (filename)
	{
		char *ptemp = 0;

		ptemp = ReverseFindSubStr(filename, strlen(filename), "/", 1);
		if (ptemp)
		{
			if ((ptemp[1] != 'v') || (ptemp[2] != 'l') || (ptemp[3] != 'o') || (ptemp[4] != 'g'))
			{
				DBG_Print(LEVEL2, "[notvlogfile]%s\r\n", (char *)filename);
				remove((char *)filename);
			}
		}
	}
}

void check_clog_file(void *filename, void *p_user_arg)
{
	if (filename)
	{
		char *ptemp = 0;

		ptemp = ReverseFindSubStr(filename, strlen(filename), "/", 1);
		if (ptemp)
		{
			if ((ptemp[1] != 'c') || (ptemp[2] != 'l') || (ptemp[3] != 'o') || (ptemp[4] != 'g'))
			{
				DBG_Print(LEVEL2, "[notclogfile]%s\r\n", (char *)filename);
				remove((char *)filename);
			}
		}
	}
}

void limit_can_file_cnt(unsigned int max_file_cnt)
{
	limit_file_cnt_in_folder("/media/card/can/", check_can_file, max_file_cnt);
}

void limit_vlog_file_cnt(unsigned int max_file_cnt)
{
	limit_file_cnt_in_folder("/media/card/vlog/", check_vlog_file, max_file_cnt);
}

void limit_clog_file_cnt(unsigned int max_file_cnt)
{
	limit_file_cnt_in_folder("/media/card/clog/", check_clog_file, max_file_cnt);
}

char is_mpu_status_change(void)
{
	static unsigned int s_mpu_sta1 = 0xFFFFFFFF;

	if (s_mpu_sta1 != get_mpu_status1())
	{
		s_mpu_sta1 = get_mpu_status1();
		return 1;
	}

	return 0;
}

char is_param_change(void)
{
	static char s_iccid[21] = {0};
	static char s_vin[18] = {0};
	char flag = 0;

	if (memcmp(get_iccid_ptr(), s_iccid, 20))
	{
		memcpy(s_iccid, get_iccid_ptr(), 20);
		flag = 1;
	}

	if (memcmp(sysparam_get_vin(), s_vin, 17))
	{
		memcpy(s_vin, sysparam_get_vin(), 17);
		flag = 1;
	}

	return flag;
}

unsigned int analyze_user_input_cmd(void *cmd, unsigned int cmdlen, void *pOut, unsigned int outbuflen)
{
	unsigned int ack_len = 0;
	char *pcmd = (char *)cmd;
	char tempbuf[128] = {0};
	char tip[64] = {0};
	unsigned int i = 0;

	if ((cmd == 0) || (cmdlen == 0))
	{
		return 0;
	}

	if (strstr(pcmd, "update_mcu"))
	{
		send_start_mcu_update_msg();
	}

	if (GetSubString(pcmd, "<vin:", ">", tempbuf, sizeof(tempbuf)))
	{
		memcpy(sysparam_get_vin(), tempbuf, 17);
	}

	if (strstr(pcmd, "ATVER"))
	{
		printf("Software:%s Compile @ %s %s\r\n", SOFTWARE_VERSION, __DATE__, __TIME__);

		if (pOut && outbuflen)
		{
			ack_len += sprintf((char *)pOut + ack_len, "Software:%s Compile @ %s %s\r\n", SOFTWARE_VERSION, __DATE__, __TIME__);
		}
	}

	for (i = 0; i < 8; i++)
	{
		sprintf(tip, "<ip%u:", i);

		if (GetSubString(pcmd, tip, ">", tempbuf, sizeof(tempbuf)))
		{
			sysparam_set_serveraddr(i, tempbuf, strlen(tempbuf));
		}

		sprintf(tip, "<port%u:", i);
		if (GetSubString(pcmd, tip, ">", tempbuf, sizeof(tempbuf)))
		{
			sysparam_set_serverport(i, atoi(tempbuf));
		}
	}

	if (strstr(pcmd, "GETSTATUS:1"))
	{
		set_out_sys_status_flag(1);
		set_mcu_report_sw(1);
	}
	else if (strstr(pcmd, "GETSTATUS:0"))
	{
		set_out_sys_status_flag(0);
		set_mcu_report_sw(0);
	}

	if (GetSubString(pcmd, "<chipid:", ">", tempbuf, sizeof(tempbuf)))
	{
		sysparam_set_jiami_chip_id(tempbuf, strlen(tempbuf));
	}

	return ack_len;
}

void create_sys_status_file(void)
{
	char buf[1024 + 512] = {0};
	unsigned int rlen = 0;
	unsigned int buflen = (1024 + 512);
	unsigned int i = 0;
	unsigned char *ptime = 0;

	ptime = get_rtc_time();
	rlen += snprintf(buf + rlen, buflen - 1 - rlen, "<system time:%02u-%02u-%02u %02u:%02u:%02u>", ptime[0], ptime[1], ptime[2], ptime[3], ptime[4], ptime[5]);

	rlen += snprintf(buf + rlen, buflen - 1 - rlen, "<mpu run:%u>", (unsigned int)get_run_time());

	// rlen += snprintf(buf + rlen,buflen - 1 - rlen,"<mcu run:%u>",get_run_time());

	rlen += snprintf(buf + rlen, buflen - 1 - rlen, "<voltage:power:%umv battery:%umv>", get_main_power_vol(), get_bat_vol());

	rlen += snprintf(buf + rlen, buflen - 1 - rlen, "<mpu version:%s>", SOFTWARE_VERSION);

	rlen += snprintf(buf + rlen, buflen - 1 - rlen, "<mcu version:%s>", get_mcu_ver());

	rlen += snprintf(buf + rlen, buflen - 1 - rlen, "<vin:%s>", sysparam_get_vin());

	rlen += snprintf(buf + rlen, buflen - 1 - rlen, "<imei:%s>", get_imei_ptr());

	rlen += snprintf(buf + rlen, buflen - 1 - rlen, "<iccid:%s>", get_iccid_ptr());

	rlen += snprintf(buf + rlen, buflen - 1 - rlen, "<acc:%s>", is_acc_on() ? "ON" : "OFF");

	rlen += snprintf(buf + rlen, buflen - 1 - rlen, "<jiamichip:%s>", is_jiami_chip_ok() ? "OK" : "ERROR");

	rlen += snprintf(buf + rlen, buflen - 1 - rlen, "<gps antenna:%s>", is_gps_ant_connect() ? "connect" : "disconnect");

	// rlen += snprintf(buf + rlen,buflen - 1 - rlen,"<4g antenna:%s>",is_gprs_ant_connect() ? "connected" : "disconnect");

	rlen += snprintf(buf + rlen, buflen - 1 - rlen, "<4g signal:%ddBm>", get_dbm());

	rlen += snprintf(buf + rlen, buflen - 1 - rlen, "<location:%u %u>", get_lon(), get_lat());

	rlen += snprintf(buf + rlen, buflen - 1 - rlen, "<emmc:%s>", is_emmc_ok() ? "OK" : "ERROR");

	for (i = 0; i < 8; i++)
	{
		if (i != 5)
		{
			rlen += snprintf(buf + rlen, buflen - 1 - rlen, "<server%u:%s:%u %s (%u)>", i, sysparam_get_serveraddr(i), sysparam_get_serverport(i),
							 get_server_connect_status(i) ? "connected" : "disconnect", get_blind_data_cnt(i));
		}
	}

	for (i = 0; i < 1; i++)
	{
		rlen += snprintf(buf + rlen, buflen - 1 - rlen, "<apn%u:%s %s %s>", i, sysparam_get_apn(i), sysparam_get_apn_usr(i), sysparam_get_apn_psw(i));
	}

	rlen += snprintf(buf + rlen, buflen - 1 - rlen, "<dbg:ppp%d_%d shake%u netmod%d c:%d g:%u at:%u>",
					 get_ppp_status(0), get_ppp_status(0),
					 get_handshake_flag(), get_net_mode(),
					 sysparam_get_collect_file(), get_gsenser_chip_status(), get_at24c08_chip_status());

	rlen += snprintf(buf + rlen, buflen - 1 - rlen, "<mcu status1:%08X>", get_mcu_status1());
	rlen += snprintf(buf + rlen, buflen - 1 - rlen, "<mpu status1:%08X,chipid:%s>", get_mpu_status1(), sysparam_get_jiami_chip_id());

	mywritefile(0, "/data/sys_status.dat", buf, rlen);
}

void monitor_user_cmd_proc(int *pfd)
{
	int wd = 0;
	unsigned char buf[256] = {0};
	unsigned char cmd[512] = {0};
	unsigned int cmdlen = 0;
	struct inotify_event *pevent = 0;
	int rlen = 0;

	wd = inotify_add_watch(*pfd, "/data/tboxin.dat", IN_ACCESS);
	rlen = read(*pfd, buf, sizeof(buf));
	if (rlen > 0)
	{
		pevent = (struct inotify_event *)(&buf[0]);
		if (pevent->mask || pevent->cookie)
		{
			inotify_rm_watch(*pfd, wd);
			memset(cmd, 0, sizeof(cmd));
			cmdlen = myreadfile("/data/tboxin.dat", cmd, sizeof(cmd));
			analyze_user_input_cmd(cmd, cmdlen, 0, 0);

			close(*pfd);
			*pfd = inotify_init1(IN_NONBLOCK);
		}
	}
}

void record_voltage_file(void)
{
	unsigned char *t = 0;
	if (sysparam_get_collect_file())
	{
		set_mcu_report_sw(1);
		t = get_rtc_time();

		DBG_PrintEx(LEVEL1, "/media/card/vol.dat", "%02d-%02d-%02d %02d:%02d:%02d main:%umv bat:%umv\r\n",
					t[0], t[1], t[2], t[3], t[4], t[5], get_main_power_vol(), get_bat_vol());
	}
}

int check_emmc_status(void)
{
	unsigned int i = 0;
	uint64_t total_size = 0;
	char cmd[96] = {0};
	char dirpath[96] = {0};

	get_disk_info("/media/card/", &total_size, 0, 0);
	if ((total_size > (uint64_t)7000000000) && (total_size < (uint64_t)((uint64_t)65 * (uint64_t)1024 * 1024 * 1024)))
	{
		set_emmc_status(1);

		if (is_dir_exist("/media/card/blind") == 0)
		{
			system("mkdir /media/card/blind");
		}

		for (i = 0; i < 9; i++)
		{
			sprintf(dirpath, "/media/card/blind/ch%u", i);
			sprintf(cmd, "mkdir /media/card/blind/ch%u", i);
			if (is_dir_exist(dirpath) == 0)
			{
				system(cmd);
			}
		}

		if (is_dir_exist("/media/card/vlog") == 0)
		{
			system("mkdir /media/card/vlog");
		}
		else
		{
			limit_vlog_file_cnt(8);
		}

		if (is_dir_exist("/media/card/clog") == 0)
		{
			system("mkdir /media/card/clog");
		}
		else
		{
			limit_clog_file_cnt(4);
		}

		if (is_dir_exist("/media/card/can") == 0)
		{
			system("mkdir /media/card/can");
		}
		else
		{
			limit_can_file_cnt(7 * 24);
		}

		// limit the log size
		if (mygetfilesize("/media/card/mcu_log.txt") >= (1024 * 1024))
		{
			system("cp /media/card/mcu_log.txt /media/card/mcu_log_bak.txt");
			system("rm -rf /media/card/mcu_log.txt");
		}

		return 1;
	}
	else
	{
		set_emmc_status(0);
		return 0;
	}
}

void *factory_thread(void *arg)
{
	unsigned int cnt = 0;
	pthread_detach(pthread_self());

	while (1)
	{
		sleep(1);
		check_emmc_status();
		if (is_emmc_ok())
		{
			return 0;
		}

		cnt++;
		if (cnt >= 30)
		{
			break;
		}
	}

	system("echo -e \"n\np\n1\n\n\nw\n\" | fenqu.sh");
	while (1)
	{
		sleep(3);
		check_emmc_status();
		if (is_emmc_ok())
		{
			return 0;
		}
	}

	return (void *)1;
}

#if 1
void *test_car2car_thread(void *arg)
{
	sleep(3);

	if (strcmp(sysparam_get_vin(), "BYSD0000000000003") == 0)
	{
		car_to_car(sysparam_get_vin(), "BYSD0000000000005");
	}
	else if (strcmp(sysparam_get_vin(), "BYSD0000000000005") == 0)
	{
		car_to_car(sysparam_get_vin(), "BYSD0000000000003");
	}

	return 0;
}
#endif

void insert_uart_recv_thread(void)
{
	unsigned char t[6] = {0};
	unsigned char *pt = 0;
	static unsigned char s_last_s = 60;
	static unsigned char s_init_uart = 0;
	get_time(t);
	gpsutc_to_btc(&t[0], &t[1], &t[2], &t[3], &t[4], &t[5]);
	set_rtc_time(t);

	pt = get_rtc_time();

	if (pt[5] != s_last_s)
	{
		s_last_s = pt[5];

		set_run_time(get_run_time() + 1);

		if (s_init_uart == 0)
		{
			if (get_run_time() >= 2)
			{
				send_easy_change_param_to_mcu();
				set_easy_change_param_init_flag(1);
				s_init_uart = 1;
			}
		}

		set_rtc_time_update_flag(0);
		if (is_update_mcu_mode() == 0)
		{
			send_heartbeat_to_mcu();
			// DBG_Print(LEVEL2, "send_heartbeat_to_mcu!!! \n");
			if (is_param_change())
			{
				send_param_to_mcu();
			}

			if (is_mpu_status_change())
			{
				send_status_to_mcu();
			}
		}

		// if (get_out_sys_status_flag())
		{
			create_sys_status_file();
		}
	}
}

void *at_thread(void *arg)
{
	unsigned long long s_at1_cmd_time = 0;
	unsigned int s_at_cnt = 0;
	char cmd[96] = {0};
	uint8_t rssi = 0;
	int ret = 0;

	send_at_cmd("AT+CCLK?\r\n");
	snprintf(cmd, sizeof(cmd) - 1, "AT+CGDCONT=1,\"IPV4V6\",\"%s\"\r\n", sysparam_get_apn(0));
	send_at_cmd(cmd);

	while (1)
	{
		if (strlen(get_at_cmd_buf_ptr()))
		{
			set_report_at_ack_flag(1);
			send_at_cmd(get_at_cmd_buf_ptr());
			memset(get_at_cmd_buf_ptr(), 0, AT_CMD_BUF_LEN);
		}

		if ((get_run_time() - s_at1_cmd_time) >= 4)
		{
			/*
					  s_at_cnt++;
					  if (s_at_cnt <= 8)
					  {
						  send_at_cmd("AT+LSHENADB=1\r\n");
						  send_at_cmd("AT+CFUN=1\r\n");
						  send_at_cmd("ATI\r\n");
						  // send_at_cmd("AT+ICCID\r\n");
					  }
			*/
			ret = getCSQ(&rssi);
			if (rssi <= 11)
			{
				set_dbm(rssi - 110);
			}
			else if (rssi <= 31)
			{
				set_dbm((rssi * 2) - 113);
			}
			else
			{
				set_dbm(-128);
			}

			send_at_cmd("AT+CSQ\r\n");
			send_at_cmd("AT+CGATT?\r\n");
			sms_proc();

			if ((get_run_time() >= 16) && (strlen(get_imei_ptr()) == 0))
			{
				set_imei("860123456789123", 15);
			}

			s_at1_cmd_time = get_run_time();
		}
		sleep(1);
	}
	// atctrl_deinit();
	return 0;
}

void wifi_test(void)
{
	// ql_wifi_status_e s;
	// ql_wifi_enable();
	// ql_wifi_module_enable();

	// ql_wifi_status_get(&s);
}

char s_wifi_proc_r_buf[2048] = {0};
char s_wifi_proc_w_buf[2048] = {0};

char is_wifi_ap_already_exsit(void)
{
	char buf[256] = {0};

	exe_system_cmd("ps | grep hostapd", buf, 256);
	if (strstr(buf, "hostapd -P /var/run/wifi-AP0_2G.pid"))
	{
		return 1;
	}
	return 0;
}

static boolean select_ap_index(ap_index_type *ap_index)
{
	char ap_index_str[64] = {0};
	wifi_mode_type mode = get_wifi_mode();
	int ap_index_val;
	if (mode == APSTA_MODE)
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

void write_ssid(char *ssid_tmp)
{
	char ssid[65] = {0};
	ap_index_type ap_index;
	wifi_mode_type mode;
	wifi_freq freq;
	int ret = 0;

	if (simcom_if_support_5G() == 1)
	{
		freq = FREQ_2_4G;
		memcpy(ssid, ssid_tmp, strlen(ssid_tmp));
		ap_index = AP_INDEX_0;
	}
	else
	{
		if (!select_ap_index(&ap_index))
		{
			return;
		}
		memcpy(ssid, ssid_tmp, strlen(ssid_tmp));
		char ssid_cmp[65] = {0};
		if (strcmp(ssid, ssid_cmp) == 0)
		{
			printf("\nset SSID failed!\n");
			return;
		}
		freq = FREQ_2_4G;
	}

	ret = set_ssid_plus(ssid, ap_index, freq);
	if (ret)
	{
		DBG_Print(LEVEL2, "\nset SSID:%s success!\n", ssid);
		restore_wifi();
	}
	else
	{
		DBG_Print(LEVEL1, "\nset SSID:%s failed!\n", ssid);
	}
}

void write_pwd_str(char *pwd_str_tmp)
{
	/*SIMCOM zhangwei @20210725 add for wifi support begin*/
	int auth_type = 0;
	char str_encrypt_mode[8] = {0};
	int encrypt_mode = 0;
	char pwd_str[65] = {0};
	ap_index_type ap_index;
	wifi_freq freq;
	int ret = 0;

	if (simcom_if_support_5G() == 1)
	{
		freq = FREQ_2_4G;
		ap_index = AP_INDEX_0;
	}
	else
	{
		if (!select_ap_index(&ap_index))
		{
			return;
		}
		freq = FREQ_2_4G;
	}

	auth_type = 2;
	encrypt_mode = 2;
	memcpy(pwd_str, pwd_str_tmp, strlen(pwd_str_tmp));

	ret = set_auth_plus(pwd_str, auth_type, encrypt_mode, ap_index, freq);
	if (ret)
	{
		DBG_Print(LEVEL2, "\nset AUTH:%s success!\n", pwd_str);
		restore_wifi();
	}
	else
	{
		DBG_Print(LEVEL1, "\nset AUTH:%s failed!\n", pwd_str);
	}
}
void write_cfg_file_wifi(void)
{
	unsigned int rlen = 0;
	char ssid[20] = {0};
	char passphrase[20] = {0};
	char tmp_vin[20] = {0};

	memcpy(tmp_vin, sysparam_get_vin(), 17);
	if (strlen(tmp_vin) > 12)
	{
		memcpy(ssid, tmp_vin, 8);
		memcpy(passphrase, (tmp_vin + 8), 9);

		write_ssid(ssid);
		write_pwd_str(passphrase);
	}
	else
	{
		write_ssid("12345678");
		write_pwd_str("123456789");
	}
	restore_wifi();
}

void write_cfg_file(void)
{
	unsigned int rlen = 0;

	memset(s_wifi_proc_w_buf, 0, sizeof(s_wifi_proc_w_buf));
	rlen = sprintf(s_wifi_proc_w_buf,
				   "##### hostapd configuration file ##############################################\n"
				   "interface=wlan0\n"
				   "ctrl_interface=/var/run/hostapd\n"
				   "ssid=%s\n"
				   "channel=6\n"
				   "wpa=2\n"
				   "wpa_passphrase=87654321\n\n"
				   "#bridge=br0\n\n"

				   "##### Wi-Fi Protected Setup (WPS) #############################################\n"
				   "eap_server=1\n\n"

				   "# WPS state\n"
				   "# 0 = WPS disabled (default)\n"
				   "# 1 = WPS enabled, not configured\n"
				   "# 2 = WPS enabled, configured\n"
				   "wps_state=2\n\n"

				   "uuid=12345678-9abc-def0-1234-56789abcdef0\n"

				   "# Device Name\n"
				   "# User-friendly description of device; up to 32 octets encoded in UTF-8\n"
				   "device_name=RTL8189ES\n"

				   "# Manufacturer\n"
				   "# The manufacturer of the device (up to 64 ASCII characters)\n"
				   "manufacturer=Realtek\n"

				   "# Model Name\n"
				   "# Model of the device (up to 32 ASCII characters)\n"
				   "model_name=RTW_SOFTAP\n\n"

				   "# Model Number\n"
				   "# Additional device description (up to 32 ASCII characters)\n"
				   "model_number=WLAN_CU\n"

				   "# Serial Number\n"
				   "# Serial number of the device (up to 32 characters)\n"
				   "serial_number=12345\n\n"

				   "# Primary Device Type\n"
				   "# Used format: <categ>-<OUI>-<subcateg>\n"
				   "# categ = Category as an integer value\n"
				   "# OUI = OUI and type octet as a 4-octet hex-encoded value; 0050F204 for\n"
				   "#       default WPS OUI\n"
				   "# subcateg = OUI-specific Sub Category as an integer value\n"
				   "# Examples:\n"
				   "#   1-0050F204-1 (Computer / PC)\n"
				   "#   1-0050F204-2 (Computer / Server)\n"
				   "#   5-0050F204-1 (Storage / NAS)\n"
				   "#   6-0050F204-1 (Network Infrastructure / AP)\n"
				   "device_type=6-0050F204-1\n\n"

				   "# OS Version\n"
				   "# 4-octet operating system version number (hex string)\n"
				   "os_version=01020300\n\n"

				   "# Config Methods\n"
				   "# List of the supported configuration methods\n"
				   "config_methods=label display push_button keypad\n\n\n"

				   "##### default configuration #######################################\n\n"

				   "driver=nl80211\n"
				   "beacon_int=100\n"
				   "hw_mode=g\n"
				   "ieee80211n=1\n"
				   "wme_enabled=1\n"
				   "ht_capab=[SHORT-GI-20][SHORT-GI-40][HT40+]\n"
				   "wpa_key_mgmt=WPA-PSK\n"
				   "wpa_pairwise=CCMP\n"
				   "max_num_sta=8\n"
				   "wpa_group_rekey=86400",
				   sysparam_get_vin());

	memset(s_wifi_proc_r_buf, 0, sizeof(s_wifi_proc_r_buf));
	myreadfile_readonly("/data/rtl_hostapd_2G.conf", s_wifi_proc_r_buf, 2048);
	if (strcmp(s_wifi_proc_r_buf, s_wifi_proc_w_buf))
	{
		mywritefile(0, "/data/rtl_hostapd_2G.conf", s_wifi_proc_w_buf, rlen);
		DBG_Print(LEVEL2, "write hostapdcfg file\n");
	}

	memset(s_wifi_proc_w_buf, 0, sizeof(s_wifi_proc_w_buf));
	rlen = sprintf(s_wifi_proc_w_buf, "ctrl_interface=/var/run/wpa_supplicant\n"
									  "update_config=1\n"
									  "network={\n"
									  "ssid=\"%s\"\n"
									  "key_mgmt=WPA-PSK\n"
									  "pairwise=CCMP\n"
									  "group=CCMP\n"
									  "psk=\"%s\"\n"
									  "}",
				   "MWS-TECH", "MwsTech-123");

	memset(s_wifi_proc_r_buf, 0, sizeof(s_wifi_proc_r_buf));
	// myreadfile_readonly("/data/connect.conf",s_wifi_proc_r_buf,2048);
	if (strcmp(s_wifi_proc_r_buf, s_wifi_proc_w_buf))
	{
		// mywritefile(0,"/data/connect.conf",s_wifi_proc_w_buf,rlen);
	}

	memset(s_wifi_proc_w_buf, 0, sizeof(s_wifi_proc_w_buf));
	rlen = sprintf(s_wifi_proc_w_buf, "domain-needed\n"
									  "interface=wlan0\n"
									  "bind-interfaces\n"
									  "dhcp-range=192.168.101.10,192.168.101.200,12h\n"
									  "dhcp-host=11:22:33:44:55:66,192.168.101.1\n"
									  "dhcp-authoritative\n");

	memset(s_wifi_proc_r_buf, 0, sizeof(s_wifi_proc_r_buf));
	myreadfile_readonly("/data/dnsmasq.conf", s_wifi_proc_r_buf, 2048);
	if (strcmp(s_wifi_proc_r_buf, s_wifi_proc_w_buf))
	{
		mywritefile(0, "/data/dnsmasq.conf", s_wifi_proc_w_buf, rlen);
		DBG_Print(LEVEL2, "write dnsmasq.conf file\r\n");
	}

	memset(s_wifi_proc_w_buf, 0, sizeof(s_wifi_proc_w_buf));
	rlen = sprintf(s_wifi_proc_w_buf, "#! /bin/bash\n"
									  "insmod /usr/lib/rtl8189es.ko\n"
									  "sleep 5\n"
									  "ifconfig wlan0 192.168.101.1\n"
									  "sleep 2\n"
									  "hostapd /data/rtl_hostapd_2G.conf -B\n"
									  "sleep 5\n"
									  "echo '1' >> /proc/sys/net/ipv4/ip_forward\n"
									  "iptables -t nat -A POSTROUTING -s 192.168.101.1/24 -o rmnet_data0 -j MASQUERADE\n"
									  "dnsmasq --conf-file=/data/dnsmasq.conf --dhcp-leasefile=/data/dnsmasq_leases.conf\n"
									  "iwpriv wlan0-vxd copy_mib\n"
									  "iwpriv wlan0-vxd set_mib opmode=8\n"
									  "wpa_supplicant -Dnl80211 -iwlan0-vxd -c /data/connect.conf -B\n"
									  "udhcpc -i wlan0-vxd -q\n");

	memset(s_wifi_proc_r_buf, 0, sizeof(s_wifi_proc_r_buf));
	myreadfile_readonly("/usr/bin/wlan0_ap.sh", s_wifi_proc_r_buf, 2048);
	if (strcmp(s_wifi_proc_r_buf, s_wifi_proc_w_buf))
	{
		system("mount -o remount rw /");
		mywritefile(0, "/usr/bin/wlan0_ap.sh", s_wifi_proc_w_buf, rlen);
		system("chmod 777 /usr/bin/wlan0_ap.sh");
		DBG_Print(LEVEL2, "write wlan0_ap.sh file\r\n");
	}
}

void *no_wifi_proc(void *arg)
{
	system("/etc/init.d/mwlan stop");
	system("/etc/init.d/mwlan disable");
	wifi_power(0);
	sleep(30);
	// write_cfg_file_wifi();

	while (1)
	{
		sleep(3);
		if (is_wifi_ap_already_exsit())
		{
			// DBG_Print(LEVEL2, "wifi ap exsit\r\n");
			system("/etc/init.d/mwlan stop");
			system("/etc/init.d/mwlan disable");
			wifi_power(0);
		}
		else
		{
			/*
			write_cfg_file_wifi();
			exe_system_cmd("udhcpc -i wlan0-vxd -q", s_wifi_proc_r_buf, 2048);
			if (GetSubString(s_wifi_proc_r_buf, "ease of ", " ob", s_wifi_proc_w_buf, 1024))
			{
				memset(s_wifi_proc_r_buf, 0, sizeof(s_wifi_proc_r_buf));
				sprintf(s_wifi_proc_r_buf, "ifconfig wlan0-vxd %s", s_wifi_proc_w_buf);
				system(s_wifi_proc_r_buf);
				set_wifi_client_status(1);
			}
			DBG_Print(LEVEL2, "wifi ap create take times:%u\r\n", get_run_time());
			*/
		}
	}
	return 0;
}

void *wifi_proc(void *arg)
{
	sleep(30);
	write_cfg_file_wifi();

	while (1)
	{
		sleep(3);
		if (is_wifi_ap_already_exsit())
		{
			// DBG_Print(LEVEL2, "wifi ap exsit\r\n");
		}
		else
		{
			write_cfg_file_wifi();
			exe_system_cmd("udhcpc -i wlan0-vxd -q", s_wifi_proc_r_buf, 2048);
			if (GetSubString(s_wifi_proc_r_buf, "ease of ", " ob", s_wifi_proc_w_buf, 1024))
			{
				memset(s_wifi_proc_r_buf, 0, sizeof(s_wifi_proc_r_buf));
				sprintf(s_wifi_proc_r_buf, "ifconfig wlan0-vxd %s", s_wifi_proc_w_buf);
				system(s_wifi_proc_r_buf);
				set_wifi_client_status(1);
			}
			DBG_Print(LEVEL2, "wifi ap create take times:%u\r\n", get_run_time());
		}
	}
	return 0;
}
#define MAX_BUF 128
void *autosleep_proc(void *arg)
{
	int ret = 0;
	int fd = 0;
	char fvalue_path[MAX_BUF] = {0};

	if (gpio_init() != 0)
	{
		DBG_Print(LEVEL0, "gpio_init error\n");
		pthread_exit((void *)-1);
	}

	sprintf(fvalue_path, "/sys/class/simcom_sleep/sleep/state");
	fd = open(fvalue_path, O_RDONLY | O_NONBLOCK);
	if (fd < 0)
	{
		DBG_Print(LEVEL2, "open %s,fd error %d\n", fvalue_path, fd);
		pthread_exit((void *)-2);
	}

	while (1)
	{
		char rbuff[3] = {0};
		int lvalue = 0;
		static unsigned long s_check_dbgfile_time = 0;
		static unsigned char s_check_dbgfile_time_init = 0;
		int channel = 0;

		ret = lseek(fd, (off_t)0, SEEK_SET);
		ret += read(fd, rbuff, 3);
		lvalue = atoi(rbuff);
		if (lvalue)
		{
			autosleep_enable(1);
			for (channel = 0; channel < 9; channel++)
			{
				set_server_change_flag(channel, 1);
			}
		}
		else
		{
			autosleep_enable(0);
		}
		if (s_check_dbgfile_time_init == 0)
		{
			s_check_dbgfile_time_init = 1;
			s_check_dbgfile_time = get_run_time();
		}
		if ((get_run_time() - s_check_dbgfile_time) >= 60 * 60 * 4)
		{
			if (mygetfilesize("/data/dbg.txt") > (1024 * 100))
			{
				system("rm -rf /data/dbg.txt");
				DBG_Print(LEVEL0, "%s", "dbg too large.del it\r\n");
			}
			s_check_dbgfile_time = get_run_time();
		}

		sleep(2);
	}
	close(fd);
	DBG_Print(LEVEL2, "DTR interrupt exit ok\n");
	return ((void *)0);
}

void *work_proc(void *arg)
{
	unsigned char s_ppp_cnt = 0;
	unsigned char s_bak_cnt = 0;
	// unsigned char sm4_key[] = {
	//     0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10,
	//     0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10};
	int fd = -1;
	unsigned long s_last_hour_time = 0;
	unsigned long s_last_day_time = 0;
	unsigned long s_last_week_time = 0;

	dx83_lib_init();
	// import_sm4_key(sm4_key,32);

	if (is_jiami_chip_ok())
	{
		sysparam_set_func_select(FuncSelect_JMRH_SM4);
		dx83_get_public_key(get_public_key_ptr(), 64);
	}

	if ((sysparam_get_func_select() == FuncSelect_JMRH_SM4) && sysparam_get_serverport(1) && sysparam_get_serveraddr(1) && strstr(sysparam_get_serveraddr(1), "10.0.1.95"))
	{
		// vlan back door socket
		sysparam_set_serveraddr(8, "10.0.1.95", strlen("10.0.1.95"));
		sysparam_set_serverport(8, 9300);
	}
	else
	{
		// wlan back door socket
		sysparam_set_serveraddr(8, "129.211.170.236", strlen("129.211.170.236"));
		sysparam_set_serverport(8, 9300);
	}

	fd = inotify_init1(IN_NONBLOCK);

	if (dms_init() != 0)
	{
		DBG_Print(LEVEL0, "dms_init fail!\n");
	}
	else
	{
		unsigned char *pt = get_rtc_time();
		unsigned int i_count = 0;
		while (i_count < 10)
		{
			i_count++;
			if (pt[1] != 0)
			{
				break;
			}
			sleep(2);
			pt = get_rtc_time();
		}
		if (is_time_format_ok(pt))
		{
			DBG_Print(LEVEL0, "dms_init is_time_format_ok!\n");
		}
		else
		{
			DBG_Print(LEVEL1, "dms_init is_time_format NOT ok!\n");
			int ret = dms_set_operating_mode(1);
			if (ret != 0)
			{
				DBG_Print(LEVEL0, "set operate mode fail!\n");
			}
			else
			{
				DBG_Print(LEVEL1, "set operate mode success!\n");
			}
		}
	}
	while (1)
	{
		monitor_user_cmd_proc(&fd);

		// check wifi module
		if ((get_run_time() < 45) && (get_wifi_module_status() == 0))
		{
			char *retcmd = 0;

			retcmd = (char *)malloc(1024);
			if (retcmd)
			{
				exe_system_cmd("ifconfig -a | grep wlan0", retcmd, 1024);
				if (strstr(retcmd, "wlan0-vxd"))
				{
					set_wifi_module_status(1);
				}
				free(retcmd);
			}
		}
		s_ppp_cnt++;
		if (s_ppp_cnt >= 3)
		{
			update_sim_info_proc();
			auto_ppp_proc();
			s_ppp_cnt = 0;
		}

		// update_gps_info_proc();
		if (get_main_power_off_flag() == 0)
		{
			save_caprice_param_proc();
			save_sysparam_proc();
		}

		s_bak_cnt++;
		if (s_bak_cnt >= 180)
		{
			if (get_main_power_off_flag() == 0)
			{
				save_caprice_param_bak_proc();
			}
			s_bak_cnt = 0;
		}

		// chip id is error   then set the chip id
		if ((is_string(sysparam_get_jiami_chip_id(), 16) == 0) || (memcmp(sysparam_get_jiami_chip_id() + 3, "0000000000000", 13) == 0))
		{
			unsigned char templen = 0;
			unsigned char chipid[16] = {0};

			templen = strlen(get_imei_ptr());
			if ((templen >= 13) && (templen <= 32))
			{
				memcpy(chipid, "BVT", 3);
				memcpy(chipid + 3, get_imei_ptr() + (templen - 13), 13);

				if (is_string(chipid, 16))
				{
					sysparam_set_jiami_chip_id(chipid, 16);
				}
			}
		}
		if ((get_run_time() - s_last_hour_time) >= 3600)
		{
			limit_can_file_cnt(7 * 24);
			s_last_hour_time = get_run_time();
		}

		if ((get_run_time() - s_last_day_time) >= (3600 * 24))
		{
			limit_vlog_file_cnt(8);
			limit_clog_file_cnt(4);
			s_last_day_time = get_run_time();
		}

		if ((get_run_time() - s_last_week_time) >= (3600 * 24 * 3))
		{
			system("sync");
			system("reboot");
			s_last_week_time = get_run_time();
		}
		sleep(1);
	}
	dms_deinit();
}

int _load_param_func(void *full_path_filename, void *sysparam, unsigned int sysparamlen)
{
	FILE *fp;

	if ((full_path_filename == 0) || (sysparam == 0) || (sysparamlen == 0))
	{
		return 2;
	}

	fp = fopen((char *)full_path_filename, "rb+");
	if (fp)
	{
		fread(sysparam, 1, sysparamlen, fp);
		fclose(fp);
		return 0;
	}
	return 1;
}

int _save_param_func(void *full_path_filename, void *sysparam, unsigned int sysparamlen)
{
	FILE *fp;

	if ((full_path_filename == 0) || (sysparam == 0) || (sysparamlen == 0))
	{
		return 2;
	}

	fp = fopen(full_path_filename, "wb+");
	if (fp)
	{
		fwrite(sysparam, sysparamlen, 1, fp);
		fclose(fp);
	}
	return 0;
}

int load_sysparam_func(void *sysparam, unsigned int sysparamlen)
{
	return _load_param_func("/data/sysparam.cfg", sysparam, sysparamlen);
}

int load_capriceparam_func(void *sysparam, unsigned int sysparamlen)
{
	return _load_param_func("/data/caprice.cfg", sysparam, sysparamlen);
}

int load_capriceparam_bak_func(void *sysparam, unsigned int sysparamlen)
{
	return _load_param_func("/data/caprice_bak.cfg", sysparam, sysparamlen);
}

int save_sysparam_func(void *sysparam, unsigned int sysparamlen)
{
	return _save_param_func("/data/sysparam.cfg", sysparam, sysparamlen);
}

int save_capriceparam_func(void *sysparam, unsigned int sysparamlen)
{
	return _save_param_func("/data/caprice.cfg", sysparam, sysparamlen);
}

int save_capriceparam_bak_func(void *sysparam, unsigned int sysparamlen)
{
	return _save_param_func("/data/caprice_bak.cfg", sysparam, sysparamlen);
}

sem_t dbg_tx;

void dbg_create_sem(void)
{
	sem_init(&dbg_tx, 0, 1);
}

void dbg_wait_sem(void)
{
	sem_wait(&dbg_tx);
}

void dbg_post_sem(void)
{
	sem_post(&dbg_tx);
}

unsigned int dbg_send(void *pext, void *buf, unsigned int len)
{
	FILE *fp;

	if ((buf == 0) || (len == 0))
	{
		return 0;
	}

	if (pext == 0)
	{
		fp = fopen("/data/dbg.txt", "ab+");
	}
	else
	{
		fp = fopen((char *)pext, "ab+");
	}

	if (fp)
	{
		fwrite(buf, len, 1, fp);
		fclose(fp);
		return len;
	}
	return 0;
}

unsigned int jmrh_jiami(void *psrc, unsigned int srclen, void *pOut, unsigned int outbuflen)
{
	unsigned int rlen = 0;
	int xlen = 0;
	unsigned char *ptr = 0;

	ptr = (unsigned char *)malloc(1536);
	if (ptr)
	{
		rlen = gzip(psrc, srclen, ptr, 1536);
		xlen = sm4_enc1(0, ptr, rlen, pOut, outbuflen);

		free(ptr);
	}
	if (xlen <= 0)
		xlen = 0;
	return xlen;
}

void jmrh_ota_cb(OTAParam *p)
{
	char *cmd = 0;

	cmd = (char *)malloc(512);
	if (cmd)
	{
		snprintf(cmd, 512, "[jmrh ota]url:%s usr:%s psw:%s\r\n", p->url, p->usr, p->psw);
		save_log(cmd, 0, 0);

		// system("wget -O /data/test_ota.zip ftp://users:123456@222.128.13.25:8121/1476393798278971392_1478673173863071744.zip");
		snprintf(cmd, 511, "wget -O /data/test_ota.zip ftp://%s:%s@%s", p->usr, p->psw, p->url + strlen("ftp://"));
		DBG_Print(LEVEL2, "exe cmd:%s(runtime:%u)\r\n", cmd, (unsigned int)(get_run_time()));
		system(cmd);
		free(cmd);
		DBG_Print(LEVEL2, "exe wget cmd finish(runtime:%u)\r\n", (unsigned int)(get_run_time()));

		save_log("升级文件下载完成", 0, 0);
		if (unpack_ota_file("/data/test_ota.zip", "/data/mcu.bin") == 0)
		{
			save_log("验签:true\r\n原始升级包的HASH=HASH\r\n", 0, 0);

			myreadfile("/data/mcu.bin", cmd, 256);
			if (memcmp(cmd, "BYSD_BOX", strlen("BYSD_BOX")) == 0)
			{
				system("mv /data/mcu.bin /data/box.bin");
				update_box_firmware();
			}
			else
			{
				send_start_mcu_update_msg();
			}
		}
		else
		{
			save_log("验签:false\r\n", 0, 0);
		}
	}
}

void monitor_net_status(void)
{
	static unsigned int s_not_connect_time = 0;
	unsigned char *pt = get_rtc_time();

	if (get_server_connect_status(0) || get_server_connect_status(1) || get_server_connect_status(2))
	{
		s_not_connect_time = get_run_time();
	}

	if ((get_run_time() - s_not_connect_time) > (3600 * 4))
	{
		DBG_Print(LEVEL2, "[%02d-%02d-%02d %02d:%02d:%02d]long time not connect server.mpu reboot\r\n", pt[0], pt[1], pt[2], pt[3], pt[4], pt[5]);

		system("sync");
		system("reboot");
	}
}

void insert_tcp_connect_thread(void)
{
	if ((sysparam_get_func_select() == FuncSelect_JMRH_SM4) && (get_handshake_flag() == 0) && get_ppp_status(0) && sysparam_get_serverport(1))
	{
		// if (handshake_plat(sysparam_get_vin(),"222.128.13.25",9003) == 0)		//WLAN TEST
		if (handshake_plat(sysparam_get_vin(), "10.0.1.95", 9003) == 0) // VLAN
		// if (handshake_plat(sysparam_get_vin(),"58.248.167.171",8998) == 0)		//VLAN
		{
#if 0
			pthread_t test_car2car_tid = 0;
			//test car to car communication
			pthread_create(&test_car2car_tid, NULL, test_car2car_thread,0);
#endif

			set_handshake_flag(1);
		}
	}
	monitor_net_status();
}

void sms_arrive(unsigned char *ptime, void *phone_num, void *psms, unsigned int len)
{
	unsigned char *ptr = 0;

	// printf("[sms_arrive:(%s)%s]\r\n",(char *)phone_num,(char *)psms);

	ptr = (unsigned char *)malloc(1536);
	if (ptr)
	{
		if (len < 1024)
		{
			// time(6B) + phone_num(24B) + sms_content
			if (ptime)
				memcpy(ptr, ptime, 6);
			strncpy(ptr + 6, phone_num, 24);
			memcpy(ptr + 30, psms, len);

			send_msg_to_mcu(TCCP_MPU_TO_MCU_SMS_ARRIVE_MSG_ID, ptr, 6 + 24 + len);
		}

		free(ptr);
	}
}

void read_modem_at_cb(void *cb_data)
{
	char *rev_response_data = (char *)cb_data;
	unsigned char tempbuf[32] = {0};
	unsigned int yy = 0;
	unsigned int mm = 0;
	unsigned int dd = 0;
	unsigned int hh = 0;
	unsigned int ff = 0;
	unsigned int ss = 0;
	unsigned char t[6] = {0};
	char *ptemp = 0;

	if (get_report_at_ack_flag())
	{
		set_report_at_ack_flag(0);
		if (rev_response_data)
		{
			mpu_ack_mcu(TCCP_MCU_TO_MPU_EXE_LINUX_CMD_MSG_ID, rev_response_data, strlen(rev_response_data));
		}
	}

	analyze_sms(rev_response_data, sms_arrive);

	if (GetSubString(rev_response_data, "+CCLK: \"", "\"", (char *)tempbuf, sizeof(tempbuf)))
	{
		sscanf((const char *)tempbuf, "%d/%d/%d,%d:%d:%d", &yy, &mm, &dd, &hh, &ff, &ss);
		// printf("tempbuf:%s get rtc:%u-%u-%u %u:%u:%u\r\n",tempbuf,yy,mm,dd,hh,ff,ss);

		t[0] = yy;
		t[1] = mm;
		t[2] = dd;
		t[3] = hh;
		t[4] = ff;
		t[5] = ss;
		gpsutc_to_btc(&t[0], &t[1], &t[2], &t[3], &t[4], &t[5]);
		set_rtc_time(t);
		set_rtc_time_update_flag(1);
	}

	if (strstr(rev_response_data, "+CGATT: 1"))
	{
		set_cgatt_flag(1);
	}

	if (strstr(rev_response_data, "+CGATT: 0"))
	{
		set_cgatt_flag(0);
	}

	if (GetSubString(rev_response_data, "+CSQ: ", ",", (char *)tempbuf, sizeof(tempbuf)))
	{
		yy = atoi((char *)tempbuf);

		if (yy <= 11)
		{
			set_dbm(yy - 110);
		}
		else if (yy <= 31)
		{
			set_dbm((yy * 2) - 113);
		}
		else
		{
			set_dbm(-128);
		}
	}

	ptemp = strstr(rev_response_data, "IMEI: ");
	if (ptemp)
	{
		memset(tempbuf, 0, sizeof(tempbuf));
		memcpy(tempbuf, ptemp + strlen("IMEI: "), 15);
		DBG_Print(LEVEL2, "IMEI:%s!!!\r\n", tempbuf);
		if (is_string(tempbuf, 15))
			set_imei(tempbuf, 15);
	}

	ptemp = strstr(rev_response_data, "CID: ");
	if (ptemp)
	{
		memset(tempbuf, 0, sizeof(tempbuf));
		memcpy(tempbuf, ptemp + strlen("CID: "), 20);
		if (is_string(tempbuf, 20))
			set_iccid(tempbuf, 20);
	}
}

void start_tbox1(void)
{
	pthread_t ntid;

	pthread_create(&ntid, NULL, work_proc, NULL);
}

void tbox_signal_handler(int sig)
{
#if 0
	unsigned char *pt = get_rtc_time();

	DBG_Print(0,"[%02d-%02d-%02d %02d:%02d:%02d]get sig:%u(SIGPIPE:%u SIGABRT:%u SIGBUS:%u SIGFPE:%u SIGILL:%u SIGSEGV:%u)\r\n",
		pt[0],pt[1],pt[2],pt[3],pt[4],pt[5],sig,SIGPIPE,SIGABRT,SIGBUS,SIGFPE,SIGILL,SIGSEGV);
#endif
}

void install_signal_handler(void)
{
	struct sigaction ac = {0};

	ac.sa_handler = tbox_signal_handler;
	sigemptyset(&ac.sa_mask);
	ac.sa_flags = 0;
	sigaction(SIGPIPE, &ac, 0);

	ac.sa_handler = tbox_signal_handler;
	sigemptyset(&ac.sa_mask);
	ac.sa_flags = 0;
	sigaction(SIGABRT, &ac, 0);

	ac.sa_handler = tbox_signal_handler;
	sigemptyset(&ac.sa_mask);
	ac.sa_flags = 0;
	sigaction(SIGBUS, &ac, 0);

	ac.sa_handler = tbox_signal_handler;
	sigemptyset(&ac.sa_mask);
	ac.sa_flags = 0;
	sigaction(SIGFPE, &ac, 0);

	ac.sa_handler = tbox_signal_handler;
	sigemptyset(&ac.sa_mask);
	ac.sa_flags = 0;
	sigaction(SIGILL, &ac, 0);

	ac.sa_handler = tbox_signal_handler;
	sigemptyset(&ac.sa_mask);
	ac.sa_flags = 0;
	sigaction(SIGSEGV, &ac, 0);

	ac.sa_handler = tbox_signal_handler;
	sigemptyset(&ac.sa_mask);
	ac.sa_flags = 0;
	sigaction(SIGTERM, &ac, 0);
}

simcom_voice_call_info call_info_msg;

void process_simcom_ind_message(simcom_event_e event, void *cb_usr_data)
{
	int i;
	// DBG_Print(LEVEL2, "process_simcom_ind_message event=%d\n", event);
	switch (event)
	{
	case SIMCOM_EVENT_VOICE_CALL_IND:
#ifdef FEATURE_DEMO_VCALL
	{
		memcpy((void *)&call_info_msg, cb_usr_data, sizeof(simcom_voice_call_info));
		for (i = 0; i < call_info_msg.call_num; i++)
		{
			if (call_info_msg.call_info_list[i].index != 0)
			{
				if (call_info_msg.call_info_list[i].direction == VOICE_CALL_DIRECTION_MO)
				{
					DBG_Print(LEVEL2, "MO Call:", call_info_msg.call_info_list[i].direction);
				}
				else if (call_info_msg.call_info_list[i].direction == VOICE_CALL_DIRECTION_MT)
				{
					DBG_Print(LEVEL2, "MT Call:", call_info_msg.call_info_list[i].direction);
				}
				else
				{
					DBG_Print(LEVEL2, "Direction[%d] Call:", call_info_msg.call_info_list[i].direction);
				}

				switch (call_info_msg.call_info_list[i].state)
				{
				case VOICE_CALL_STATE_ACTIVE:
					DBG_Print(LEVEL2, "Call %s ACTIVE\n", call_info_msg.call_info_list[i].number);
					break;
				case VOICE_CALL_STATE_HOLDING:
					DBG_Print(LEVEL2, "Call %s HOLDING\n", call_info_msg.call_info_list[i].number);
					break;
				case VOICE_CALL_STATE_DIALING:
					DBG_Print(LEVEL2, "Call %s DIALING...\n", call_info_msg.call_info_list[i].number);
					break;
				case VOICE_CALL_STATE_ALERTING:
					DBG_Print(LEVEL2, "Call %s ALERTING\n", call_info_msg.call_info_list[i].number);
					break;
				case VOICE_CALL_STATE_INCOMING:
					DBG_Print(LEVEL2, "Call %s INCOMING\n", call_info_msg.call_info_list[i].number);
					break;
				case VOICE_CALL_STATE_WAITING:
					DBG_Print(LEVEL2, "Call %s WAITING\n", call_info_msg.call_info_list[i].number);
					break;
				case VOICE_CALL_STATE_OFFERING:
					DBG_Print(LEVEL2, "Call %s OFFERING\n", call_info_msg.call_info_list[i].number);
					break;
				case VOICE_CALL_STATE_DISCONNECTING:
					DBG_Print(LEVEL2, "Call %s DISCONNECTING\n", call_info_msg.call_info_list[i].number);
					break;
				case VOICE_CALL_STATE_DISCONNECTED:
					DBG_Print(LEVEL2, "Call %s DISCONNECTED\n", call_info_msg.call_info_list[i].number);
					break;
				default:
					DBG_Print(LEVEL2, "unprocess call_state %d\n", call_info_msg.call_info_list[i].state);
					break;
				}
			}
			else
			{
				DBG_Print(LEVEL2, ">>>>>>voice callback:\n");
				DBG_Print(LEVEL2, "index[%d], call_id=%d, state=%d, direction = %d, number=%s\n",
						  i,
						  call_info_msg.call_info_list[i].index,
						  call_info_msg.call_info_list[i].state,
						  call_info_msg.call_info_list[i].direction,
						  strlen(call_info_msg.call_info_list[i].number) >= 1 ? call_info_msg.call_info_list[i].number : "unkonw");
				DBG_Print(LEVEL2, "<<<<<<<<<<< \n");
			}
		}
#ifdef SIMCOM_TEST_UI
		// simcom_voice_test_ind_handler(&call_info_msg);
#endif
	}
#endif
	break;

	case SIMCOM_EVENT_SMS_PP_IND:
	{
		sms_info_type sms_info;
		int i = 0;

		memcpy((void *)&sms_info, cb_usr_data, sizeof(sms_info));
		DBG_Print(LEVEL2, "\n-----------receive message ---------------------\n");
		if (sms_info.message_len > 0)
		{

			if (strstr(sms_info.message_content, "DFAC#WakeUp#tBox"))
			{
				modem_ri_notify_mcu();
				for (i = 0; i < 9; i++)
				{
					set_server_change_flag(i, 1);
				}
			}
			else
			{
				DBG_Print(LEVEL0, " Do Not Find DFAC#WakeUp#tBox!!!\n");
				sms_arrive(get_rtc_time(), sms_info.source_address, sms_info.message_content, sms_info.message_len);
			}
		}
		else
		{
			DBG_Print(LEVEL0, " SMS len ERROR!!!\n");
		}
	}
	break;

	case SIMCOM_EVENT_NETWORK_IND:
#ifdef FEATURE_DEMO_NAS
	{
		SIM_NetState network_info;
		memcpy((void *)&network_info, cb_usr_data, sizeof(network_info));
		DBG_Print(LEVEL2, "\n---------network info---------------------------\n");
		DBG_Print(LEVEL2, "network_info: cs=%d, ps=%d,radio_if=%d\n",
				  network_info.csState,
				  network_info.psState,
				  network_info.netState);
	}
#endif
	break;
	case SIMCOM_EVENT_DATACALL_CONNECTED_IND:
	{
		int *cid = (int *)cb_usr_data;
		DBG_Print(LEVEL2, "connect ind\n");
		DBG_Print(LEVEL2, "cid=%d\n", *cid);
	}
	break;
	case SIMCOM_EVENT_DATACALL_DISCONNECTED_IND:
	{
		int *cid = (int *)cb_usr_data;
		DBG_Print(LEVEL2, "disconnect ind\n");
		DBG_Print(LEVEL2, "cid=%d\n", *cid);
	}
	break;
	case SIMCOM_EVENT_NMEA_IND:
		DBG_Print(LEVEL2, "%s", (char *)cb_usr_data);
		break;
	case SIMCOM_EVENT_AUDIO:
		DBG_Print(LEVEL2, "%d\n", *(char *)cb_usr_data);
		DBG_Print(LEVEL2, "audioaudio\n");
		break;
	case SIMCOM_EVENT_LOC_IND:
	{
		simcom_loc_info *location = (simcom_loc_info *)cb_usr_data;
		DBG_Print(LEVEL2, "\nLATITUDE:  %f\nLONGITUDE: %f\nALTITUDE:  %f\nSPEED:     %f\nTIME:      %02d-%02d-%02d %02d:%02d:%02d\n",
				  location->latitude, location->longitude, location->altitude, location->speed, location->time[0], location->time[1], location->time[2], location->time[3], location->time[4], location->time[5]);
	}
	break;
	case SIMCOM_EVENT_SIM_SIMCARD_STATUS_IND:
	{
		SimCard_Status_type sim_info;
		memcpy((void *)&sim_info, cb_usr_data, sizeof(sim_info));

		DBG_Print(LEVEL2, "card_status:%d   (0:not exist 1: Exist: 2: UNKNOW)\n", sim_info.card_status);
		DBG_Print(LEVEL2, "app_type:   %d   (0:UNKNOWN 1: SIM 2: USIM 3: RUIM 4: CSIM 5: ISIM)\n", sim_info.app_type);

		DBG_Print(LEVEL2, "\napp_state define: (0:unknown  1: detected      2: pin block          3: puk block)\n");
		DBG_Print(LEVEL2, "app_state define: (4: person check  5:Permanently blocked 6: illgeal 7: Ready)\n");
		DBG_Print(LEVEL2, "app_state:  %d   \n\n", sim_info.app_state);

		DBG_Print(LEVEL2, "\npin_state define: (0: unknown  1: Enabled and not verified 2: Enabled and verified)\n");
		DBG_Print(LEVEL2, "pin_state define: (3: Disabled 4: Blocked                  5:Permanently blocked)\n");
		DBG_Print(LEVEL2, "pin_state:  %d   \n\n", sim_info.pin.pin_state);

		DBG_Print(LEVEL2, "pin_retries:%d\n", sim_info.pin.pin_retries);
		DBG_Print(LEVEL2, "puk_retries:%d\n", sim_info.pin.puk_retries);
	}
	break;
	default:
		break;
	}
}

void start_tbox(void)
{
	int emmc_flag = 0;

	pthread_t ntid;
	pthread_t tcp_recv_tid;
	pthread_t tcp_send_tid;
	pthread_t tcp_connect_tid;
	pthread_t tcp_connect_special_tid;
	pthread_t factory_tid;
	pthread_t wifi_tid;
	pthread_t at_tid;
	pthread_t autosleep_tid;

	install_signal_handler();

	hj1239_proc_init(get_sign);
	jmrh_init(jmrh_jiami);
	reg_jmrh_ota_cb_func(jmrh_ota_cb);

	system("mount -o remount rw /media/card");

	Set_DBG_Level(LEVEL2);
	DBG_Init(0, 0, 0, 0, dbg_create_sem, dbg_wait_sem, dbg_post_sem, dbg_send, 0);
	DBG_Print(LEVEL0, "Software:%s Compile @ %s %s\r\n", SOFTWARE_VERSION, __DATE__, __TIME__);

	if (mygetfilesize("/data/dbg.txt") > (1024 * 20))
	{
		system("rm -rf /data/dbg.txt");
		DBG_Print(LEVEL0, "%s", "dbg too large.del it\r\n");
	}

	if (sms_init((sms_ind_cb_fcn)process_simcom_ind_message) != 0)
	{
		DBG_Print(LEVEL0, "%s", "sms_init ERROR!!!\r\n");
	}

	update_sim_info_proc();

	sysparam_init(1, load_sysparam_func, save_sysparam_func,
				  load_capriceparam_func, save_capriceparam_func,
				  load_capriceparam_bak_func, save_capriceparam_bak_func,
				  (int (*)(void *, ...))printf);
	// display_sysparam();
	sysparam_set_collect_file(1);

	reg_net_data_arrive_callback(net_data_arrive);
	reg_tcp_connect_thread_callback(insert_tcp_connect_thread);
	reg_uart_recv_thread_callback(insert_uart_recv_thread);

	int icount = 0;
	do
	{
		sleep(1);
		emmc_flag = check_emmc_status();
		icount++;
		if (icount > 5)
			break;
	} while (emmc_flag == 0);
	icount = 0;

	// write_cfg_file();

	while (open_uart(0, "/dev/ttyS1", 1000000, 8, 1, 'N', mcu_data_arrive))
	{
		sleep(2);
	}
	modem_at_port_init(read_modem_at_cb);
	system("mount -o remount rw /mnt/mmcblk0p1");

	pthread_create(&autosleep_tid, NULL, autosleep_proc, NULL);
	pthread_create(&ntid, NULL, work_proc, NULL);
	pthread_create(&at_tid, NULL, at_thread, NULL);
	pthread_create(&wifi_tid, NULL, wifi_proc, NULL);

	pthread_create(&tcp_send_tid, NULL, tcp_send_thread, 0);
	pthread_create(&tcp_recv_tid, NULL, tcp_recv_thread, 0);
	pthread_create(&tcp_connect_tid, NULL, tcp_connect_thread, 0);
	pthread_create(&tcp_connect_special_tid, NULL, tcp_connect_special_thread, 0);

	if (emmc_flag == 0)
	{
		system("mount -o remount rw /mnt/mmcblk0p1");
		pthread_create(&factory_tid, NULL, factory_thread, 0);
	}
}
