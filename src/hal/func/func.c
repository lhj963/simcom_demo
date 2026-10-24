#include "func.h"

unsigned char is_string(void *ptr, unsigned int len)
{
	unsigned int i = 0;
	unsigned char dat = 0;

	if ((ptr == 0) || (len == 0))
	{
		return 0;
	}

	for (i = 0; i < len; i++)
	{
		dat = *((unsigned char *)ptr + i);
		if (((dat >= 'a') && (dat <= 'z')) || ((dat >= 'A') && (dat <= 'Z')) || ((dat >= '0') && (dat <= '9')))
		{
		}
		else
		{
			return 0;
		}
	}

	return 1;
}

char _is_leap_year(unsigned int year)
{
	unsigned char flag = 0;

	if (((year % 4) == 0) && (year % 100))
	{
		flag = 1;
	}

	if ((year % 400) == 0)
	{
		flag = 1;
	}
	return flag;
}

unsigned char leap_month_day[] =
	{0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

unsigned char no_leap_month_day[] =
	{0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

char _add_day(unsigned int *year,
			  unsigned char *month, unsigned char *day, unsigned char want_to_add_day)
{
	char leap_year_flag = _is_leap_year(*year); // ÈòÄê±êÖŸ
	unsigned char cur_month_day = 0;			// µ±Ç°ÔÂ·ÝµÄÌìÊý
	unsigned long after_add = *day + want_to_add_day;

	if (leap_year_flag)
	{
		cur_month_day = leap_month_day[*month];
	}
	else
	{
		cur_month_day = no_leap_month_day[*month];
	}

	if (after_add > cur_month_day)
	{
		*day = after_add - cur_month_day;
		if ((*month + 1) > 12)
		{
			*month = 1;
			*year = *year + 1;
		}
		else
		{
			*month = *month + 1;
		}
	}
	else
	{
		*day = after_add;
	}

	return 0;
}

void gpsutc_to_btc(unsigned char *year, unsigned char *month, unsigned char *day,
				   unsigned char *hour, unsigned char *min, unsigned char *sec)
{
	unsigned char yy = *year;
	unsigned char mm = *month;
	unsigned char dd = *day;
	unsigned char hh = *hour;
	// unsigned char mmin = *min;
	// unsigned char ss = *sec;
	unsigned int tempyear = 0;
	unsigned char tempmonth = 0;
	unsigned char tempday = 0;

	if ((hh + 8) >= 24) // Ð¡Ê±²úÉúœøÎ»
	{
		*hour = hh + 8 - 24; // ŽŠÀíÐ¡Ê±

		tempyear = yy + 2000;
		tempmonth = mm;
		tempday = dd;

		_add_day(&tempyear, &tempmonth, &tempday, 1);
		*year = tempyear - 2000;
		*month = tempmonth;
		*day = tempday;
	}
	else
	{
		*hour = hh + 8;
	}
}

void mywritefile(char create_way, void *full_path_file_name, void *ptr, unsigned int len)
{
	int ret = 0;
	FILE *fp;
	if ((ptr == 0) || (full_path_file_name == 0) || (strlen((char *)full_path_file_name) == 0))
	{
		return;
	}
	if (create_way == 0)
	{
		fp = fopen((char *)full_path_file_name, "wb+");
	}
	else
	{
		fp = fopen((char *)full_path_file_name, "ab+");
	}
	if (fp)
	{
		if (len)
		{
			ret = fwrite((char *)ptr, len, 1, fp);
		}

		fclose(fp);
	}
}

unsigned int mygetfilesize(void *filename)
{
	FILE *fp;
	int rlen = 0;

	if (filename == 0)
	{
		return 0;
	}

	fp = fopen((char *)filename, "rb");
	if (fp)
	{
		fseek(fp, 0, SEEK_END);
		rlen = ftell(fp);
		fclose(fp);
	}
	if (rlen > 0)
	{
	}
	else
	{
		rlen = 0;
	}
	return rlen;
}

unsigned int __myreadfile(char *open_mode, void *filename, unsigned int offset, void *pbuf, unsigned int buflen)
{
	FILE *fp;
	unsigned int rlen = 0;

	if ((filename == 0) || (pbuf == 0) || (buflen == 0))
	{
		return 0;
	}

	fp = fopen((char *)filename, open_mode);
	if (fp)
	{
		if (fseek(fp, offset, SEEK_SET) == 0)
		{
			rlen = fread(pbuf, 1, buflen, fp);
		}
		fclose(fp);
	}
	return rlen;
}

unsigned int myreadfile1(void *filename, unsigned int offset, void *pbuf, unsigned int buflen)
{
	return __myreadfile("rb+", filename, offset, pbuf, buflen);
}

unsigned int myreadfile(void *filename, void *pbuf, unsigned int buflen)
{
	return myreadfile1(filename, 0, pbuf, buflen);
}

unsigned int myreadfile_readonly(void *filename, void *pbuf, unsigned int buflen)
{
	return __myreadfile("rb", filename, 0, pbuf, buflen);
}

char is_ip_string(void *str)
{
	char *psrc = (char *)str;

	if (str == 0)
	{
		return 0;
	}

	while (1)
	{
		if (*psrc == 0)
		{
			break;
		}

		if (((*psrc >= '0') && (*psrc <= '9')) || (*psrc == '.'))
		{
		}
		else
		{
			return 0;
		}
		psrc++;
	}

	return 1;
}

char is_bcd_time_format_ok(INT8U bcd_time[6])
{
	INT8U time[6] = {0};

	memcpy(time, bcd_time, 6);

	// ���Ƿ�Ϸ�
	if (time[0] > 0x99)
	{
		return 0;
	}
	// ���Ƿ�Ϸ�
	if ((time[1] > 0x12) || (time[1] == 0))
	{
		return 0;
	}
	// ���Ƿ�Ϸ�
	if ((time[2] > 0x31) || (time[2] == 0))
	{
		return 0;
	}
	// ʱ�Ƿ�Ϸ�
	if (time[3] > 0x23)
	{
		return 0;
	}

	// ���Ƿ�Ϸ�
	if (time[4] > 0x59)
	{
		return 0;
	}

	// ���Ƿ�Ϸ�
	if (time[5] > 0x59)
	{
		return 0;
	}

	return 1;
}

char is_time_format_ok(INT8U time[6])
{
	// ���Ƿ�Ϸ�
	if ((time[0] > 60) || (time[0] < 22))
	{
		return 0;
	}
	// ���Ƿ�Ϸ�
	if ((time[1] > 12) || (time[1] == 0))
	{
		return 0;
	}
	// ���Ƿ�Ϸ�
	if ((time[2] > 31) || (time[2] == 0))
	{
		return 0;
	}
	// ʱ�Ƿ�Ϸ�
	if (time[3] > 23)
	{
		return 0;
	}

	// ���Ƿ�Ϸ�
	if (time[4] > 59)
	{
		return 0;
	}

	// ���Ƿ�Ϸ�
	if (time[5] > 59)
	{
		return 0;
	}

	return 1;
}

int timecmp(INT8U time1[6], INT8U time2[6])
{
	char buf1[16] = {0};
	char buf2[16] = {0};
	int ret1 = 0;

	sprintf(buf1, "%02d%02d%02d%02d%02d%02d", time1[0], time1[1], time1[2], time1[3], time1[4], time1[5]);
	sprintf(buf2, "%02d%02d%02d%02d%02d%02d", time2[0], time2[1], time2[2], time2[3], time2[4], time2[5]);

	ret1 = memcmp(buf1, buf2, 12);
	return ret1;
}

// �ж�Ŀ��ʱ��desTime �Ƿ������ʼʱ��ͽ���ʱ��֮�� ������ʼʱ��ͽ���ʱ��
// 201911333333  202001  20200232332233
char is_between_time(INT8U desTime[6], INT8U start_time[6], INT8U end_time[6])
{
	char buf1[16] = {0};
	char buf2[16] = {0};
	char buf3[16] = {0};
	int ret1 = 0;
	int ret2 = 0;

	if ((is_time_format_ok(desTime) == 0) || (is_time_format_ok(start_time) == 0) || (is_time_format_ok(end_time) == 0))
	{
		return 0;
	}

	sprintf(buf1, "%02d%02d%02d%02d%02d%02d", desTime[0], desTime[1], desTime[2], desTime[3], desTime[4], desTime[5]);
	sprintf(buf2, "%02d%02d%02d%02d%02d%02d", start_time[0], start_time[1], start_time[2], start_time[3], start_time[4], start_time[5]);
	sprintf(buf3, "%02d%02d%02d%02d%02d%02d", end_time[0], end_time[1], end_time[2], end_time[3], end_time[4], end_time[5]);

	ret1 = memcmp(buf1, buf2, 12);
	ret2 = memcmp(buf1, buf3, 12);

	if ((ret1 >= 0) && (ret2 <= 0))
	{
		return 1;
	}
	return 0;
}

#include "time.h"

// ��������ʱ��ֱ�ӵļ���ж�����
unsigned int two_time_inv(INT8U start_time[6], INT8U end_time[6])
{
	struct tm time_cha;
	unsigned long t1 = 0;

	struct tm time_cha1;
	unsigned long t2 = 0;

	if ((is_time_format_ok(start_time) == 0) || (is_time_format_ok(end_time) == 0))
	{
		return 0;
	}

	time_cha.tm_year = (start_time[0] + 2000) - 1900; // tm�ṹ��¼��Ϊʵ��-1900
	time_cha.tm_mon = start_time[1] - 1;
	time_cha.tm_mday = start_time[2];
	time_cha.tm_hour = start_time[3];
	time_cha.tm_min = start_time[4];
	time_cha.tm_sec = start_time[5];
	t1 = mktime(&time_cha); // ��ô�1970��1��1��0ʱ0��0��������ȥ��ʱ�䣬��

	time_cha1.tm_year = (end_time[0] + 2000) - 1900; // tm�ṹ��¼��Ϊʵ��-1900
	time_cha1.tm_mon = end_time[1] - 1;
	time_cha1.tm_mday = end_time[2];
	time_cha1.tm_hour = end_time[3];
	time_cha1.tm_min = end_time[4];
	time_cha1.tm_sec = end_time[5];
	t2 = mktime(&time_cha1); // ��ô�1970��1��1��0ʱ0��0��������ȥ��ʱ�䣬��

	return (t2 - t1);
}

// ȫ0 �� ȫ FF ���ж�Ϊ��
int IsObjNULL(void *pdat, INT32U len)
{
	INT32U i = 0;
	INT8U temp = 0;

	for (i = 0; i < len; i++)
	{
		temp = *((INT8U *)pdat + i);

		if ((temp != 0x00) && (temp != 0xFF))
		{
			return 0;
		}
	}
	return 1;
}

unsigned char ToBcd(unsigned char dec)
{
	if (dec >= 100)
	{
		return 0;
	}

	return (unsigned char)(((dec / 10) << 4) | (dec % 10));
}

INT8U bcd_to_dec(INT8U bcd)
{
	INT8U high4 = (bcd >> 4) & 0x0F;
	INT8U low4 = bcd & 0x0F;

	return high4 * 10 + low4;
}

INT8U XOR_Check(INT8U *pdat, INT32U len)
{
	INT32U i = 0;
	INT8U temp = pdat[0];

	if (len == 1)
		return pdat[0];

	for (i = 1; i < len; i++)
	{
		temp ^= pdat[i];
	}
	return temp;
}

/**
�������ܣ���ȡpSubStr1��pSubStr2֮����ַ���
��ڲ���:
		str:Դ�ַ���
		pSubStr1:��ʼ���ַ���
		pSubStr2:��β���ַ���
		outbuflen:����Ļ�������С
���ڲ���:
		pOutBuf:�����ߴ���Ļ����� ���ڽ���pSubStr1��pSubStr2֮�������
����ֵ:��������򷵻�pSubStr1��ָ�� ���򷵻�NULL
����ʾ����GetSubString("<Server@IP:192.168.1.19>","<Server#IP:",">",sub);
		ִ�н����sub = "192.168.1.19"
 */
char *GetSubString(char *str, char *pSubStr1, char *pSubStr2, char *pOutBuf, unsigned int outbuflen)
{
	char *pTemp1 = 0;
	char *pTemp2 = 0;
	unsigned int templen = 0;

	if ((str == 0) || (pSubStr1 == 0) || (pOutBuf == 0) || (outbuflen == 0))
	{
		return 0;
	}

	pTemp1 = strstr(str, pSubStr1);
	// δ�ҵ���һ���ַ���
	if (pTemp1 == 0)
		return 0;

	// ����ڶ������ַ����������0 ���� Ϊ��ֵ ��ô���ؽ����Ĭ��Ϊȡ��һ���ַ�������β֮�������
	if (pSubStr2 && strlen(pSubStr2))
	{
		pTemp2 = strstr(pTemp1 + strlen((char *)pSubStr1), pSubStr2);
	}
	else
	{
		pTemp2 = str + strlen(str);
	}
	if (pTemp2)
	{
		templen = pTemp2 - (pTemp1 + strlen((char *)pSubStr1));
		templen = (outbuflen >= templen) ? templen : outbuflen;
		memset(pOutBuf, 0, outbuflen);
		if (templen)
			memcpy(pOutBuf, pTemp1 + strlen((char *)pSubStr1), templen);

		return pTemp1;
	}
	return 0;
}

/**
��������:��pSrc�������� ���������pDesĿ�괮
�˿ڲ���:pSrc Դ������
		ulSrcLen Դ����������
		pDes Ŀ���ַ���
		ulDesLen Ŀ���ַ�������
���ڲ���:��
����ֵ:���ҳɹ� ����Ŀ�괮ͷָ�� ����ʧ�� ����NULL
˵��:
Դ������ aucSrc[] = "\x55\x32\x33\x00\x34\x33\x00\x34";
Ŀ�껺���� aucDes[] = "\x33\x00\x34";
	ReverseMemstr(aucSrc,8,"\x00\x34",2);
	ִ�н������aucSrc+6
*/
void *ReverseFindSubStr(void *pSrc, int ulSrcLen, void *pDes, int ulDesLen)
{
	INT32S i = 0;
	INT32S j = 0;
	INT8U *pSource = (INT8U *)pSrc;
	INT8U *pTarget = (INT8U *)pDes;

	if ((pSrc == 0) || (ulSrcLen == 0) || (pDes == 0) || (ulDesLen == 0))
	{
		return 0;
	}

	for (i = (ulSrcLen - 1); i >= 0; i--)
	{
		// �ҵ���һ���ֽ�
		if (pSource[i] == *pTarget)
		{
			for (j = 0; j < ulDesLen; j++)
			{
				if (
					((i + j) >= ulSrcLen) || (pSource[i + j] != pTarget[j]))
				{
					break;
				}
			}
			if (j == ulDesLen)
			{
				return (pSource + i);
			}
		}
	}

	return 0;
}

void *_find_sub_str(void *pbuf, unsigned int ulBufLen,
					void *SubStr, unsigned int ulSubStrLen)
{
	unsigned char *pSrc = (unsigned char *)pbuf;
	unsigned char *pSubStr = (unsigned char *)SubStr;
	unsigned int i = 0;
	unsigned int j = 0;

	if ((pbuf == 0) || (ulBufLen == 0) || (SubStr == 0) || (ulSubStrLen == 0))
	{
		return 0;
	}

	for (i = 0; i < ulBufLen; i++)
	{
		if (*(pSrc + i) == *pSubStr) // �ҵ���һ���ֽ�
		{
			for (j = 0; j < ulSubStrLen; j++)
			{
				if (*(pSubStr + j) != *(pSrc + i + j))
				{
					// return 0;
					break;
				}
			}
			if (j == ulSubStrLen)
			{
				return (pSrc + i);
			}
		}
	}
	return 0;
}

/**

  ����ֵ��0 ת���ɹ�
		��0 ת��ʧ��
"89AF3300" ת�����Ϊ 0x89af3300 ������result��
*/
INT32U StrToHex(INT8U *str, INT8U len, INT32U *result)
{
	INT32U dat = 0;
	INT8U temp = 0;
	INT8U i = 0;
	INT8U *pStr = str;

	if (str == 0)
		return 1;
	if ((str[0] == '0') && ((str[1] == 'x') || (str[1] == 'X')))
	{
		pStr = str + 2;
	}

	if (len == 0) // ��������� ��ʾ�������'\0'��β���ַ���
	{
		len = strlen((char *)pStr);
	}
	for (i = 0; i < len; i++)
	{

		dat <<= 4;
		if ((pStr[i] >= '0') && (pStr[i] <= '9'))
		{
			temp = pStr[i] - '0';
		}
		else if ((pStr[i] >= 'a') && (pStr[i] <= 'f'))
		{
			temp = pStr[i] - 'a' + 10;
		}
		else if ((pStr[i] >= 'A') && (pStr[i] <= 'F'))
		{
			temp = pStr[i] - 'A' + 10;
		}
		else // ���Ϸ����ַ�
		{
			return 2;
		}
		dat |= temp;
	}
	if (result)
		*result = dat;
	return 0;
}

int IsHexChar(INT8U ch)
{
	if (((ch >= '0') && (ch <= '9')) || ((ch >= 'a') && (ch <= 'f')) || ((ch >= 'A') && (ch <= 'F')))
	{
		return 1;
	}
	return 0;
}

/**
*str = "0203ed"
����
*str = "02 03 ed";

pdat[0] = 0x02;pdat[1] = 0x03;pdat[2] = 0xed;

����ֵ��3

����ֵС�ڻ������ ��ʾת������
*/
int HexStrToArray(INT8U *str1, INT8U *pdat, INT32U pdatlen)
{
	INT8U aucTemp[3] = {0};
	INT8U ucTempLen = 0;
	INT32U len = 0;
	INT8U *str = str1;
	if (str == 0)
		return -1;

	if (strlen(str1) >= 2)
	{
		if (str1[0] == '0' && ((str1[1] == 'x') || (str1[1] == 'X')))
			str = str1 + 2;
	}

	// left align
	if (strlen(str) % 2)
	{
		aucTemp[0] = '0';
		ucTempLen = 1;
	}

	while (1)
	{
		if (*str == 0)
		{
			break;
		}
		else if (!IsHexChar(*str))
		{
		}
		else
		{
			aucTemp[ucTempLen++] = *str;
			if (ucTempLen == 2)
			{
				INT32U dat = 0;

				if (StrToHex(aucTemp, 0, &dat) == 0)
				{
					pdat[len++] = dat;
					if (pdatlen && (len >= pdatlen))
						break;
				}
				ucTempLen = 0;
			}
		}
		str++;
	}
	return len;
}

/**
����a[] = {0x11,0x22,0x33};
ת��Ϊ"11 22 33" �浽buf��
*/
int ArrayToHexStr(void *pdat, INT32U ulDatLen, void *buf, INT32U ulBufSize)
{
	INT8U *pSrc = (INT8U *)pdat;
	INT8U *pDes = (INT8U *)buf;
	INT32U i = 0;
	INT32U len = 0;

	for (i = 0; i < ulDatLen; i++)
	{
		INT8U ucHigh4 = (*(pSrc + i)) / 16;
		INT8U ucLow4 = (*(pSrc + i)) % 16;

		*(pDes + len++) = (ucHigh4 >= 10) ? (ucHigh4 - 10 + 'A') : (ucHigh4 + '0');
		*(pDes + len++) = (ucLow4 >= 10) ? (ucLow4 - 10 + 'A') : (ucLow4 + '0');

		*(pDes + len++) = ' ';
		if ((len >= ulBufSize) && ulBufSize)
			break;
	}
	return len;
}

/**
����a[] = {0x11,0x22,0x33};
ת��Ϊ"112233" �浽buf��
*/
int ArrayToHexStr1(void *pdat, INT32U ulDatLen, void *buf, INT32U ulBufSize)
{
	INT8U *pSrc = (INT8U *)pdat;
	INT8U *pDes = (INT8U *)buf;
	INT32U i = 0;
	INT32U len = 0;

	for (i = 0; i < ulDatLen; i++)
	{
		INT8U ucHigh4 = (*(pSrc + i)) / 16;
		INT8U ucLow4 = (*(pSrc + i)) % 16;

		*(pDes + len++) = (ucHigh4 >= 10) ? (ucHigh4 - 10 + 'A') : (ucHigh4 + '0');
		*(pDes + len++) = (ucLow4 >= 10) ? (ucLow4 - 10 + 'A') : (ucLow4 + '0');

		//*(pDes + len++) = ' ';
		if ((len >= ulBufSize) && ulBufSize)
			break;
	}
	return len;
}

#define MAX_Q_CNT 8

typedef struct tagQueue
{
	void *ptr; // ���л�����
	INT32U ulWrite;
	INT32U ulRead;

	INT32U ulOneUnitDataSize; // һ�����ݵ�Ԫ�Ĵ�С
	INT32U ulTotalCnt;		  // �ܹ��ж��ٸ����ݵ�Ԫ
} Queue;

static Queue s_astQ[MAX_Q_CNT] = {0};

int FindFreeQueue(void)
{
	INT8U i = 0;

	for (i = 0; i < MAX_Q_CNT; i++)
	{
		if (s_astQ[i].ptr == 0)
		{
			return i;
		}
	}
	return -1;
}

/**
��������:��� ��bufָ����ָ������ ������������
��ڲ���:
	Qnum:���б�� ���ж��б�Ų����ظ�
	ptr:���л�����
	ulOneUnitDataSize:һ�����ݵ�Ԫ�Ĵ�С
	cnt:���ݵ�Ԫ���ܸ���
���ڲ���:
	��
����ֵ:���ض��б�� С��0 �򴴽�����ʧ��
*/
int CreateQueue(int Qnum, void *ptr, INT32U ulOneUnitDataSize, INT32U cnt)
{
	int idx = 0;

	// ������
	if ((ptr == 0) || (ulOneUnitDataSize == 0) || (cnt == 0))
	{
		return -1;
	}

	if (Qnum < 0)
	{
		idx = FindFreeQueue();
		if (idx < 0)
		{
			return -2;
		}
	}
	else
	{
		idx = Qnum;
		if ((idx >= MAX_Q_CNT) || s_astQ[idx].ptr)
		{
			return -2;
		}
	}

	memset(s_astQ + idx, 0, sizeof(s_astQ[idx]));
	s_astQ[idx].ptr = ptr;
	s_astQ[idx].ulOneUnitDataSize = ulOneUnitDataSize;
	s_astQ[idx].ulTotalCnt = cnt;

	return idx;
}

/**
��������:��� ��bufָ����ָ������ ������������
��ڲ���:
	Qnum:���б��
	buf:��ӵĻ�����
���ڲ���:
	��
����ֵ:

*/
int QueueIn(INT8U Qnum, void *buf)
{
	INT32U idx = 0;

	// ������
	if ((buf == 0) || (Qnum >= MAX_Q_CNT))
	{
		return -1;
	}

	if ((s_astQ[Qnum].ptr == 0) || (s_astQ[Qnum].ulOneUnitDataSize == 0) ||
		(s_astQ[Qnum].ulTotalCnt == 0))
	{
		return -2;
	}

	// �ж϶����Ƿ�����
	if (s_astQ[Qnum].ulWrite >= (s_astQ[Qnum].ulRead + s_astQ[Qnum].ulTotalCnt))
	{
		return -3;
	}

	idx = s_astQ[Qnum].ulWrite % s_astQ[Qnum].ulTotalCnt;
	// DBG_Print(LEVEL3,"Queue%d Msg In(idx:%d)\r\n",Qnum,idx);
	memcpy((INT8U *)(s_astQ[Qnum].ptr) + idx * s_astQ[Qnum].ulOneUnitDataSize, buf, s_astQ[Qnum].ulOneUnitDataSize);
	s_astQ[Qnum].ulWrite++;

	return 0;
}

// ��ȡ������ӵ�Ԫ�ظ���
INT32U GetQueueElementCnt(INT8U Qnum)
{
	// ������
	if (Qnum >= MAX_Q_CNT)
	{
		return 0;
	}

	if (s_astQ[Qnum].ulWrite < s_astQ[Qnum].ulRead)
	{
		s_astQ[Qnum].ulWrite = s_astQ[Qnum].ulRead;
		return 0;
	}

	return (s_astQ[Qnum].ulWrite - s_astQ[Qnum].ulRead);
}

// ����
int QueueOut(INT8U Qnum, void *buf)
{
	INT32U idx = 0;

	// ������
	if (Qnum >= MAX_Q_CNT)
	{
		return -1;
	}

	if ((s_astQ[Qnum].ptr == 0) || (s_astQ[Qnum].ulOneUnitDataSize == 0) ||
		(s_astQ[Qnum].ulTotalCnt == 0))
	{
		return -2;
	}

	// �ж϶����Ƿ�Ϊ��
	if (GetQueueElementCnt(Qnum) == 0)
	{
		return -3;
	}

	idx = s_astQ[Qnum].ulRead % s_astQ[Qnum].ulTotalCnt;
	// DBG_Print(LEVEL3,"Queue%d Msg Out(idx:%d)\r\n",Qnum,idx);
	if (buf)
		memcpy(buf, (INT8U *)(s_astQ[Qnum].ptr) + idx * s_astQ[Qnum].ulOneUnitDataSize, s_astQ[Qnum].ulOneUnitDataSize);
	QueueMsgDealOk(Qnum);

	return 0;
}

// ���Ӳ��Զ��Ӷ�ָ��
void *QueueGetCurMsg(INT8U Qnum)
{
	INT32U idx = 0;

	// ������
	if (Qnum >= MAX_Q_CNT)
	{
		return 0;
	}

	if ((s_astQ[Qnum].ptr == 0) || (s_astQ[Qnum].ulOneUnitDataSize == 0) ||
		(s_astQ[Qnum].ulTotalCnt == 0))
	{
		return 0;
	}

	// �ж϶����Ƿ�Ϊ��
	if (GetQueueElementCnt(Qnum) == 0)
	{
		return 0;
	}

	idx = s_astQ[Qnum].ulRead % s_astQ[Qnum].ulTotalCnt;

	return (INT8U *)(s_astQ[Qnum].ptr) + idx * s_astQ[Qnum].ulOneUnitDataSize;
}

// ֪ͨ���� ���ӽ���
int QueueMsgDealOk(INT8U Qnum)
{
	if (s_astQ[Qnum].ulRead < s_astQ[Qnum].ulWrite)
	{
		// DBG_Print(LEVEL3,"Queue%d Msg(idx:%u) Deal Finish.\r\n",Qnum,s_astQ[Qnum].ulRead);
		s_astQ[Qnum].ulRead++;
	}
	return 0;
}
