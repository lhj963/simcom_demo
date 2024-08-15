#ifndef __DBG_H__
#define __DBG_H__

#ifdef __cplusplus
extern "C"
{
#endif

#define LEVEL0 0
#define LEVEL1 1
#define LEVEL2 2
#define LEVEL3 3
#define LEVEL4 4 // Ƶ�ʴ��Ϊ2�� 3�����Ϣ 4��
#define LEVEL5 5 // 1��һ������Ϣ 5����5������
#define LEVEL6 6
#define LEVEL7 7
#define LEVEL8 8
#define LEVEL9 9

#define LEVEL_GPS (LEVEL9 + 1)	// ֻ��ӡGPSģ���������Ϣ
#define LEVEL_GPRS (LEVEL9 + 2) // ֻ��ӡGPRSģ���������Ϣ
#define LEVEL_CAN (LEVEL9 + 3)	// ֻ��ӡCANģ���������Ϣ

#define MAX_DBG_CNT 4

	typedef void (*DBGUsartInit)(unsigned long bps, char cParity);
	typedef unsigned int (*DBGUsartSend_unlock)(void *pext, void *buf, unsigned int len);
	typedef unsigned int (*DBGUsartRead)(void *buf, unsigned int ulBufSize, unsigned int ulTimeoutMs);

	/**
	��������:���Դ��ڳ�ʼ��
	��ڲ���:
		num:��ʶ�ڼ�·����
		pDBGUsartInitFunc:���ڳ�ʼ������ָ��
		bps:������
		cParity:0��У�� 1��У�� 2żУ��
		pDBGUsartSend_unlock:���ڷ��ͺ���ָ��
		pDBGUsartRead:�����ں���ָ��
	���ڲ���:
		��
	����ֵ:��
	*/
	extern int DBG_Init(unsigned char num,
						DBGUsartInit pDBGUsartInitFunc,
						unsigned long bps, char cParity,
						void (*create_sem_func)(void),
						void (*wait_sem_func)(void),
						void (*post_sem_func)(void),
						DBGUsartSend_unlock pDBGUsartSend_unlock,
						DBGUsartRead pDBGUsartRead);

	extern unsigned int ReadDBGUsart(unsigned char DBG_PortNum, void *buf,
									 unsigned int ulBufSize, unsigned int ulTimeoutMs);

	extern void DBG_ChangeParam(unsigned char num, unsigned long bps, char cParity);

	extern void Set_DBG_Level(int iLevel);

	extern int Get_DBG_Level(void);

	extern int DBG_Print(int iLevel, void *str, ...);
	extern int DBG_PrintEx(int iLevel, void *pext, void *str, ...);

	extern void DBG_PrintBuf(int iLevel, void *tip, void *str, unsigned int length);
	extern void DBG_PrintBufEx(int iLevel, void *pext, void *tip, void *str, unsigned int length);

	extern unsigned char DBG_SendChar(unsigned char ch);

	extern void DBG_SendData(void *pdat, unsigned int len);
	extern void DBG_SendDataEx(void *pext, void *pdat, unsigned int len);

#ifdef __cplusplus
}
#endif

#endif
