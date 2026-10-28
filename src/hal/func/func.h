#ifndef __FUNC_H__
#define __FUNC_H__
#ifdef __cplusplus
extern "C" {
#endif

#include "stdio.h"
#include "stdlib.h"
#include "string.h"


typedef unsigned int INT32U;
typedef unsigned short INT16U;
typedef unsigned char INT8U;

typedef int INT32S;
typedef short INT16S;
typedef char INT8S;

extern unsigned char is_string(void *ptr,unsigned int len);

extern void gpsutc_to_btc(unsigned char *year,unsigned char *month,unsigned char *day,
						unsigned char *hour,unsigned char *min,unsigned char *sec);

extern void mywritefile(char create_way,void *full_path_file_name,void *ptr,unsigned int len);

/* Run argv[0] with posix_spawnp (no shell). Returns the exit status, or -1. */
extern int exec_argv_status(char *const argv[]);

/*
 * MCU remote command: only `wget -O <fixed path> <url>` is executed, via argv.
 * Returns 0 after wget exits 0. out_path is a static allowlisted path.
 * url_out receives the URL. Any other payload is rejected and not executed.
 */
extern int mcu_exec_allowlisted_cmd(const void *payload, unsigned int len,
									const char **out_path, char *url_out, unsigned int url_out_len);

/* FTP GET via wget argv. Credentials are separate arguments, never a shell string. */
extern int safe_ftp_get(const char *url, const char *user, const char *pass, const char *outfile);

extern unsigned int myreadfile1(void *filename,unsigned int offset,void *pbuf,unsigned int buflen);

extern unsigned int myreadfile(void *filename,void *pbuf,unsigned int buflen);
extern unsigned int myreadfile_readonly(void *filename,void *pbuf,unsigned int buflen);

extern unsigned int mygetfilesize(void *filename);

#define ARRAY_ELEMENT_CNT(ary) (sizeof(ary) / sizeof(ary[0]))

extern char is_ip_string(void *str);

extern int timecmp(INT8U time1[6],INT8U time2[6]);


extern unsigned int two_time_inv(INT8U start_time[6],INT8U end_time[6]);


extern char is_time_format_ok(INT8U time[6]);
extern char is_bcd_time_format_ok(INT8U bcd_time[6]);

extern char is_between_time(INT8U desTime[6],INT8U start_time[6],INT8U end_time[6]);

extern int IsObjNULL(void *pdat,INT32U len);

extern unsigned char ToBcd(unsigned char dec);
extern INT8U bcd_to_dec(INT8U bcd);

extern INT8U XOR_Check(INT8U *pdat,INT32U len);

extern char *GetSubString(char *str,char *pSubStr1,char *pSubStr2,char *pOutBuf,unsigned int outbuflen);
extern void *ReverseFindSubStr(void *pSrc,int ulSrcLen,void *pDes,int ulDesLen);
extern void *_find_sub_str(void *pbuf,unsigned int ulBufLen,void *SubStr,unsigned int ulSubStrLen);
							 

extern int HexStrToArray(INT8U *str1,INT8U *pdat,INT32U pdatlen);

/**
����a[] = {0x11,0x22,0x33};
ת��Ϊ"11 22 33" �浽buf��
*/
extern int ArrayToHexStr(void *pdat,INT32U ulDatLen,void *buf,INT32U ulBufSize);

/**
����a[] = {0x11,0x22,0x33};
ת��Ϊ"112233" �浽buf��
*/
extern int ArrayToHexStr1(void *pdat,INT32U ulDatLen,void *buf,INT32U ulBufSize);



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
extern int CreateQueue(int Qnum,void *ptr,INT32U ulOneUnitDataSize,INT32U cnt);

/**
��������:��� ��bufָ����ָ������ ������������
��ڲ���:
	Qnum:���б��
	buf:��ӵĻ�����
���ڲ���:
	��
����ֵ:

*/
extern int QueueIn(INT8U Qnum,void *buf);

//��ȡ������ӵ�Ԫ�ظ���
extern INT32U GetQueueElementCnt(INT8U Qnum);

//����
extern int QueueOut(INT8U Qnum,void *buf);

extern void *QueueGetCurMsg(INT8U Qnum);

extern int QueueMsgDealOk(INT8U Qnum);





#ifdef __cplusplus
}
#endif

#endif


