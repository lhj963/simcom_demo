#include "mcu.h"

#define ONE_BAG_DATA_SIZE 1024

unsigned int s_send_file_len = 0;
unsigned char *pfile = 0;
unsigned int mcu_file_size = 0;

// unsigned char s_start_mcu_update_flag = 0;

#if 0
char is_mcu_update_file_exsit(void)
{
	FILE *fp;
	fp = fopen("/data/mcu.bin","r");
	if (fp == 0)
	{
		return 0;
	}
	fclose(fp);
	return 1;
}
#endif
void send_msg_to_mcu(unsigned int msgid, void *content, unsigned int len)
{
	unsigned int rlen = 0;
	unsigned char *sendbuf = 0;

	sendbuf = malloc(3072);
	if (sendbuf)
	{
		rlen = tccp_pack(msgid, content, len, sendbuf, 3072);

		// need sem
		uart_send_bytes(0, sendbuf, rlen);

		free(sendbuf);
	}
	else
	{
		printf("[send_msg_to_mcu]mem error\r\n");
	}
}

void mpu_ack_mcu(unsigned int msgid, void *content, unsigned int len)
{
	unsigned char *ptr = 0;
	unsigned int rlen = 0;

	ptr = malloc(3072);
	if (ptr)
	{
		ptr[rlen++] = msgid >> 8;
		ptr[rlen++] = msgid;
		if (content && len)
		{
			memcpy(ptr + rlen, content, len);
			rlen += len;
		}
		send_msg_to_mcu(TCCP_MPU_ACK_MCU_MSG_ID, ptr, rlen);

		free(ptr);
	}
	else
	{
		printf("[mpu_ack_mcu]mem error\r\n");
	}
}

void send_start_mcu_update_msg(void)
{
	unsigned int rlen = 0;
	unsigned char sendbuf[128] = {0};
	unsigned char ptr[64] = {0};
	unsigned int xlen = 0;

	s_send_file_len = 0;
	if (pfile == 0)
	{
		pfile = (unsigned char *)malloc(256 * 1024);
		if (pfile == 0)
			return;

		if (pfile)
		{
			FILE *fp;
			fp = fopen("/data/mcu.bin", "rb+");
			if (fp == 0)
			{
				free(pfile);
				pfile = 0;
				return;
			}
			mcu_file_size = fread(pfile, 1, 256 * 1024, fp);
			fclose(fp);
		}
	}

	set_update_mcu_mode(1);
	rlen = tccp_pack(TCCP_FILE_SEND_START_MSG_ID, ptr, xlen, sendbuf, sizeof(sendbuf));
	// need sem
	uart_send_bytes(0, sendbuf, rlen);
}

void send_update_file_to_mcu(void)
{
	unsigned char tempbuf[ONE_BAG_DATA_SIZE] = {0};
	unsigned char sendbuf[ONE_BAG_DATA_SIZE + 128] = {0};
	unsigned int rlen = 0;
	int xlen = 0;

	if ((pfile == 0) || (mcu_file_size == 0))
	{
		if (pfile)
			free(pfile);
		return;
	}

	if (mcu_file_size >= (s_send_file_len + ONE_BAG_DATA_SIZE))
	{
		xlen = ONE_BAG_DATA_SIZE;
	}
	else
	{
		xlen = mcu_file_size - s_send_file_len;
	}

	if (xlen > 0)
		memcpy(tempbuf, pfile + s_send_file_len, xlen);

	if (xlen <= 0)
	{
		// free the file mem
		if (pfile)
			free(pfile);
		pfile = 0;

		system("rm -rf /data/mcu.bin");
		system("rm -rf /data/box.bin");
		system("sync");

		// send finish
		rlen = tccp_pack(TCCP_FILE_SEND_FINISH_MSG_ID, 0, 0, sendbuf, sizeof(sendbuf));
		// need sem
		uart_send_bytes(0, sendbuf, rlen);

		set_update_mcu_mode(0);
		return;
	}

	rlen = tccp_pack(TCCP_SEND_FILE_MSG_ID, tempbuf, xlen, sendbuf, sizeof(sendbuf));
	// need sem
	uart_send_bytes(0, sendbuf, rlen);

	s_send_file_len += xlen;
}

void get_all_file_in_dir(void *floder_path)
{
	DIR *pdir;
	struct dirent *pDirent = NULL;
	struct stat dirstat;

	if (floder_path == 0)
	{
		return;
	}

	if (stat((char *)floder_path, &dirstat) == -1)
	{
		DBG_Print(LEVEL2, "3cannot access %s\r\n", (char *)floder_path);
		return;
	}

	if (dirstat.st_mode & __S_IFDIR)
	{
		pdir = opendir((char *)floder_path);
		while ((pDirent = readdir(pdir)) != NULL)
		{
			if (strcmp(pDirent->d_name, ".") && strcmp(pDirent->d_name, ".."))
			{
				DBG_Print(LEVEL2, "[%s]%s\r\n", (char *)floder_path, pDirent->d_name);
			}
		}
		closedir(pdir);
	}
}

char is_cmd_mpu_ver_ok(char *cmd)
{
	char tempbuf[48] = {0};
	char cur_ver[48] = {0};
	unsigned int cver = 1;
	unsigned int dver = 0;

	GetSubString(SOFTWARE_VERSION, "MPU_V", ".", cur_ver, sizeof(cur_ver));
	if (GetSubString(cmd, "MPU_V", ".bin", tempbuf, sizeof(tempbuf)))
	{
	}
	else
	{
		if (GetSubString(cmd, "MPU_V", ".BIN", tempbuf, sizeof(tempbuf)))
		{
		}
	}

	cver = atoi(cur_ver);
	dver = atoi(tempbuf);
	if (cver == dver)
	{
		return 1;
	}
	// printf("the big ver is not same(cur:%u des:%u)\r\n",cver,dver);
	return 0;
}

char check_mpu_file(void)
{
	unsigned int total_size = 0;
	unsigned int crc32 = 0;
	unsigned int read_crc32 = 0;
	unsigned int rlen = 0;
	unsigned int offset = 0;
	unsigned char *pbuf = 0;
	unsigned int buflen = 1024 * 64;

	total_size = mygetfilesize("/data/mpu.bin");
	if (total_size < (20 * 1024))
	{
		return 0;
	}

	pbuf = (unsigned char *)malloc(buflen);
	if (pbuf == 0)
	{
		return 0;
	}

	rlen = myreadfile1("/data/mpu.bin", total_size - 4, pbuf, buflen);
	read_crc32 = (pbuf[0] << 24) + (pbuf[1] << 16) + (pbuf[2] << 8) + pbuf[3];

	while (1)
	{
		rlen = myreadfile1("/data/mpu.bin", offset, pbuf, (buflen >= (total_size - 4 - offset)) ? (total_size - 4 - offset) : buflen);
		if (rlen == 0)
		{
			break;
		}
		if (offset == 0)
		{
			// the file format is error
			if (memcmp(pbuf + 1, "ELF", 3))
			{
				if (pbuf)
					free(pbuf);
				return 0;
			}
		}
		crc32 = calc_crc32(crc32, pbuf, rlen);

		offset += rlen;
	}

	if (pbuf)
		free(pbuf);

	if (crc32 != read_crc32)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

unsigned char has_mcu_firmware_flag = 0;
void mcu_firmware_proc(void *pdat, unsigned int datlen)
{
	if (datlen)
	{
		has_mcu_firmware_flag = 1;
	}
	else
	{
		has_mcu_firmware_flag = 0;
	}

	system("rm -rf /data/mcu.bin");
	DBG_SendDataEx("/data/mcu.bin", pdat, datlen);
}

void mpu_firmware_proc(void *pdat, unsigned int datlen)
{
	system("mount -o remount rw /");
	system("rm -rf /data/mpu.bin");
	DBG_SendDataEx("/data/mpu.bin", pdat, datlen);

	system("cp /data/mpu.bin /usr/bin/time_daemon");
	system("chmod 777 /usr/bin/time_daemon");
}

void cfg_firmware_proc(void *pdat, unsigned int datlen)
{
	if (pdat && datlen)
	{
		mywritefile(0, "/data/sysparam.cfg", pdat, datlen);
	}
}

void lib_firmware_proc(void *filename, void *pdat, unsigned int datlen)
{
	box_install_library((const char *)filename, pdat, datlen);
}

void update_box_firmware(void)
{
	unsigned char *ptr = 0;
	unsigned int buflen = (1024 * 1024 * 6);
	unsigned int rlen = 0;
	char flag = 0;
	int ret = 0;

	ptr = (unsigned char *)malloc(buflen);
	if (ptr)
	{
		has_mcu_firmware_flag = 0;
		rlen = myreadfile("/data/box.bin", ptr, buflen);

		ret = box_firmware_unpack(ptr, rlen, mcu_firmware_proc, mpu_firmware_proc, cfg_firmware_proc, lib_firmware_proc);
		printf("unpack ret:%d\r\n", ret);
		if (ret == 0)
		{
			flag = 1;
		}
		free(ptr);
	}

	if (flag)
	{
		system("sync");

		if (has_mcu_firmware_flag)
		{
			send_start_mcu_update_msg();
		}
		else
		{
			system("reboot");
		}
	}
}

int create_blind_file(unsigned char channel, void *pdat, unsigned int datlen)
{
	static unsigned long long s_file_idx = 0;
	char fn[96] = {0};
	unsigned char *pt = 0;
	unsigned long ms = 0;

	if ((channel >= 7) || (pdat == 0) || (datlen == 0))
	{
		return 1;
	}

	if (get_main_power_off_flag())
	{
		return 3;
	}

	if (is_emmc_ok() == 0)
	{
		return 2;
	}

	pt = get_rtc_time();
	ms = (unsigned long)(get_run_time_ms());

	s_file_idx++;
	snprintf(fn, sizeof(fn), "/media/card/blind/ch%u/%02u%02u%02u%02u%02u%02u_%lu_%lu.dat", channel,
			 pt[0], pt[1], pt[2], pt[3], pt[4], pt[5], (long unsigned int)s_file_idx, (long unsigned int)ms);
	mywritefile(0, fn, pdat, datlen);
	return 0;
}

void ftp_upload_cb(unsigned int ret)
{
	char tip[64] = {0};
	unsigned int rlen = 0;

	rlen = sprintf(tip, "ftp upload ret:%u\r\n", ret);
	mpu_ack_mcu(TCCP_MCU_TO_MPU_EXE_LINUX_CMD_MSG_ID, tip, rlen);
}

typedef struct tagOTAFileInfo
{
	unsigned int start_addr[16];
	unsigned int block_size[16];
	unsigned int block_cnt;
	unsigned int lingjian_id;
} OTAFileInfo;

int analyze_sandian_ota_cfg_file(char *file_path, OTAFileInfo *drv_blk, OTAFileInfo *app_blk)
{
	unsigned int rlen = 0;
	char *pbuf = 0;
	char *pdrv = 0;
	char *papp = 0;
	char *ptemp = 0;
	char *ptemp1 = 0;
	char *pdh = 0;
	char *ptail = 0;
	int ret = 0;
	unsigned int addr = 0;
	unsigned int block_size = 0;
	unsigned int idx = 0;
	char tempbuf[64] = {0};
	char a1[32] = {0};
	char a2[32] = {0};
	unsigned char dat[16] = {0};
	unsigned char cnt = 0;
	unsigned int templen = 0;

	if ((drv_blk == 0) || (app_blk == 0))
	{
		return -1;
	}

	memset(drv_blk, 0, sizeof(OTAFileInfo));
	memset(app_blk, 0, sizeof(OTAFileInfo));

	pbuf = (char *)malloc(1024 * 5);
	if (pbuf)
	{
		rlen = myreadfile_readonly(file_path, pbuf, 1024 * 5);
		if (rlen)
		{
			pdrv = strstr(pbuf, "drv");
			papp = strstr(pbuf, "app");
			ptemp = pbuf;

			while (1)
			{
				ptemp1 = GetSubString(ptemp, "{", "}", tempbuf, sizeof(tempbuf));
				if (ptemp1)
				{
					pdh = strstr(ptemp1, ",");
					ptail = strstr(ptemp1, "}");

					if (pdh && ptail)
					{
						memset(a1, 0, sizeof(a1));
						memset(a2, 0, sizeof(a2));
						templen = pdh - ptemp1 - 1;
						if (templen >= sizeof(a1))
						{
							templen = sizeof(a1) - 1;
						}
						if (pdh > (ptemp1 + 1))
							memcpy(a1, ptemp1 + 1, templen);

						templen = ptail - pdh - 1;
						if (templen >= sizeof(a2))
						{
							templen = sizeof(a2) - 1;
						}
						if (ptail > (pdh + 1))
							memcpy(a2, pdh + 1, templen);
						// printf("tempbuf:%s a1:%s a2:%s\r\n",tempbuf,a1,a2);
					}

					cnt = HexStrToArray(a1, dat, sizeof(dat));
					if (cnt == 4)
					{
						addr = (dat[0] << 24) + (dat[1] << 16) + (dat[2] << 8) + dat[3];
					}
					else if (cnt == 3)
					{
						addr = (dat[0] << 16) + (dat[1] << 8) + dat[2];
					}
					else if (cnt == 2)
					{
						addr = (dat[0] << 8) + dat[1];
					}
					else if (cnt == 1)
					{
						addr = dat[0];
					}

					cnt = HexStrToArray(a2, dat, sizeof(dat));
					if (cnt == 4)
					{
						block_size = (dat[0] << 24) + (dat[1] << 16) + (dat[2] << 8) + dat[3];
					}
					else if (cnt == 3)
					{
						block_size = (dat[0] << 16) + (dat[1] << 8) + dat[2];
					}
					else if (cnt == 2)
					{
						block_size = (dat[0] << 8) + dat[1];
					}
					else if (cnt == 1)
					{
						block_size = dat[0];
					}

					// printf("addr:%u size:%u ptemp1:%08X papp:%08X pdrv:%08X\r\n",addr,block_size,ptemp1,papp,pdrv);
					if (ptemp1 > papp) // app block info
					{
						idx = app_blk->block_cnt;
						app_blk->start_addr[idx] = addr;
						app_blk->block_size[idx] = block_size;
						app_blk->block_cnt++;
					}
					else
					{
						if (ptemp1 > pdrv) // drv block info
						{
							idx = drv_blk->block_cnt;
							drv_blk->start_addr[idx] = addr;
							drv_blk->block_size[idx] = block_size;
							drv_blk->block_cnt++;
						}
						else
						{
							if (strstr(tempbuf, "lingjianid:ld_bms"))
							{
								drv_blk->lingjian_id = 4;
								app_blk->lingjian_id = 4;
							}
						}
					}
					ptemp = ptemp1 + 3;
				}
				else
				{
					break;
				}
			}
		}
		else
		{
			ret = -2;
		}
		free(pbuf);
	}
	else
	{
		ret = -3;
	}
	return ret;
}

int func_test(unsigned int func_test_id)
{
	switch (func_test_id)
	{
	case 1:
		system("reboot");
		break;
	case 2:
		send_start_mcu_update_msg();
		break;
	case 3:
		get_all_file_in_dir("/data/blind/ch0");
		break;
	case 4:
	{
		OTAFileInfo drv_info;
		OTAFileInfo app_info;

		if (analyze_sandian_ota_cfg_file("/data/sandian_bms/cfg.txt", &drv_info, &app_info) == 0)
		{
			printf("lingjianid:%u\r\n", drv_info.lingjian_id);
			printf("---drv info(block count:%u)---\r\n", drv_info.block_cnt);
			printf("blk0:%u,%u\r\n", drv_info.start_addr[0], drv_info.block_size[0]);

			printf("---app info(block count:%u)---\r\n", app_info.block_cnt);
			printf("blk0:%u,%u\r\n", app_info.start_addr[0], app_info.block_size[0]);
			printf("blk1:%u,%u\r\n", app_info.start_addr[1], app_info.block_size[1]);
			printf("blk2:%u,%u\r\n", app_info.start_addr[2], app_info.block_size[2]);
			printf("blk3:%u,%u\r\n", app_info.start_addr[3], app_info.block_size[3]);
			printf("blk4:%u,%u\r\n", app_info.start_addr[4], app_info.block_size[4]);
			printf("blk5:%u,%u\r\n", app_info.start_addr[5], app_info.block_size[5]);
			printf("blk6:%u,%u\r\n", app_info.start_addr[6], app_info.block_size[6]);
			printf("blk7:%u,%u\r\n", app_info.start_addr[7], app_info.block_size[7]);
			printf("blk8:%u,%u\r\n", app_info.start_addr[8], app_info.block_size[8]);
		}
	}
	break;
	case 5:
		system("cp /data/mpu.bin /usr/bin/time_daemon");
		system("chmod 777 /usr/bin/time_daemon");
		system("sync");
		system("reboot");
		break;
	case 6:
	{
		system("echo 22 > /sys/class/gpio/export");
		system("echo 23 > /sys/class/gpio/export");
		system("echo 24 > /sys/class/gpio/export");
		system("echo 25 > /sys/class/gpio/export");

		system("echo 1 > /sys/class/gpio/gpio22/value");
		system("echo 1 > /sys/class/gpio/gpio23/value");
		system("echo 1 > /sys/class/gpio/gpio24/value");
		system("echo 1 > /sys/class/gpio/gpio25/value");

		system("echo out > /sys/class/gpio/gpio22/direction");
		system("echo out > /sys/class/gpio/gpio23/direction");
		system("echo out > /sys/class/gpio/gpio24/direction");
		system("echo out > /sys/class/gpio/gpio25/direction");
	}
	break;
	case 7:
	{
		system("echo 0 > /sys/class/gpio/gpio22/value");
		system("echo 0 > /sys/class/gpio/gpio23/value");
		system("echo 0 > /sys/class/gpio/gpio24/value");
		system("echo 0 > /sys/class/gpio/gpio25/value");

		system("echo out > /sys/class/gpio/gpio22/direction");
		system("echo out > /sys/class/gpio/gpio23/direction");
		system("echo out > /sys/class/gpio/gpio24/direction");
		system("echo out > /sys/class/gpio/gpio25/direction");
	}
	break;
	case 8:
	{
		unsigned int rlen = 0;
		unsigned int i = 0;
		unsigned char buf[512] = {0};
		unsigned char pdat[] = {0x23, 0x23, 0x01, 0x4C, 0x47, 0x44, 0x43, 0x48, 0x41, 0x31, 0x47, 0x38, 0x4D, 0x41, 0x31, 0x32,
								0x37, 0x37, 0x33, 0x33, 0x01, 0x01, 0x00, 0x1C, 0x15, 0x08, 0x0E, 0x08, 0x2F, 0x16, 0x00, 0x01,
								0x38, 0x39, 0x38, 0x36, 0x30, 0x34, 0x39, 0x33, 0x31, 0x39, 0x32, 0x31, 0x39, 0x30, 0x34, 0x39,
								0x34, 0x33, 0x32};

		// dx83_test3();
		rlen = sm4_enc1(0, pdat, sizeof(pdat), buf, 512);
		for (i = 0; i < rlen; i++)
		{
			printf("%02X", buf[i]);
		}
		printf("\r\n");

		// gzip_file("/data/1.txt","/data/1.txt.gz");
	}
	break;
	case 9:

		break;
	case 10:
	{
		unsigned char sm4_key[] = {
			0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10,
			0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10};

		printf("import sm4 test key\r\n");
		import_sm4_key(sm4_key, 32);
	}
	break;
	case 11:
		handshake_plat(sysparam_get_vin(), "222.128.13.25", 9008);
		break;
	case 12:
		dx83_press_test();
		break;
	case 13:
		send_sms("1064899120154", "just test ght sms", strlen("just test ght sms"));
		break;
	case 14:

		get_disk_info("/media/card/", 0, 0, 0);

		break;
	case 15:
		set_ppp_status(0, 1);
		break;

	case 16:
	{
		char buf[256] = {0};
		unsigned int rlen = 0;

		rlen = tccp_pack(0x01, "\x33\x33\x33\x33\x33\x33", 6, buf, 256);
		DBG_PrintBuf(LEVEL1, "create tccp pack:\r\n", buf, rlen);
	}
	break;
	case 17:
	{
		unsigned char buf[64] =
			{
				0xA5,
				0x5A,
				0x00,
				0x01,
				0x00,
				0x06,
			};

		unsigned char buf1[64] =
			{
				0xA5,
				0x5A,
				0x00,
				0x01,
				0x00,
				0x06,
				0x33,
				0x33,
				0x33,
				0x33,
				0x33,
				0x33,
				0xF8,
				0xA5,

			};

		unsigned char buf2[64] =
			{
				0xA5,
				0x5A,
				0x00,
				0x01,
				0x00,
				0x06,
				0x33,
				0x33,
				0x33,
				0x33,
				0x33,
				0x33,
			};
		unsigned char buf3[64] =
			{
				0xA5, 0x5A, 0x00, 0x01, 0x00, 0x06, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0xF8,
				0xA5, 0x5A, 0x00, 0x01, 0x00, 0x06, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0xF8,
				0xA5, 0x5A, 0x00, 0x01, 0x00, 0x06, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0xF8,
				0xA5, 0x5A, 0x00, 0x01, 0x00, 0x06, 0x33, 0x33, 0x33};
		unsigned int rlen = 0;

		rlen = tccp_unpack1(buf, 6, 0, 0);
		DBG_PrintBuf(LEVEL1, "remain0:\r\n", buf, rlen);

		rlen = tccp_unpack1(buf1, 13 + 1, 0, 0);
		DBG_PrintBuf(LEVEL1, "remain1:\r\n", buf1, rlen);

		rlen = tccp_unpack1(buf2, 12, 0, 0);
		DBG_PrintBuf(LEVEL1, "remain2:\r\n", buf2, rlen);

		rlen = tccp_unpack1(buf3, 13 * 3 + 9, 0, 0);
		DBG_PrintBuf(LEVEL1, "remain3:\r\n", buf3, rlen);
	}
	break;
	case 18:
		printf("start wget cmd\r\n");
		// wget ftp://wugk:123456@192.168.0.88/auto_LNMP.sh

		// system("wget -O /data/test_ota.zip ftp://222.128.13.25:8121/1476393798278971392_1478673173863071744.zip --ftp-user=users --ftp-password=123456");
		system("wget -O /data/test_ota.zip ftp://users:123456@222.128.13.25:8121/1476393798278971392_1478673173863071744.zip");

		printf("exe wget cmd finish\r\n");
		break;
	case 19:
	{
		unsigned char buf[] =
			{
				0x00,
				0x00,
				0x00,
				0x00,
				0x00,
				0x00,
				0x00,
				0x00,
				0x44,
				0x66,
				0x74,
				0x70,
				0x3A,
				0x2F,
				0x2F,
				0x32,
				0x32,
				0x32,
				0x2E,
				0x31,
				0x32,
				0x38,
				0x2E,
				0x31,
				0x33,
				0x2E,
				0x32,
				0x35,
				0x3A,
				0x38,
				0x31,
				0x32,
				0x31,
				0x2F,
				0x31,
				0x34,
				0x37,
				0x36,
				0x33,
				0x39,
				0x33,
				0x37,
				0x39,
				0x38,
				0x32,
				0x37,
				0x38,
				0x39,
				0x37,
				0x31,
				0x33,
				0x39,
				0x32,
				0x5F,
				0x31,
				0x34,
				0x37,
				0x38,
				0x36,
				0x37,
				0x33,
				0x31,
				0x37,
				0x33,
				0x38,
				0x36,
				0x33,
				0x30,
				0x37,
				0x31,
				0x37,
				0x34,
				0x34,
				0x2E,
				0x7A,
				0x69,
				0x70,
				0x05,
				0x75,
				0x73,
				0x65,
				0x72,
				0x73,
				0x06,
				0x31,
				0x32,
				0x33,
				0x34,
				0x35,
				0x36,
				0x7F,
				0x40,
				0x3A,
				0xD3,
				0x57,
				0xEB,
				0x93,
				0x00,
				0x29,
				0x15,
				0xA9,
				0x0C,
				0x3B,
				0x84,
				0xB1,
				0x9A,
				0x00,
				0x00,
				0x00,
				0x00,
				0x00,
				0x00,
				0x00,
				0x00,
				0x00,
				0x00,
				0x00,
				0x00,
				0x00,
				0x00,
				0x00,
				0x00,
			};
		jmrh_analyze_plat_data(buf, sizeof(buf));
	}
	break;
	case 20:
		unpack_ota_file("/data/test_ota.zip", "/data/test_ota.bin");
		break;
	case 21:

		break;
	case 22:
	{
		unsigned char *ptr = 0;

		ptr = (unsigned char *)malloc(1024 * 1024 * 5);
		if (ptr == 0)
		{
			DBG_Print(LEVEL2, "malloc error\r\n");
		}
		else
		{
			free(ptr);
			DBG_Print(LEVEL2, "malloc ok\r\n");
		}
	}
	break;
	case 23:
		update_box_firmware();
		break;

	case 24:
	{
		unsigned char buf[128] = {0};
		unsigned char rlen = 0;

		dx83_get_public_key(get_public_key_ptr(), 64);

		DBG_PrintBuf(LEVEL1, "public key:\r\n", get_public_key_ptr(), 64);

		rlen = get_sign("\x23\x23\x01\x33\x33\x33\x33\x33\x33\x33\x33\x33\x33\x33\x33\x33\x33", 17, buf, sizeof(buf));
		DBG_PrintBuf(LEVEL1, "get sign:\r\n", buf, rlen);
	}

	break;
	case 25:
		car_to_car("BYSD0000000000003", "BYSD0000000000005");
		break;

	case 26:
	{
		int i = 0;
		char cmd[160] = {0};
		char cur_file_name[32] = {0};
		unsigned char *pt = get_rtc_time();

		sprintf(cur_file_name, "vlog%02d%02d%02d.txt", pt[0], pt[1], pt[2]);
		for (i = 0; i < 320; i++)
		{
			sprintf(cmd, "cp /media/card/vlog/%s /media/card/vlog/vlog210%03d.txt", cur_file_name, i);
			system(cmd);
		}
	}
	break;
	case 27:
		ftp_download(0, "ercpftp", "DFercp2020");

		break;

	case 28:
		ftp_upload("129.211.170.236", 21, "ftpusr", "C7qoFgmzLXQ8Qwtk", "/media/card/can/can22020403.dat", ftp_upload_cb);
		break;

	case 30:
		printf("first\r\n");
		system("echo 0 > /sys/devices/78d9000.usb/power_supply/usb/present");
		usleep(5000);
		system("echo 1 > /sys/devices/78d9000.usb/power_supply/usb/present");
		usleep(5000);
		system("echo 0 > /sys/devices/78d9000.usb/power_supply/usb/present");
		break;
	case 31:
		printf("enter sleep\r\n");

#if 1
		system("echo 2 > /sys/class/gpio/export");
		system("echo 3 > /sys/class/gpio/export");
		system("echo 38 > /sys/class/gpio/export");

		system("echo 0 > /sys/class/gpio/gpio2/value");
		system("echo 0 > /sys/class/gpio/gpio3/value");
		system("echo 0 > /sys/class/gpio/gpio38/value");

		system("echo out > /sys/class/gpio/gpio2/direction");
		system("echo out > /sys/class/gpio/gpio3/direction");
		system("echo out > /sys/class/gpio/gpio38/direction");
#endif

		// system("echo 0 > /sys/devices/78d9000.usb/power_supply/usb/present");
		// system("echo mem > /sys/power/state");
		break;
	default:
		break;
	}

	return 0;
}

unsigned int sandian_file_size = 0;
unsigned char *p_sandian_file = 0;

unsigned int get_sandian_file_data(void *file_path, unsigned int offset, void *pbuf, unsigned int wantlen)
{
	unsigned int templen = 0;

	if ((pbuf == 0) || (wantlen == 0) || (file_path == 0))
		return 0;

	if (offset == 0)
	{
		p_sandian_file = malloc(512 * 1024);
		sandian_file_size = myreadfile1("/data/sandian_mcu/mcu_drv.bin", offset, p_sandian_file, 512 * 1024);
		memcpy(pbuf, p_sandian_file, wantlen);
		return wantlen;
	}
	else
	{

		memcpy(pbuf, p_sandian_file + offset, templen);
		return templen;
	}
}

void analyze_mcu_data(unsigned int msgid, void *content, unsigned int contentlen)
{
	unsigned char *ptr = (unsigned char *)content;
	switch (msgid)
	{
	case TCCP_MCU_TO_MPU_TEST_MSG_ID:
	{
		unsigned int func_test_id = 0;
		if (ptr && (contentlen >= 2))
		{
			func_test_id = (ptr[0] << 8) + ptr[1];
			printf("func_test_id:%u\r\n", func_test_id);

			func_test(func_test_id);
		}
	}
	break;

	case TCCP_MCU_SEND_NET_DATA_MSG_ID:
		if (ptr && (contentlen > 1) && (ptr[0] < 9) && sysparam_get_serverport(ptr[0]))
		{
			// port is 0  means the link is not avail

			if ((sysparam_get_func_select() == FuncSelect_JMRH_SM4) && ((ptr[0] == 1) || (ptr[0] == 2) || (ptr[0] == 5)))
			{
				// limit the blind file cnt
				if (get_blind_data_cnt(ptr[0]) < 200000)
					create_blind_file(ptr[0], ptr + 1, contentlen - 1);
			}
			else
			{
				if (get_server_connect_status(ptr[0]) && (get_dbm() > -90))
				{
					push_data_send_to_buffer(ptr[0], ptr + 1, contentlen - 1);
				}
				else
				{
					// limit the blind file cnt
					if (get_blind_data_cnt(ptr[0]) < 200000)
						create_blind_file(ptr[0], ptr + 1, contentlen - 1);
				}
			}
		}
		break;

	case TCCP_FILE_SEND_NEXT_MSG_ID: // send next bag to mcu
		send_update_file_to_mcu();
		break;

	case TCCP_MCU_TO_MPU_PARAM_MSG_ID:

		if (memcmp(sysparam_get_vin(), ptr, 17))
		{
			memcpy(sysparam_get_vin(), ptr, 17);
		}
		break;

	case TCCP_MCU_TO_MPU_EXE_LINUX_CMD_MSG_ID:
		if (ptr && contentlen && (contentlen <= 384))
		{
			unsigned char update_mcu_flag = 0;
			unsigned char update_mpu_flag = 0;
			unsigned char update_box_flag = 0;
			unsigned char update_sandian_vcu_flag = 0;
			unsigned char update_sandian_mcu_flag = 0;
			unsigned char update_sandian_bms_flag = 0;

			unsigned int rlen = 0;
			char cmd[384 + 1] = {0};
			char url[384 + 1] = {0};
			char tip[64] = {0};
			char ack[1024 + 1] = {0};
			unsigned int ack_len = 0;
			int restart_sync = 0;
			int cmd_rc = 0;
			const char *out_path = 0;

			/*
			 * MCU framing is only a header plus XOR. Never pass that payload to a shell.
			 * Allow reboot/sync, a fixed diagnostic set, and wget -O to known OTA paths.
			 */
			cmd_rc = mcu_exec_allowlisted_cmd(ptr, contentlen, &out_path, url, sizeof(url), ack, sizeof(ack), &ack_len, &restart_sync);
			if (cmd_rc == 2)
			{
				rlen = snprintf(tip, sizeof(tip), "execute ok\r\n");
				mpu_ack_mcu(TCCP_MCU_TO_MPU_EXE_LINUX_CMD_MSG_ID, tip, rlen);
				mcu_run_restart(restart_sync);
			}
			else if (cmd_rc == 0)
			{
				if (ack_len == 0)
				{
					ack_len = snprintf(ack, sizeof(ack), "execute ok\r\n");
				}
				mpu_ack_mcu(TCCP_MCU_TO_MPU_EXE_LINUX_CMD_MSG_ID, ack, ack_len);

				if (out_path && (strcmp(out_path, "/data/mcu.bin") == 0))
				{
					update_mcu_flag = 1;
				}
				if (out_path && (strcmp(out_path, "/data/mpu.bin") == 0) && is_cmd_mpu_ver_ok(url))
				{
					update_mpu_flag = 1;
				}
				if (out_path && (strcmp(out_path, "/data/box.bin") == 0) && is_cmd_mpu_ver_ok(url))
				{
					update_box_flag = 1;
				}
				if (out_path && (strcmp(out_path, "/data/sandian_mcu.zip") == 0))
				{
					update_sandian_mcu_flag = 1;
				}
				if (out_path && (strcmp(out_path, "/data/sandian_vcu.zip") == 0))
				{
					update_sandian_vcu_flag = 1;
				}
				if (out_path && (strcmp(out_path, "/data/sandian_bms.zip") == 0))
				{
					update_sandian_bms_flag = 1;
				}
			}
			else
			{
				rlen = snprintf(tip, sizeof(tip), "execute rejected\r\n");
				mpu_ack_mcu(TCCP_MCU_TO_MPU_EXE_LINUX_CMD_MSG_ID, tip, rlen);
			}

			if (update_mcu_flag)
			{
				if (mygetfilesize("/data/mcu.bin") > 4096)
				{
					send_start_mcu_update_msg();
				}
			}

			if (update_mpu_flag)
			{
				if (check_mpu_file()) // check the file
				{
					if (get_main_power_off_flag() == 0)
					{

						system("mount -o remount rw /");
						system("cp /data/mpu.bin /usr/bin/time_daemon");
						system("chmod 777 /usr/bin/time_daemon");
						system("sync");
						system("reboot");
					}
				}
				else
				{
					char tip[128] = {0};

					rlen = snprintf(tip, sizeof(tip), "check mpu file error(size:%u)\r\n", mygetfilesize("/data/mpu.bin"));
					mpu_ack_mcu(TCCP_MCU_TO_MPU_EXE_LINUX_CMD_MSG_ID, tip, rlen);
				}
			}

			if (update_box_flag && (get_main_power_off_flag() == 0))
			{
				update_box_firmware();
			}

			if (update_sandian_mcu_flag)
			{
				unsigned int drv_file_size = 0;
				unsigned int app_file_size = 0;

				system("rm -rf /data/sandian_mcu");
				system("mkdir /data/sandian_mcu");
				system("unzip /data/sandian_mcu.zip -d /data/sandian_mcu/");

				system("mv /data/sandian_mcu/*boot*.bin /data/sandian_mcu/mcu_drv.bin");
				system("mv /data/sandian_mcu/*app*.bin /data/sandian_mcu/mcu_app.bin");

				cmd[0] = 0;
				cmd[1] = 1;

				// ev18
				cmd[2] = 0x00;
				cmd[3] = 0x00;
				cmd[4] = 0x98;
				cmd[5] = 0x00;

				// em26
				// cmd[2] = 0x70;
				// cmd[3] = 0x00;
				// cmd[4] = 0x01;
				// cmd[5] = 0x00;

				drv_file_size = mygetfilesize("/data/sandian_mcu/mcu_drv.bin");
				cmd[6] = drv_file_size >> 24;
				cmd[7] = drv_file_size >> 16;
				cmd[8] = drv_file_size >> 8;
				cmd[9] = drv_file_size;

				// ev18
				cmd[10] = 0x00;
				cmd[11] = 0x30;
				cmd[12] = 0x84;
				cmd[13] = 0x00;

				// em26
				// cmd[10] = 0x80;
				// cmd[11] = 0x02;
				// cmd[12] = 0x84;
				// cmd[13] = 0x00;

				app_file_size = mygetfilesize("/data/sandian_mcu/mcu_app.bin");
				cmd[14] = app_file_size >> 24;
				cmd[15] = app_file_size >> 16;
				cmd[16] = app_file_size >> 8;
				cmd[17] = app_file_size;

				if (drv_file_size || app_file_size)
				{
					send_msg_to_mcu(TCCP_MPU_TO_MCU_USER_MSG_ARRIVE_MSG_ID, cmd, 18);
				}
				else
				{
					system("rm -rf /data/sandian_mcu");
					system("rm -rf /data/sandian_mcu.zip");
				}
			}

			if (update_sandian_vcu_flag)
			{
				unsigned int drv_file_size = 0;
				unsigned int app_file_size = 0;

				system("rm -rf /data/sandian_vcu");
				system("mkdir /data/sandian_vcu");
				system("unzip /data/sandian_vcu.zip -d /data/sandian_vcu/");

				system("mv /data/sandian_vcu/*boot*.bin /data/sandian_vcu/vcu_drv.bin");
				system("mv /data/sandian_vcu/*app*.bin /data/sandian_vcu/vcu_app.bin");

				cmd[0] = 0;
				cmd[1] = 2;

				cmd[2] = 0x40;
				cmd[3] = 0x00;
				cmd[4] = 0x00;
				cmd[5] = 0x00;

				drv_file_size = mygetfilesize("/data/sandian_vcu/vcu_drv.bin");
				// drv_file_size = 0x1000;
				cmd[6] = drv_file_size >> 24;
				cmd[7] = drv_file_size >> 16;
				cmd[8] = drv_file_size >> 8;
				cmd[9] = drv_file_size;

				cmd[10] = 0x00;
				cmd[11] = 0x01;
				cmd[12] = 0x00;
				cmd[13] = 0x00;

				app_file_size = mygetfilesize("/data/sandian_vcu/vcu_app.bin");
				// app_file_size = 0x000F0000;
				cmd[14] = app_file_size >> 24;
				cmd[15] = app_file_size >> 16;
				cmd[16] = app_file_size >> 8;
				cmd[17] = app_file_size;

				if (drv_file_size || app_file_size)
				{
					send_msg_to_mcu(TCCP_MPU_TO_MCU_USER_MSG_ARRIVE_MSG_ID, cmd, 18);
				}
				else
				{
					system("rm -rf /data/sandian_vcu");
					system("rm -rf /data/sandian_vcu.zip");
				}
			}
			else if (update_sandian_bms_flag)
			{
				unsigned int drv_file_size = 0;
				unsigned int app_file_size = 0;
				OTAFileInfo drv_info = {0};
				OTAFileInfo app_info = {0};
				unsigned int tlen = 0;
				unsigned int i = 0;

				system("rm -rf /data/sandian_bms");
				system("mkdir /data/sandian_bms");
				system("unzip /data/sandian_bms.zip -d /data/sandian_bms/");

				system("mv /data/sandian_bms/*boot*.bin /data/sandian_bms/bms_drv.bin");
				system("mv /data/sandian_bms/*app*.bin /data/sandian_bms/bms_app.bin");

				if (analyze_sandian_ota_cfg_file("/data/sandian_bms/cfg.txt", &drv_info, &app_info) == 0)
				{
					cmd[tlen++] = 0;
					cmd[tlen++] = 4;

					cmd[tlen++] = drv_info.block_cnt;
					for (i = 0; i < drv_info.block_cnt; i++)
					{
						// 4byte addr
						cmd[tlen++] = drv_info.start_addr[i] >> 24;
						cmd[tlen++] = drv_info.start_addr[i] >> 16;
						cmd[tlen++] = drv_info.start_addr[i] >> 8;
						cmd[tlen++] = drv_info.start_addr[i];

						// 4byte block size
						cmd[tlen++] = drv_info.block_size[i] >> 24;
						cmd[tlen++] = drv_info.block_size[i] >> 16;
						cmd[tlen++] = drv_info.block_size[i] >> 8;
						cmd[tlen++] = drv_info.block_size[i];
					}

					cmd[tlen++] = app_info.block_cnt;
					for (i = 0; i < app_info.block_cnt; i++)
					{
						// 4byte addr
						cmd[tlen++] = app_info.start_addr[i] >> 24;
						cmd[tlen++] = app_info.start_addr[i] >> 16;
						cmd[tlen++] = app_info.start_addr[i] >> 8;
						cmd[tlen++] = app_info.start_addr[i];

						// 4byte block size
						cmd[tlen++] = app_info.block_size[i] >> 24;
						cmd[tlen++] = app_info.block_size[i] >> 16;
						cmd[tlen++] = app_info.block_size[i] >> 8;
						cmd[tlen++] = app_info.block_size[i];
					}

					if (drv_info.block_cnt || app_info.block_cnt)
					{
						send_msg_to_mcu(TCCP_MPU_TO_MCU_USER_MSG_ARRIVE_MSG_ID, cmd, tlen);
					}
					else
					{
						system("rm -rf /data/sandian_bms");
						system("rm -rf /data/sandian_bms.zip");
					}
				}
				else
				{
					cmd[0] = 0;
					cmd[1] = 3;

					cmd[2] = 0x40;
					cmd[3] = 0x02;
					cmd[4] = 0xC0;
					cmd[5] = 0x00;

					drv_file_size = mygetfilesize("/data/sandian_bms/bms_drv.bin");
					// drv_file_size = 0x1000;
					cmd[6] = drv_file_size >> 24;
					cmd[7] = drv_file_size >> 16;
					cmd[8] = drv_file_size >> 8;
					cmd[9] = drv_file_size;

					cmd[10] = 0x01;
					cmd[11] = 0x00;
					cmd[12] = 0x00;
					cmd[13] = 0x00;

					app_file_size = mygetfilesize("/data/sandian_bms/bms_app.bin");
					// app_file_size = 0x000F0000;
					cmd[14] = app_file_size >> 24;
					cmd[15] = app_file_size >> 16;
					cmd[16] = app_file_size >> 8;
					cmd[17] = app_file_size;
					if (drv_file_size || app_file_size)
					{
						send_msg_to_mcu(TCCP_MPU_TO_MCU_USER_MSG_ARRIVE_MSG_ID, cmd, 18);
					}
					else
					{
						system("rm -rf /data/sandian_bms");
						system("rm -rf /data/sandian_bms.zip");
					}
				}
			}
		}
		break;
	case TCCP_MCU_TO_MPU_STORE_CAN_DATA_MSG_ID:
	{
		char fn[128] = {0};
		unsigned char *pt = get_rtc_time();

		if ((contentlen > 4) && is_emmc_ok() && (get_main_power_off_flag() == 0))
		{
			if (ptr[0] && ptr[1])
			{
				sprintf(fn, "/media/card/can/can%02d%02d%02d%02d.dat", ptr[0], ptr[1], ptr[2], ptr[3]);
				DBG_SendDataEx(fn, ptr + 4, contentlen - 4);
			}
			else
			{
				if (is_time_format_ok(pt))
				{
					sprintf(fn, "/media/card/can/can%02d%02d%02d%02d.dat", pt[0], pt[1], pt[2], pt[3]);
					DBG_SendDataEx(fn, ptr + 4, contentlen - 4);
				}
			}
		}
	}
	break;

	case TCCP_MCU_TO_MPU_PARAM_EXT_MSG_ID:
	{
		unsigned short param_id = 0;
		// unsigned short templen = 0;
		unsigned short clen = 0;
		unsigned int xlen = 0;

		while (contentlen > 4)
		{
			param_id = (ptr[xlen++] << 8);
			param_id += ptr[xlen++];

			clen = (ptr[xlen++] << 8);
			clen += ptr[xlen++];

			switch (param_id)
			{
			case PARAM_ID_FROM_MCU_SOFTWARE_VERSION:
				if (clen)
					memcpy(get_mcu_ver(), ptr + xlen, (clen > 36) ? 36 : clen);
				break;
			case PARAM_ID_FROM_MCU_MAIN_POWER_VOL:
				set_main_power_vol((ptr[xlen] << 8) + ptr[xlen + 1]);
				break;
			case PARAM_ID_FROM_MCU_BAT_VOL:
				set_bat_vol((ptr[xlen] << 8) + ptr[xlen + 1]);
				break;
			case PARAM_ID_FROM_MCU_SERVER:
				sysparam_set_serveraddr(ptr[xlen], ptr + xlen + 3, clen - 3);
				sysparam_set_serverport(ptr[xlen], (ptr[xlen + 1] << 8) + ptr[xlen + 2]);
				break;
			case PARAM_ID_FROM_MCU_CHIP_ID:
				sysparam_set_jiami_chip_id(ptr + xlen, 16);
				break;
			case PARAM_ID_FROM_MCU_POSITION:
				DBG_Print(LEVEL2, "2analyze_mcu_data(ytes):\r\n");
				set_lon((ptr[xlen + 0] << 24) + (ptr[xlen + 1] << 16) + (ptr[xlen + 2] << 8) + ptr[xlen + 3]);
				set_lat((ptr[xlen + 4] << 24) + (ptr[xlen + 5] << 16) + (ptr[xlen + 6] << 8) + ptr[xlen + 7]);
				break;
			default:
				break;
			}

			xlen += clen;

			if (contentlen >= (2 + 2 + clen))
			{
				contentlen -= (2 + 2 + clen);
			}
			else
			{
				contentlen = 0;
			}
		}
	}
	break;

	case TCCP_MCU_TO_MPU_STATUS_MSG_ID:
		set_mcu_status1((ptr[0] << 24) + (ptr[1] << 16) + (ptr[2] << 8) + ptr[3]);
		break;

	case TCCP_MCU_TO_MPU_STORE_CACHE_MSG_ID:
		if (contentlen > 1)
		{
			push_data_to_cache(ptr[0], ptr + 1, contentlen - 1);
			// save_log("[store cache]",ptr + 1,contentlen - 1);
			if ((get_main_power_off_flag() == 0) && get_store_cache_data_flag() && (ptr[0] == 0) && (contentlen > 30))
			{
				char tip[32] = {0};

				snprintf(tip, 31, "<%02d-%02d-%02d %02d:%02d:%02d>", ptr[25], ptr[26], ptr[27], ptr[28], ptr[29], ptr[30]);
				save_cache_log(tip, ptr + 1, contentlen - 1);
			}
		}
		break;

	case TCCP_MCU_TO_MPU_GET_CACHE_MSG_ID:
	{
		unsigned int idx = 0;
		unsigned int xlen = 0;
		unsigned char *p = 0;

		idx = (ptr[1] << 24) + (ptr[2] << 16) + (ptr[3] << 8) + ptr[4];
		xlen = get_cache_data_len_by_idx(ptr[0], idx);
		if (xlen)
		{
			p = malloc(1536);
			if (p)
			{
				p[0] = ptr[0]; // cache id
				memcpy(p + 1, get_cache_data_by_idx(ptr[0], idx), xlen);
				send_msg_to_mcu(TCCP_MPU_TO_MCU_CACHE_DATA_ARRIVE_MSG_ID, p, xlen + 1);

				free(p);
			}
		}
	}
	break;
	case TCCP_MCU_TO_MPU_GET_SM2_SIGN_MSG_ID:
	{
		unsigned char buf[256] = {0};
		unsigned int rlen = 0;

		rlen = get_sign(ptr, contentlen, buf, 256);

		send_msg_to_mcu(TCCP_MPU_TO_MCU_SM2_SIGN_DATA_ARRIVE_MSG_ID, buf, rlen);

		DBG_PrintBufEx(LEVEL0, "/media/card/sign.txt", "raw data:", ptr, contentlen);
		DBG_PrintBufEx(LEVEL0, "/media/card/sign.txt", "sign:", buf, rlen);
	}

	break;
	case TCCP_MCU_TO_MPU_DBG_DISP_MSG_ID:
	{
		if (contentlen && ptr && (contentlen < 1500) && (get_main_power_off_flag() == 0))
		{
			// DBG_PrintBufEx(LEVEL0,"/data/dbg_tip.txt",0,ptr,contentlen);
			char *ptip = 0;
			unsigned int xlen = 0;

			ptip = malloc(1536);
			if (ptip)
			{
				memcpy(ptip, "{mcu:", strlen("{mcu:"));
				xlen += strlen("{mcu:");

				memcpy(ptip + xlen, ptr, contentlen);
				xlen += contentlen;

				ptip[xlen++] = '}';

				mywritefile(0, "/data/dbg_tip.txt", ptip, xlen);
				free(ptip);
			}
		}
	}
	break;

	case TCCP_MCU_TO_MPU_SAVE_LOG_MSG_ID:
		if (is_emmc_ok() && (get_main_power_off_flag() == 0))
			DBG_SendDataEx("/media/card/mcu_log.txt", ptr, contentlen);
		break;

	case TCCP_MCU_TO_MPU_SAVE_LOG_HEX_MSG_ID:
	{
		unsigned int xlen = 0;
		unsigned char *p = 0;

		p = malloc(1024 * 5);
		if (p)
		{
			xlen = ArrayToHexStr1(ptr, (contentlen > 2048) ? 2048 : contentlen, p, 5 * 1024);
			p[xlen++] = '\n';

			if (is_emmc_ok() && (get_main_power_off_flag() == 0))
				DBG_SendDataEx("/media/card/mcu_log.txt", p, xlen);
			free(p);
		}
	}
	break;

	case TCCP_MCU_TO_MPU_STORE_E_CHANGE_PARAM_MSG_ID:

		if (ptr && contentlen && (get_main_power_off_flag() == 0) && get_easy_change_param_init_flag())
		{
			mywritefile(0, "/data/mcu_e_param.dat", ptr, contentlen);
		}
		break;

	case TCCP_MCU_TO_MPU_FTP_UPLOAD_FILE_MSG_ID:
		// ftp_upload("129.211.170.236",21,"ftpusr","C7qoFgmzLXQ8Qwtk","/media/card/can/can22020403.dat",ftp_upload_cb);
		break;
	case TCCP_MCU_TO_MPU_FTP_UPLOAD_FILE1_MSG_ID:
	{
		if (contentlen < 512)
		{
			unsigned char *pbuf = 0;

			pbuf = malloc(512);
			if (pbuf)
			{
				memset(pbuf, 0, 512);
				memcpy(pbuf, ptr, contentlen);
				ftp_upload("129.211.170.236", 21, "ftpusr", "C7qoFgmzLXQ8Qwtk", pbuf, ftp_upload_cb);
				free(pbuf);
			}
		}
	}
	break;

	case TCCP_MCU_TO_MPU_GET_FILE_DATA_MSG_ID:
	{
		unsigned int xlen;
		unsigned int offset = 0;
		unsigned int wantlen = 0;
		unsigned char *pbuf = 0;

		pbuf = (unsigned char *)malloc(3072);
		if (pbuf)
		{
			offset = (ptr[1] << 24) + (ptr[2] << 16) + (ptr[3] << 8) + ptr[4];
			wantlen = (ptr[5] << 24) + (ptr[6] << 16) + (ptr[7] << 8) + ptr[8];
			if (wantlen > 2048)
			{
				wantlen = 2048;
			}

			// printf("filetype:%u offset:%u wantlen:%u\n",ptr[0],offset,wantlen);
			memcpy(pbuf, ptr, 5);

			switch (ptr[0])
			{
			case 1:
				xlen = myreadfile1("/data/sandian_mcu/mcu_drv.bin", offset, pbuf + 5, wantlen);
				// xlen = get_sandian_file_data("/data/sandian_mcu/mcu_drv.bin",offset,pbuf + 5,wantlen);
				break;
			case 2:
				xlen = myreadfile1("/data/sandian_mcu/mcu_app.bin", offset, pbuf + 5, wantlen);
				break;

			case 11:
				xlen = myreadfile1("/data/sandian_bms/bms_drv.bin", offset, pbuf + 5, wantlen);
				break;
			case 12:
				xlen = myreadfile1("/data/sandian_bms/bms_app.bin", offset, pbuf + 5, wantlen);
				break;

			case 21:
				xlen = myreadfile1("/data/sandian_vcu/vcu_drv.bin", offset, pbuf + 5, wantlen);
				break;
			case 22:
				xlen = myreadfile1("/data/sandian_vcu/vcu_app.bin", offset, pbuf + 5, wantlen);
				break;

			default:
				break;
			}

			if (xlen && (xlen <= 3072))
				send_msg_to_mcu(TCCP_MPU_TO_MCU_FILE_DATA_ARRIVE_MSG_ID, pbuf, xlen + 5);

			free(pbuf);
		}
	}
	break;
	case TCCP_MCU_TO_MPU_SEND_USER_MSG_MSG_ID: // mcu user msg

		break;

	case TCCP_MCU_TO_MPU_EXE_AT_CMD_MSG_ID:
		if (ptr && contentlen && (contentlen <= 1152))
		{
			char *pcmd = get_at_cmd_buf_ptr();

			memset(pcmd, 0, AT_CMD_BUF_LEN);
			memcpy(pcmd, ptr, contentlen);
			*(pcmd + contentlen) = '\r';
			*(pcmd + contentlen + 1) = '\n';
		}
		break;

	case TCCP_MCU_TO_MPU_SEND_SMS_MSG_ID:
		// printf("[send sms:%s]%s\r\n",ptr,ptr + 24);
		send_sms(ptr, ptr + 24, contentlen - 24);
		break;

	default:
		break;
	}
}

void mcu_error_data(void *pdat, unsigned int len)
{
#if 0
	char tip[64] = {0};
	unsigned char *pt = get_rtc_time();

	sprintf(tip,"(%02d-%02d-%02d %02d:%02d:%02d)mcu err msg:\r\n",pt[0],pt[1],pt[2],pt[3],pt[4],pt[5]);
	DBG_PrintBuf(LEVEL1,tip,pdat,len);
#endif
}

char is_mcu_update_file_ok(char *filename)
{
	if (mygetfilesize(filename) > 4096)
	{
		return 1;
	}

	return 0;
}

unsigned int mcu_data_arrive(void *pdat, unsigned int datlen)
{
	// unsigned char *ptr = (unsigned char *)pdat;
	static unsigned char s_mcu_init_flag = 0;
	unsigned int rlen = 0;

	unsigned int index = 0;

	DBG_Print(LEVEL2, "mcu data arrive(%uBytes):\r\n", datlen);
#if 1
	for (index = 0; index < datlen; index++)
	{
		DBG_Print(LEVEL2, "%02X", ((unsigned char *)pdat)[index]);
	}
	DBG_Print(LEVEL2, "\r\n");
#endif
	rlen = tccp_unpack1(pdat, datlen,
						analyze_mcu_data,
						mcu_error_data);

	if ((s_mcu_init_flag == 0) && (get_run_time() >= 6))
	{
		if (is_mcu_update_file_ok("/data/mcu.bin"))
		{
			send_start_mcu_update_msg();
		}
		else
		{
			remove("/data/mcu.bin");
		}
		s_mcu_init_flag = 1;
	}
	return rlen;
}

void get_time(unsigned char t[6])
{
	time_t tNow = 0;
	struct tm tmNow;

	time(&tNow);
	if (tNow < 0)
	{
		// perror("time");
		return;
	}

	ctime(&tNow);

	// memset(&tmNow, 0 , sizeof(tmNow));
	// gmtime_r(&tNow,&tmNow);
	// printf("gmtime: %d 年%d 月 %d 日%d 时 %d 分 %d 秒 \n",tmNow.tm_year+1900,tmNow.tm_mon+1,tmNow.tm_mday,tmNow.tm_hour+8,tmNow.tm_min,tmNow.tm_sec);

	memset(&tmNow, 0, sizeof(tmNow));
	localtime_r(&tNow, &tmNow);
	// printf("localtime: %d 年%d 月 %d 日%d 时 %d 分 %d 秒 \n",tmNow.tm_year+1900,tmNow.tm_mon+1,tmNow.tm_mday,tmNow.tm_hour,tmNow.tm_min,tmNow.tm_sec);

	t[0] = tmNow.tm_year + 1900 - 2000;
	t[1] = tmNow.tm_mon + 1;
	t[2] = tmNow.tm_mday;
	t[3] = tmNow.tm_hour;
	t[4] = tmNow.tm_min;
	t[5] = tmNow.tm_sec;
}

void send_status_to_mcu(void)
{
	unsigned int xlen = 0;
	unsigned char ptr[64] = {0};
	unsigned int temp = 0;

	// sta1
	temp = get_mpu_status1();
	ptr[xlen++] = temp >> 24;
	ptr[xlen++] = temp >> 16;
	ptr[xlen++] = temp >> 8;
	ptr[xlen++] = temp;

	// sta2
	ptr[xlen++] = 0;
	ptr[xlen++] = 0;
	ptr[xlen++] = 0;
	ptr[xlen++] = 0;
	// sta3
	ptr[xlen++] = 0;
	ptr[xlen++] = 0;
	ptr[xlen++] = 0;
	ptr[xlen++] = 0;
	// sta4
	ptr[xlen++] = 0;
	ptr[xlen++] = 0;
	ptr[xlen++] = 0;
	ptr[xlen++] = 0;
	send_msg_to_mcu(TCCP_MPU_TO_MCU_STATUS_MSG_ID, ptr, xlen);
}

void send_param_to_mcu(void)
{
	unsigned int xlen = 0;
	unsigned char ptr[384] = {0};
	unsigned int templen = 0;

	memcpy(ptr + xlen, sysparam_get_vin(), 17);
	xlen += 17;

	memcpy(ptr + xlen, get_iccid_ptr(), 20);
	xlen += 20;

	templen = strlen(get_imei_ptr());
	if (templen > 20)
	{
		templen = 20;
	}
	ptr[xlen++] = templen;
	memcpy(ptr + xlen, get_imei_ptr(), templen);
	xlen += templen;

	ptr[xlen++] = 0;

	send_msg_to_mcu(TCCP_MPU_SEND_PARAM_MSG_ID, ptr, xlen);
}

void monitor_mpu_time(void)
{
	static unsigned char s_last_time[6] = {60, 60, 60, 60, 60, 60};
	unsigned char *pt = get_rtc_time();
	unsigned char temp = 0;

	if (s_last_time[5] != pt[5])
	{
		if (s_last_time[5] != 60)
		{
			temp = s_last_time[5] + 1;
			if (temp >= 60)
				temp = 0;

			if (temp != pt[5])
			{
				unsigned char xlen = 0;
				char tip[128] = {0};

				xlen = sprintf(tip, "[mpu]last:%02d-%02d-%02d %02d:%02d:%02d cur:%02d-%02d-%02d %02d:%02d:%02d\r\n",
							   s_last_time[0], s_last_time[1], s_last_time[2], s_last_time[3], s_last_time[4], s_last_time[5],
							   pt[0], pt[1], pt[2], pt[3], pt[4], pt[5]);
				if (is_emmc_ok())
					DBG_SendDataEx("/media/card/mcu_log.txt", tip, xlen);
			}
		}

		memcpy(s_last_time, pt, 6);
	}
}
void send_heartbeat_to_mcu(void)
{
	unsigned int xlen = 0;
	unsigned char ptr[512] = {0};
	unsigned int templen = 0;
	unsigned int paramid = 0;
	unsigned int paramlen = 0;
	unsigned int dat = 0;
	unsigned int i = 0;
	static unsigned char s_public_key_report_flag = 0;

	// get_time(ptr);
	// set_rtc_time(ptr);

	memcpy(ptr, get_rtc_time(), 6);
	xlen += 6;
	// printf("linux time:%d %d %d %d %d %d\r\n",ptr[0],ptr[1],ptr[2],ptr[3],ptr[4],ptr[5]);

	templen = get_lon();
	ptr[xlen++] = templen >> 24;
	ptr[xlen++] = templen >> 16;
	ptr[xlen++] = templen >> 8;
	ptr[xlen++] = templen;

	templen = get_lat();
	ptr[xlen++] = templen >> 24;
	ptr[xlen++] = templen >> 16;
	ptr[xlen++] = templen >> 8;
	ptr[xlen++] = templen;

	paramid = PARAM_ID_FROM_MPU_SOFTWARE_VERSION;
	paramlen = strlen(SOFTWARE_VERSION);
	ptr[xlen++] = paramid >> 8;
	ptr[xlen++] = paramid;
	ptr[xlen++] = paramlen >> 8;
	ptr[xlen++] = paramlen;
	memcpy(ptr + xlen, SOFTWARE_VERSION, strlen(SOFTWARE_VERSION));
	xlen += strlen(SOFTWARE_VERSION);
#if 0
	paramid = PARAM_ID_FROM_MPU_GPS_SPEED;
	paramlen = 2;
	dat = get_gps_speed();
	ptr[xlen++] = paramid >> 8;
	ptr[xlen++] = paramid;
	ptr[xlen++] = paramlen >> 8;
	ptr[xlen++] = paramlen;
	ptr[xlen++] = dat >> 8;
	ptr[xlen++] = dat;
#endif
	paramid = PARAM_ID_FROM_MPU_SIM_SIGNAL_STRENGTH;
	paramlen = 2;
	dat = 0 - get_dbm();
	ptr[xlen++] = paramid >> 8;
	ptr[xlen++] = paramid;
	ptr[xlen++] = paramlen >> 8;
	ptr[xlen++] = paramlen;
	ptr[xlen++] = dat >> 8;
	ptr[xlen++] = dat;
#if 0
	paramid = PARAM_ID_FROM_MPU_GAODU;
	paramlen = 2;
	dat = get_gps_gaodu();
	ptr[xlen++] = paramid >> 8;
	ptr[xlen++] = paramid;
	ptr[xlen++] = paramlen >> 8;
	ptr[xlen++] = paramlen;
	ptr[xlen++] = dat >> 8;
	ptr[xlen++] = dat;

	paramid = PARAM_ID_FROM_MPU_ANGLE;
	paramlen = 2;
	dat = get_gps_angle();
	ptr[xlen++] = paramid >> 8;
	ptr[xlen++] = paramid;
	ptr[xlen++] = paramlen >> 8;
	ptr[xlen++] = paramlen;
	ptr[xlen++] = dat >> 8;
	ptr[xlen++] = dat;
#endif
	for (i = 0; i < 4; i++)
	{
		paramid = PARAM_ID_FROM_MPU_SERVER0_ADDR + (i * 2);
		paramlen = strlen(sysparam_get_serveraddr(i));
		ptr[xlen++] = paramid >> 8;
		ptr[xlen++] = paramid;
		ptr[xlen++] = paramlen >> 8;
		ptr[xlen++] = paramlen;
		memcpy(ptr + xlen, sysparam_get_serveraddr(i), strlen(sysparam_get_serveraddr(i)));
		xlen += strlen(sysparam_get_serveraddr(i));

		paramid = PARAM_ID_FROM_MPU_SERVER0_PORT + (i * 2);
		paramlen = 2;
		ptr[xlen++] = paramid >> 8;
		ptr[xlen++] = paramid;
		ptr[xlen++] = paramlen >> 8;
		ptr[xlen++] = paramlen;

		ptr[xlen++] = sysparam_get_serverport(i) >> 8;
		ptr[xlen++] = sysparam_get_serverport(i);
	}

	for (i = 0; i < 4; i++)
	{
		paramid = PARAM_ID_FROM_MPU_SERVER4_PORT + (i * 2);
		paramlen = 2;
		ptr[xlen++] = paramid >> 8;
		ptr[xlen++] = paramid;
		ptr[xlen++] = paramlen >> 8;
		ptr[xlen++] = paramlen;

		ptr[xlen++] = sysparam_get_serverport(i + 4) >> 8;
		ptr[xlen++] = sysparam_get_serverport(i + 4);
	}
#if 0
	paramid = PARAM_ID_FROM_MPU_GET_MCU_PARAM_SW;
	paramlen = 1;
	ptr[xlen++] = paramid >> 8;
	ptr[xlen++] = paramid;
	ptr[xlen++] = paramlen >> 8;
	ptr[xlen++] = paramlen;
	ptr[xlen++] = get_mcu_report_sw();

	if ((IsObjNULL(get_public_key_ptr(), 64) == 0) && (s_public_key_report_flag == 0))
	{
		paramid = PARAM_ID_FROM_MPU_PUBLIC_KEY;
		paramlen = 64;
		ptr[xlen++] = paramid >> 8;
		ptr[xlen++] = paramid;
		ptr[xlen++] = paramlen >> 8;
		ptr[xlen++] = paramlen;

		memcpy(ptr + xlen, get_public_key_ptr(), 64);
		xlen += 64;
		s_public_key_report_flag = 1;
	}
#endif
	if (1)
	{
/*
		paramid = PARAM_ID_FROM_MPU_CHIP_ID;
		paramlen = 16;
		ptr[xlen++] = paramid >> 8;
		ptr[xlen++] = paramid;
		ptr[xlen++] = paramlen >> 8;
		ptr[xlen++] = paramlen;
*/
		memcpy(ptr + xlen, sysparam_get_jiami_chip_id(), 16);
		xlen += 16;

		paramid = PARAM_ID_FROM_MPU_STATUS1;
		paramlen = 4;
		ptr[xlen++] = paramid >> 8;
		ptr[xlen++] = paramid;
		ptr[xlen++] = paramlen >> 8;
		ptr[xlen++] = paramlen;

		templen = get_mpu_status1();
		ptr[xlen++] = templen >> 24;
		ptr[xlen++] = templen >> 16;
		ptr[xlen++] = templen >> 8;
		ptr[xlen++] = templen;
	}

	usleep(100);
	send_msg_to_mcu(TCCP_MPU_HEAT_MSG_ID, ptr, xlen);
	usleep(100);
}

void send_easy_change_param_to_mcu(void)
{
	unsigned char *buf = 0;
	unsigned int rlen = 0;

	buf = malloc(1024 * 3);
	if (buf)
	{
		rlen = myreadfile("/data/mcu_e_param.dat", buf, 1024 * 3);
		if (rlen)
			send_msg_to_mcu(TCCP_MPU_TO_MCU_E_PARAM_ARRIVE_MSG_ID, buf, rlen);

		free(buf);
	}
}

void send_offline_msg_to_mcu(unsigned char channel)
{
	unsigned char temp[2] = {0};

	temp[0] = channel;
	send_msg_to_mcu(TCCP_MPU_SEND_OFFLINE_MSG_ID, temp, 1);
}

void send_online_msg_to_mcu(unsigned char channel)
{
	unsigned char temp[2] = {0};

	temp[0] = channel;
	send_msg_to_mcu(TCCP_MPU_SEND_ONLINE_MSG_ID, temp, 1);
}

void net_data_arrive(unsigned char channel, void *pdat, unsigned int datlen)
{
	unsigned int rlen = 0;
	unsigned char *ptr = 0;
	unsigned int xlen = 0;
	unsigned char *sendbuf = 0;
	unsigned int templen = 0;

	if ((pdat == 0) || (datlen == 0))
	{
		return;
	}

	if (channel == 5)
	{
		// send the bei an msg body to mcu
		send_msg_to_mcu(TCCP_MPU_TO_MCU_BEIAN_MSGBODY_ARRIVE_MSG_ID, get_beian_msg_body_with_sign_ptr(), 169);
	}

	if ((channel == 1) && (sysparam_get_func_select() == FuncSelect_JMRH_SM4))
	{
		jmrh_analyze_plat_data(pdat, datlen);
	}

	ptr = (unsigned char *)malloc(2048);
	sendbuf = (unsigned char *)malloc(2048);
	if ((ptr == 0) || (sendbuf == 0))
	{
		if (ptr)
			free(ptr);
		if (sendbuf)
			free(sendbuf);
		return;
	}

	ptr[0] = channel;

	templen = datlen;
	if (templen > 2020)
	{
		templen = 2020;
	}
	memcpy(ptr + 1, pdat, templen);
	xlen = 1 + templen;

	rlen = tccp_pack(TCCP_NET_DATA_ARRIVE_MSG_ID, ptr, xlen, sendbuf, 2048);

	// need sem
	uart_send_bytes(0, sendbuf, rlen);

	if (ptr)
		free(ptr);
	if (sendbuf)
		free(sendbuf);
}
