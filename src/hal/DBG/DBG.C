#include "DBG.H"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "stdarg.h"

static char buf[1024] = {0};
static int s_ulLevel = LEVEL0;

typedef struct tagDBGUsart
{
	DBGUsartInit pDBGUsartInitFunc;		//���ڳ�ʼ������ָ��
	DBGUsartSend_unlock pDBGUsartSend;	//���ڷ��ͺ���ָ��
	DBGUsartRead pDBGUsartRead;

	unsigned long bps;									//������
	char cParity;
}DBGUsart;

void (*p_dbx_create_sem_func)(void);
void (*p_dbx_wait_sem_func)(void);
void (*p_dbx_post_sem_func)(void);

DBGUsart s_astDBGUsart[MAX_DBG_CNT] = {0};
unsigned long s_ulDBGUsartUsedFlag = 0;

static int create_sem(void)
{

	return 0;
}

static void WaitDbgTxSemNew(void)
{
	if (p_dbx_wait_sem_func)
	{
		p_dbx_wait_sem_func();
	}
}

static void PostDbgTxSemNew(void)
{
	if (p_dbx_post_sem_func)
	{
		p_dbx_post_sem_func();
	}
}

int DBG_Init(unsigned char num,
				DBGUsartInit pDBGUsartInitFunc,
				unsigned long bps,char cParity,
				void (*create_sem_func)(void),
				void (*wait_sem_func)(void),
				void (*post_sem_func)(void),
				DBGUsartSend_unlock pDBGUsartSend_unlock,
				DBGUsartRead pDBGUsartRead
				)
{
	if (num >= MAX_DBG_CNT)
	{
		return 1;
	}
	s_astDBGUsart[num].pDBGUsartInitFunc = pDBGUsartInitFunc;
	s_astDBGUsart[num].pDBGUsartSend = pDBGUsartSend_unlock;
	s_astDBGUsart[num].pDBGUsartRead = pDBGUsartRead;
	s_astDBGUsart[num].bps = bps;
	s_astDBGUsart[num].cParity = cParity;
	if (s_astDBGUsart[num].pDBGUsartInitFunc)
	{
		s_astDBGUsart[num].pDBGUsartInitFunc(bps,cParity);
		//s_astDBGUsart[num].pDBGUsartSend("hello",5);
	}

	if (num == 0)
	{
		p_dbx_create_sem_func = create_sem_func;
		p_dbx_wait_sem_func = wait_sem_func;
		p_dbx_post_sem_func = post_sem_func;
		
		if (p_dbx_create_sem_func)
		{
			p_dbx_create_sem_func();
		}
	}

	/* д�봮�ڱ�ռ�ñ�־ */
	s_ulDBGUsartUsedFlag |= (1 << num);
	return create_sem();
}

static void _DBG_SenDataNewEx(void *pext,void *buf,int len)
{
	unsigned char i = 0;

	for (i = 0; i < MAX_DBG_CNT; i++)
	{
		if (s_ulDBGUsartUsedFlag & (1 << i))
		{
			if (s_astDBGUsart[i].pDBGUsartSend)
			{
				s_astDBGUsart[i].pDBGUsartSend(pext,buf,len);
			}
		}
	}
}

static void _DBG_SenDataNew(void *buf,int len)
{
	unsigned char i = 0;

	for (i = 0; i < MAX_DBG_CNT; i++)
	{
		if (s_ulDBGUsartUsedFlag & (1 << i))
		{
			if (s_astDBGUsart[i].pDBGUsartSend)
			{
				s_astDBGUsart[i].pDBGUsartSend(0,buf,len);
			}
		}
	}
}

unsigned int ReadDBGUsart(unsigned char DBG_PortNum,void *buf,
								unsigned int ulBufSize,unsigned int ulTimeoutMs)
{
	if (s_astDBGUsart[DBG_PortNum].pDBGUsartRead)
	{
		return s_astDBGUsart[DBG_PortNum].pDBGUsartRead(buf,ulBufSize,ulTimeoutMs);
	}
	return 0;
}

//��ӡ�ȼ����
int IsLevelCheckOk(int iLevel)
{
	if (iLevel == LEVEL0)
	{
		return 1;
	}

	if ((s_ulLevel == LEVEL_GPRS) && (iLevel == LEVEL_GPRS))
	{
		return 1;
	}
	else if ((s_ulLevel == LEVEL_GPS) && (iLevel == LEVEL_GPS))
	{
		return 1;
	}
	else if ((s_ulLevel == LEVEL_CAN) && (iLevel == LEVEL_CAN))
	{
		return 1;
	}
	else
	{
		if (s_ulLevel > LEVEL9)
		{
			return 0;
		}
		if ((iLevel > s_ulLevel))
		{
			return 0;
		}

	}

	return 1;
}


void Set_DBG_Level(int iLevel)
{
	s_ulLevel = iLevel;
}

int Get_DBG_Level(void)
{
	return s_ulLevel;
}

int DBG_PrintEx(int iLevel,void *pext,void *str,...)
{
	unsigned int len = 0;
	va_list ap;

	if (str == 0) return 0;

	if (IsLevelCheckOk(iLevel) == 0)
	{
		return 0;
	}

	WaitDbgTxSemNew();
	va_start(ap,str);
	len = vsnprintf(buf,sizeof(buf) - 1,(char *)str,ap);
	va_end(ap);
	if (len >= (int)sizeof(buf)) len = sizeof(buf) - 1;
	_DBG_SenDataNewEx(pext,buf,len);
	PostDbgTxSemNew();
	return len;
}

int DBG_Print(int iLevel,void *str,...)
{
	unsigned int len = 0;
	va_list ap;

	if (str == 0) return 0;

	//����ӡ�ȼ�
	if (IsLevelCheckOk(iLevel) == 0)
	{
		return 0;
	}

	WaitDbgTxSemNew();
	va_start(ap,str);
	len = vsnprintf(buf,sizeof(buf) - 1,(char *)str,ap);
	va_end(ap);
	if (len >= (int)sizeof(buf)) len = sizeof(buf) - 1;
	_DBG_SenDataNew(buf,len);
	PostDbgTxSemNew();
	return len;
}

void DBG_PrintBufEx(int iLevel,void *pext,void *tip,void *str,unsigned int length)
{
	unsigned int len = 0;
	unsigned int i = 0;
	unsigned char *ptr = (unsigned char *)str;

	if (IsLevelCheckOk(iLevel) == 0)
	{
		return ;
	}

	WaitDbgTxSemNew();

	//tip
	if (tip && strlen((char *)tip)) _DBG_SenDataNewEx(pext,tip,strlen((char *)tip));

	if (str && length)
	{
		for (i = 0; i < length; i++)
		{
			if (len >= (sizeof(buf) - 5))
			{
				_DBG_SenDataNewEx(pext,buf,len);
				len = 0;
			}
			len += snprintf(buf + len,sizeof(buf) - len,"%02X",*(ptr + i));
		}
		if (len) _DBG_SenDataNewEx(pext,buf,len);
	}

	_DBG_SenDataNewEx(pext,"\r\n",2);
	PostDbgTxSemNew();
}

void DBG_PrintBuf(int iLevel,void *tip,void *str,unsigned int length)
{
	unsigned int len = 0;
	unsigned int i = 0;
	unsigned char *ptr = (unsigned char *)str;

	//if ((str == 0) || (length == 0)) return ;

	//����ӡ�ȼ�
	if (IsLevelCheckOk(iLevel) == 0)
	{
		return ;
	}

	WaitDbgTxSemNew();

	//tip
	if (tip && strlen((char *)tip)) _DBG_SenDataNew(tip,strlen((char *)tip));

	if (str && length)
	{
		for (i = 0; i < length; i++)
		{
			if (len >= (sizeof(buf) - 5))
			{
				_DBG_SenDataNew(buf,len);
				len = 0;
			}
		
			if ((i % 16 == 0) && i)
			{
				len += snprintf(buf + len,sizeof(buf) - len,"\r\n");
			}
			len += snprintf(buf + len,sizeof(buf) - len,"%02X ",*(ptr + i));
		}
		if (len) _DBG_SenDataNew(buf,len);
	}

	_DBG_SenDataNew("\r\n",2);
	PostDbgTxSemNew();
}

unsigned char DBG_SendChar(unsigned char ch)
{
	unsigned char temp[2] = {0};

	temp[0] = ch;
	WaitDbgTxSemNew();
	_DBG_SenDataNew(temp,1);
	PostDbgTxSemNew();

	return ch;
}

void DBG_SendData(void *pdat,unsigned int len)
{
	WaitDbgTxSemNew();
	_DBG_SenDataNew(pdat,len);
	PostDbgTxSemNew();
}

void DBG_SendDataEx(void *pext,void *pdat,unsigned int len)
{
	WaitDbgTxSemNew();
	_DBG_SenDataNewEx(pext,pdat,len);
	PostDbgTxSemNew();
}

void DBG_ChangeParam(unsigned char num,unsigned long bps,char cParity)
{
	char cFlag = 0;

	if (s_astDBGUsart[num].bps != bps)
	{
		s_astDBGUsart[num].bps = bps;
		cFlag = 1;
	}
	
	if (s_astDBGUsart[num].cParity != cParity)
	{
		s_astDBGUsart[num].cParity = cParity;
		cFlag = 1;
	}

	if (cFlag)
	{
		if (s_astDBGUsart[num].pDBGUsartInitFunc)
		{
			s_astDBGUsart[num].pDBGUsartInitFunc(bps,cParity);
		}
	}
}




