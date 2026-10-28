#include "box_firmware.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#if 0
�̼���ʽ����:

��ʼ�ַ���־ BYSD_BOX	      	�̶�8Bytes
MCU�ļ���ʼƫ�Ƶ�ַ           	4Bytes
MCU�ļ���ֹƫ�Ƶ�ַ   		4Bytes

MPU�ļ���ʼƫ�Ƶ�ַ   		4Bytes
MPU�ļ���ֹƫ�Ƶ�ַ   		4Bytes

CFG�ļ���ʼƫ�Ƶ�ַ   		4Bytes
CFG�ļ���ֹƫ�Ƶ�ַ   		4Bytes

lib�ļ�����	      		1Byte
lib1�ļ�������			1Byte
lib1�ļ���   			X1Bytes
lib1�ļ���ʼƫ�Ƶ�ַ   		4Bytes
lib1�ļ���ֹƫ�Ƶ�ַ   		4Bytes

lib2�ļ�������			1Byte
lib2�ļ���   			X2Bytes
lib2�ļ���ʼƫ�Ƶ�ַ   		4Bytes
lib2�ļ���ֹƫ�Ƶ�ַ   		4Bytes
...

<����>				nBytes

CRCУ��                		4Bytes

ע��:
����ƫ�Ƶ�ַ��˵��
����: <����> ��1000�ֽ�  �����ļ���С����  MCU�ļ�:0-199  MPU�ļ�:200-899  CFG�ļ�900-949  lib�ļ�:950-999
MCU�ļ�����ʼƫ�Ƶ�ַ����0 ��ֹƫ�ƾ���199
MPU�ļ�����ʼƫ�Ƶ�ַ����200 ��ֹƫ�ƾ���899
cfg�ļ�����ʼƫ�Ƶ�ַ����900 ��ֹƫ�ƾ���949
lib�ļ�����ʼƫ�Ƶ�ַ����950 ��ֹƫ�ƾ���999
#endif

unsigned int box_firmware_pack(void *p_mcu_dat,unsigned int mcu_dat_len,
										void *p_mpu_dat,unsigned int mpu_dat_len,
										void *p_cfg_dat,unsigned int cfg_dat_len,
										unsigned char libcnt,
										void **p_lib_dat,unsigned int *lib_dat_len,
										void **lib_name,
										void *pOut,unsigned int outbuflen)
{
	unsigned int xlen = 0;
	unsigned char *ptr = (unsigned char *)pOut;
	unsigned int mcu_end_offset = 0;
	unsigned int mpu_start_offset = 0;
	unsigned int mpu_end_offset = 0;
	unsigned int cfg_start_offset = 0;
	unsigned int cfg_end_offset = 0;
	unsigned int lib_start_offset = 0;
	unsigned int lib_end_offset = 0;
	unsigned int xoffset = 0;
	unsigned int crc = 0;
	char **pfilename = (char **)lib_name;
	unsigned char i = 0;
	//unsigned char fn_len = 0;
	unsigned char **plibdat = (unsigned char **)p_lib_dat;

	if ((pOut == 0) || (outbuflen == 0))
	{
		return 0;
	}

	if ((p_mcu_dat == 0) && (p_mpu_dat == 0) && (p_cfg_dat == 0) && (libcnt == 0))
	{
		return 0;
	}

	memcpy(ptr,"BYSD_BOX",strlen("BYSD_BOX"));
	xlen += strlen("BYSD_BOX");

	//mcu	
	if ((p_mcu_dat == 0) || (mcu_dat_len == 0))
	{
		mcu_end_offset = 0;
	}
	else
	{
		mcu_end_offset = mcu_dat_len + 4 - 1;
		xoffset += (mcu_dat_len + 4);
	}

	ptr[xlen++] = 0;
	ptr[xlen++] = 0;
	ptr[xlen++] = 0;
	ptr[xlen++] = 0;

	ptr[xlen++] = mcu_end_offset >> 24;
	ptr[xlen++] = mcu_end_offset >> 16;
	ptr[xlen++] = mcu_end_offset >> 8;
	ptr[xlen++] = mcu_end_offset;

	//mpu
	if ((p_mpu_dat == 0) || (mpu_dat_len == 0))
	{
		mpu_start_offset = 0;
		mpu_end_offset = 0;
	}
	else
	{
		mpu_start_offset = xoffset;
		mpu_end_offset = mpu_start_offset + mpu_dat_len - 1;
		xoffset += mpu_dat_len;
	}

	ptr[xlen++] = mpu_start_offset >> 24;
	ptr[xlen++] = mpu_start_offset >> 16;
	ptr[xlen++] = mpu_start_offset >> 8;
	ptr[xlen++] = mpu_start_offset;

	ptr[xlen++] = mpu_end_offset >> 24;
	ptr[xlen++] = mpu_end_offset >> 16;
	ptr[xlen++] = mpu_end_offset >> 8;
	ptr[xlen++] = mpu_end_offset;

	//cfg
	if ((p_cfg_dat == 0) || (cfg_dat_len == 0))
	{
		cfg_start_offset = 0;
		cfg_end_offset = 0;
	}
	else
	{
		cfg_start_offset = xoffset;
		cfg_end_offset = cfg_start_offset + cfg_dat_len - 1;
		xoffset += cfg_dat_len;
	}
	ptr[xlen++] = cfg_start_offset >> 24;
	ptr[xlen++] = cfg_start_offset >> 16;
	ptr[xlen++] = cfg_start_offset >> 8;
	ptr[xlen++] = cfg_start_offset;

	ptr[xlen++] = cfg_end_offset >> 24;
	ptr[xlen++] = cfg_end_offset >> 16;
	ptr[xlen++] = cfg_end_offset >> 8;
	ptr[xlen++] = cfg_end_offset;

	//lib
	if (libcnt && p_lib_dat && lib_dat_len && lib_name)
	{
		ptr[xlen++] = libcnt;
		for (i = 0; i < libcnt; i++)
		{
			//�ļ�������
			ptr[xlen++] = strlen(pfilename[i]);
			//file name
			memcpy(ptr + xlen,pfilename[i],strlen(pfilename[i]));
			xlen += strlen(pfilename[i]);

			lib_start_offset = xoffset;
			lib_end_offset = lib_start_offset + lib_dat_len[i] - 1;
			xoffset += lib_dat_len[i];
		
			ptr[xlen++] = lib_start_offset >> 24;
			ptr[xlen++] = lib_start_offset >> 16;
			ptr[xlen++] = lib_start_offset >> 8;
			ptr[xlen++] = lib_start_offset;
		
			ptr[xlen++] = lib_end_offset >> 24;
			ptr[xlen++] = lib_end_offset >> 16;
			ptr[xlen++] = lib_end_offset >> 8;
			ptr[xlen++] = lib_end_offset;
		}
	}
	else
	{
		ptr[xlen++] = 0;
	}

	//data
	if (p_mcu_dat && mcu_dat_len)
	{
		memcpy(ptr + xlen,p_mcu_dat,mcu_dat_len);
		xlen += mcu_dat_len;

		crc = calc_crc32(0,p_mcu_dat,mcu_dat_len);
		ptr[xlen++] = crc >> 24;
		ptr[xlen++] = crc >> 16;
		ptr[xlen++] = crc >> 8;
		ptr[xlen++] = crc;
	}

	if (p_mpu_dat && mpu_dat_len)
	{
		memcpy(ptr + xlen,p_mpu_dat,mpu_dat_len);
		xlen += mpu_dat_len;
	}

	if (p_cfg_dat && cfg_dat_len)
	{
		memcpy(ptr + xlen,p_cfg_dat,cfg_dat_len);
		xlen += cfg_dat_len;
	}

	for (i = 0; i < libcnt; i++)
	{
		memcpy(ptr + xlen,plibdat[i],lib_dat_len[i]);
		xlen += lib_dat_len[i];
	}

	crc = calc_crc32(0,ptr,xlen);
	ptr[xlen++] = crc >> 24;
	ptr[xlen++] = crc >> 16;
	ptr[xlen++] = crc >> 8;
	ptr[xlen++] = crc;

	return xlen;
}


int box_firmware_unpack(void *pdat,unsigned int datlen,
								void (*mcu_dat_callback)(void *pdata,unsigned int datalen),
								void (*mpu_dat_callback)(void *pdata,unsigned int datalen),
								void (*cfg_dat_callback)(void *pdata,unsigned int datalen),
								void (*lib_dat_callback)(void *filename,void *pdata,unsigned int datalen)
								)
{
	unsigned char *ptr = (unsigned char *)pdat;
	unsigned char *pfiledata = 0;
	unsigned char *p_lib_info = 0;
	unsigned int fact_crc = 0;
	unsigned int calc_crc = 0;
	unsigned int mcu_end_offset = 0;
	unsigned int mpu_start_offset = 0;
	unsigned int mpu_end_offset = 0;
	unsigned int cfg_start_offset = 0;
	unsigned int cfg_end_offset = 0;
	unsigned int lib_start_offset = 0;
	unsigned int lib_end_offset = 0;
	unsigned int xlen = 0;
	unsigned int libidx = 0;
	unsigned char i = 0;
	unsigned char libcnt = 0;
	unsigned char fn_len = 0;
	char fn[64 + 1] = {0};

	if ((pdat == 0) || (datlen <= 4))
	{
		return 1;
	}

	//check head
	if ((ptr[0] != 'B') || (ptr[1] != 'Y') || (ptr[2] != 'S') || (ptr[3] != 'D')
		|| (ptr[4] != '_') || (ptr[5] != 'B') || (ptr[6] != 'O') || (ptr[7] != 'X'))
	{
		return 2;
	}

	//check crc32
	fact_crc = ptr[datlen - 1] + (ptr[datlen - 2] << 8) + (ptr[datlen - 3] << 16) + (ptr[datlen - 4] << 24);
	calc_crc = calc_crc32(0,pdat,datlen - 4);
	if (calc_crc != fact_crc)
	{
		return 3;
	}

	xlen = 8;			//head 8Bytes

	if ((xlen + 4 + 4 + 4 + 4 + 4 + 4 + 1) > datlen)
	{
		return 6;
	}

	xlen += 4;

	mcu_end_offset += (ptr[xlen++] << 24);
	mcu_end_offset += (ptr[xlen++] << 16);
	mcu_end_offset += (ptr[xlen++] << 8);
	mcu_end_offset += ptr[xlen++];

	mpu_start_offset += (ptr[xlen++] << 24);
	mpu_start_offset += (ptr[xlen++] << 16);
	mpu_start_offset += (ptr[xlen++] << 8);
	mpu_start_offset += ptr[xlen++];

	mpu_end_offset += (ptr[xlen++] << 24);
	mpu_end_offset += (ptr[xlen++] << 16);
	mpu_end_offset += (ptr[xlen++] << 8);
	mpu_end_offset += ptr[xlen++];

	cfg_start_offset += (ptr[xlen++] << 24);
	cfg_start_offset += (ptr[xlen++] << 16);
	cfg_start_offset += (ptr[xlen++] << 8);
	cfg_start_offset += ptr[xlen++];

	cfg_end_offset += (ptr[xlen++] << 24);
	cfg_end_offset += (ptr[xlen++] << 16);
	cfg_end_offset += (ptr[xlen++] << 8);
	cfg_end_offset += ptr[xlen++];

	if (xlen >= datlen)
	{
		return 6;
	}
	libcnt = ptr[xlen++];
	p_lib_info = ptr + xlen;
	for (i = 0; i < libcnt; i++)
	{
		if (xlen >= datlen)
		{
			return 6;
		}
		fn_len = ptr[xlen++];
		if ((fn_len == 0) || (fn_len > 64) || ((xlen + fn_len + 8) > datlen))
		{
			return 4;
		}
		xlen += fn_len;

		//lib star offset
		xlen += 4;
		//lib end offset
		xlen += 4;
	}

	pfiledata = ptr + xlen;
	if ((mcu_end_offset > (datlen - xlen)) ||
		(mpu_start_offset > mpu_end_offset) || (mpu_end_offset > (datlen - xlen)) ||
		(cfg_start_offset > cfg_end_offset) || (cfg_end_offset > (datlen - xlen)))
	{
		return 5;
	}

	if (mcu_end_offset)
	{
		if (mcu_dat_callback)
		{
			mcu_dat_callback(pfiledata,mcu_end_offset + 1);
		}
	}

	if (mpu_start_offset || mpu_end_offset)
	{
		if (mpu_dat_callback)
		{
			mpu_dat_callback(pfiledata + mpu_start_offset,mpu_end_offset - mpu_start_offset + 1);
		}
	}

	if (cfg_start_offset || cfg_end_offset)
	{
		if (cfg_dat_callback)
		{
			cfg_dat_callback(pfiledata + cfg_start_offset,cfg_end_offset - cfg_start_offset + 1);
		}
	}

	for (i = 0; i < libcnt; i++)
	{
		lib_start_offset = 0;
		lib_end_offset = 0;

		if ((pfiledata < p_lib_info) || ((unsigned int)(pfiledata - p_lib_info) < (libidx + 1 + 8)))
		{
			return 6;
		}
		fn_len = p_lib_info[libidx++];
		if ((fn_len == 0) || (fn_len > 64) ||
			((unsigned int)(pfiledata - p_lib_info) < (libidx + fn_len + 8)))
		{
			return 4;
		}
		memset(fn,0,sizeof(fn));
		memcpy(fn,p_lib_info + libidx,fn_len);
		libidx += fn_len;

		//lib star offset
		lib_start_offset += (p_lib_info[libidx++] << 24);
		lib_start_offset += (p_lib_info[libidx++] << 16);
		lib_start_offset += (p_lib_info[libidx++] << 8);
		lib_start_offset += (p_lib_info[libidx++]);

		//lib end offset
		lib_end_offset += (p_lib_info[libidx++] << 24);
		lib_end_offset += (p_lib_info[libidx++] << 16);
		lib_end_offset += (p_lib_info[libidx++] << 8);
		lib_end_offset += (p_lib_info[libidx++]);

		if ((lib_start_offset > lib_end_offset) || (lib_end_offset > (datlen - xlen))) return 5;
		if (lib_dat_callback)
		{
			lib_dat_callback(fn,pfiledata + lib_start_offset,lib_end_offset - lib_start_offset + 1);
		}
	}

	return 0;
}



static int box_lib_name_ok(const char *name)
{
	size_t i;
	size_t n;

	if (name == 0)
	{
		return 0;
	}
	n = strlen(name);
	if ((n == 0) || (n > 64) || (name[0] == '.') || (strstr(name, "..") != 0))
	{
		return 0;
	}
	for (i = 0; i < n; i++)
	{
		unsigned char c = (unsigned char)name[i];

		if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') ||
			  c == '.' || c == '_' || c == '-' || c == '+'))
		{
			return 0;
		}
	}
	return 1;
}

int box_install_library(const char *filename, const void *data, unsigned int len)
{
	char path[96];
	struct stat st;

	if ((box_lib_name_ok(filename) == 0) || (data == 0) || (len == 0))
	{
		return -1;
	}
	if (snprintf(path, sizeof(path), "/usr/lib/%s", filename) >= (int)sizeof(path))
	{
		return -1;
	}
	if (lstat(path, &st) == 0)
	{
		if (S_ISDIR(st.st_mode))
		{
			return -1;
		}
		/* unlink does not follow a symlink, so this cannot delete outside /usr/lib. */
		if (unlink(path) != 0)
		{
			return -1;
		}
	}
	return durable_write_file(path, data, len);
}

void test_pack_firmware(void)
{
	#if 0
	unsigned int rlen = 0;
	unsigned char pbuf[256] = {0};
	char *libname[] = {"libsm9.so","libutil.so","libpoc.so"};
	unsigned int lib_dat_len[] = {16,16,16};
	unsigned char *lib_dat[] = {"4444444444444444","5555555555555555","6666666666666666"};

	rlen = box_firmware_pack("1111111111111111",strlen("1111111111111111"),
					"2222222222222222",strlen("2222222222222222"),
					"3333333333333333",strlen("3333333333333333"),
					3,lib_dat,lib_dat_len,libname,pbuf,sizeof(pbuf)
					);
	//MyCreateFile(0,"box.bin",pbuf,rlen);
	#endif
}






