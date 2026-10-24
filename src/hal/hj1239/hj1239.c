#include "hj1239.h"

#define MIN_DATA_LEN (2 + 1 + 17 + 1 + 1 + 2 + 1)


typedef struct tagHJ1239Proc
{
	unsigned int (*sign_proc)(void *pdat,unsigned int datlen,void *pOut,unsigned int outbuflen);
	unsigned char beian_msg_body_with_sign[169];
}HJ1239Proc;

HJ1239Proc stHJ1239Proc = {0};


void hj1239_proc_init(unsigned int (*sign_proc)(void *pdat,unsigned int datlen,void *pOut,unsigned int outbuflen))
{
	stHJ1239Proc.sign_proc = sign_proc;
}

unsigned int hj1239_data_pack(void *pHJ1239FormatData,unsigned int datlen)
{
	unsigned char *pSignStart = 0;
	unsigned char *pPlaceSignStart = 0;
	unsigned int raw_data_len = 0;
	unsigned char buf[64 + 16] = {0};
	unsigned int xlen = 0;
	unsigned char cmd = 0;

	if ((pHJ1239FormatData == 0) || (datlen < MIN_DATA_LEN))
	{
		return 0;
	}

	//only cmd 02 03 07 25 add sm2 sign
	cmd = *((unsigned char *)pHJ1239FormatData + 2);
	if ((cmd != 0x02) && (cmd != 0x03) && (cmd != 0x07) && (cmd != 0x25))
	{
		return datlen;
	}

	pSignStart = (unsigned char *)pHJ1239FormatData + (2+1+17+1+1+2);
	pPlaceSignStart = (unsigned char *)pHJ1239FormatData + datlen - 1;
	raw_data_len = (*((unsigned char *)pHJ1239FormatData + (2+1+17+1+1)) << 8) + *((unsigned char *)pHJ1239FormatData + (2+1+17+1+1+1));

	//after sign the content len
	*((unsigned char *)pHJ1239FormatData + (2+1+17+1+1)) = (raw_data_len + 66) >> 8;
	*((unsigned char *)pHJ1239FormatData + (2+1+17+1+1+1)) = (raw_data_len + 66);

	//*((unsigned char *)pHJ1239FormatData + 21) = 0x03;			//SM2

	if (stHJ1239Proc.sign_proc)
	{
		//save_log("hj_raw:",pSignStart,raw_data_len);
		stHJ1239Proc.sign_proc(pSignStart,raw_data_len,buf,sizeof(buf));

		pPlaceSignStart[xlen++] = 32;
		memcpy(pPlaceSignStart + xlen,buf,32);
		xlen += 32;
		pPlaceSignStart[xlen++] = 32;
		memcpy(pPlaceSignStart + xlen,buf + 32,32);
		xlen += 32;

		if (cmd == 0x07)			//bei an msg
		{
			memcpy(stHJ1239Proc.beian_msg_body_with_sign,pSignStart,169);
		}
	}

	*((unsigned char *)pHJ1239FormatData + datlen + xlen - 1) = XOR_Check((unsigned char *)pHJ1239FormatData + 2,datlen + xlen - 3);
	return (datlen + xlen);
}

void *get_beian_msg_body_with_sign_ptr(void)
{
	return stHJ1239Proc.beian_msg_body_with_sign;
}


