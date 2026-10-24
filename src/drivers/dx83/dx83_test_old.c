#include "dx83_test_old.h"
#include "dlfcn.h"
#include "DX83_API.h"
//#include "sm9.h"
DX83_CHIP dx83;

typedef unsigned char (*DX83_ImportKey_func)(DX83_CHIP *dx83, unsigned char kid, unsigned char *key, unsigned char len);
typedef unsigned char (*init_func)(void);
typedef int (*handshake_func)(void);
typedef unsigned char (*SM4_EncDec_func)(unsigned char *psrc, int srclen, unsigned char *pOut);


typedef unsigned char (*dx83_func)(DX83_CHIP *dx83);

typedef unsigned char (*DX83_GetRandom_func)(DX83_CHIP *dx83, unsigned char *random, unsigned char len);

typedef unsigned char (*DX83_VerifyKey_func)(DX83_CHIP *dx83, unsigned char index, unsigned char *seed, unsigned char *pin_zkey);


typedef unsigned char (*DX83_CipherInit_func)(DX83_CHIP *dx83, unsigned char alg2, unsigned char kid_sym, unsigned char *iv);
typedef unsigned char (*DX83_CipherEncrypt_func)(DX83_CHIP *dx83, unsigned char *plaint, unsigned char len, unsigned char *encrypt);
typedef unsigned char (*DX83_CipherDecrypt_func)(DX83_CHIP *dx83, unsigned char *encrypt, unsigned char len, unsigned char *plaint);


void dx83_test(void)
{
	unsigned int i = 0;
	void *dx83_handle = 0;
	DX83_ImportKey_func pDX83_ImportKey_func = 0;
	init_func pinit_sm9 = 0;
	SM4_EncDec_func pjiami_func = 0;
	SM4_EncDec_func pjiemi_func = 0;
	handshake_func phandshake_func = 0;

	dx83_func dx83_software_reset_func = 0;
	dx83_func dx83_get_chip_info_func = 0;
	DX83_GetRandom_func p_dx83_get_random_func = 0;
	DX83_VerifyKey_func p_DX83_VerifyKey_func = 0;

	DX83_CipherInit_func p_DX83_CipherInit_func = 0;
	DX83_CipherEncrypt_func p_DX83_CipherEncrypt_func = 0;
	DX83_CipherDecrypt_func p_DX83_CipherDecrypt_func = 0;

	unsigned char seed[32] = {0};
	unsigned char tmpkey[64] = {0};

    unsigned char rv = 0;

	unsigned char sm4EncKey[] = {
		0x01,0x23,0x45,0x67,0x89,0xAB,0xCD,0xEF,0xFE,0xDC,0xBA,0x98,0x76,0x54,0x32,0x10,
		0x01,0x23,0x45,0x67,0x89,0xAB,0xCD,0xEF,0xFE,0xDC,0xBA,0x98,0x76,0x54,0x32,0x10
	};
    unsigned char pdat[256] = {0};
    unsigned char jiami_dat[256] = {0};
    unsigned char jiemi_dat[256] = {0};
	unsigned char tmpiv[16] = {0};
	unsigned char after_jiami_len = 240;
	int ret = 0;

	memset(tmpiv,0x11,16);
	memset(pdat,0x39,sizeof(pdat));

	dx83_handle = dlopen("/usr/lib/libsm9.so",RTLD_NOW);
	printf("dx83_handle:%08X\r\n",dx83_handle);

	if (dx83_handle)
	{
		//pinit_sm9 = dlsym(dx83_handle,"SM9_init");
		if (pinit_sm9)
		{
			//pinit_sm9();
		}
		//printf("SM9_init finish\r\n");

		//phandshake_func = dlsym(dx83_handle,"handshake");
		if (phandshake_func)
		{
			//ret = phandshake_func();
			//printf("handshake result:%d\r\n",ret);
		}

		dx83_software_reset_func = dlsym(dx83_handle,"DX83_SoftReset");
		if (dx83_software_reset_func)
		{
			dx83.sla = 0x30;
			dx83.getflag = 0;

			rv = dx83_software_reset_func(&dx83);
			printf("dx83_software_reset res:%u\r\n",rv);	
		}
		usleep(100000);

		dx83_get_chip_info_func = dlsym(dx83_handle,"DX83_GetChipInfo");
		if (dx83_get_chip_info_func)
		{
			rv = dx83_get_chip_info_func(&dx83);
			printf("get chip info res:%u flag:%u\r\n",rv,dx83.getflag);			
		}

		p_dx83_get_random_func = dlsym(dx83_handle,"DX83_GetRandom");
		if (p_dx83_get_random_func)
		{
			memset(tmpkey,0xff,32);
			rv = p_dx83_get_random_func(&dx83,seed,32);
			printf("p_dx83_get_random_func res:%u\r\n",rv);
		}

		p_DX83_VerifyKey_func = dlsym(dx83_handle,"DX83_VerifyKey");
		if (p_DX83_VerifyKey_func)
		{
			rv = p_DX83_VerifyKey_func(&dx83,0x08,seed,tmpkey);
			printf("p_DX83_VerifyKey_func res:%u\r\n",rv);
		}

		//pDX83_ImportKey_func = dlsym(dx83_handle,"DX83_ImportKey");
		//if (pDX83_ImportKey_func)
		//{
		//	rv = pDX83_ImportKey_func(&dx83, 0x48, sm4EncKey, 32);
		//	printf("pDX83_ImportKey_func res:%u\r\n",rv);			
		//}
		//printf("DX83_ImportKey finish\r\n");
#if 0
		pjiami_func = dlsym(dx83_handle,"SM4_Encrypt");
		if (pjiami_func)
		{
			rv = pjiami_func(pdat, 64, jiami_dat);
			printf("SM4_Encrypt = %d\r\n", rv);
		}

		pjiemi_func = dlsym(dx83_handle,"SM4_Decrypt");
		if (pjiemi_func)
		{
			rv = pjiemi_func(jiami_dat, 64, jiemi_dat);
			printf("jiemi result:%s(%uBytes)\r\n",jiemi_dat,rv);
		}
#else
	//SM4 ECB CBC CFB OFB CTR
	for(i=0;i<5;i++)
	{
		p_DX83_CipherInit_func = dlsym(dx83_handle,"DX83_CipherInit");
		if (p_DX83_CipherInit_func)
		{
			rv = p_DX83_CipherInit_func(&dx83,ALG2_SM4_ECB+i,0x48,tmpiv);
			printf("[%u]p_DX83_CipherInit_func res:%u\r\n",i,rv);
		}

		p_DX83_CipherEncrypt_func = dlsym(dx83_handle,"DX83_CipherEncrypt");
		if (p_DX83_CipherEncrypt_func)
		{
			rv = p_DX83_CipherEncrypt_func(&dx83,pdat,after_jiami_len,jiami_dat);
			printf("p_DX83_CipherEncrypt_func res:%u\r\n",rv);
		}

		p_DX83_CipherInit_func = dlsym(dx83_handle,"DX83_CipherInit");
		if (p_DX83_CipherInit_func)
		{
			rv = p_DX83_CipherInit_func(&dx83,ALG2_SM4_ECB+i,0x48,tmpiv);
			printf("p_DX83_CipherInit_func res:%u\r\n",rv);
		}

		p_DX83_CipherDecrypt_func = dlsym(dx83_handle,"DX83_CipherDecrypt");
		if (p_DX83_CipherDecrypt_func)
		{
			unsigned int j = 0;

			memset(jiemi_dat,0,sizeof(jiemi_dat));
			rv = p_DX83_CipherDecrypt_func(&dx83,jiami_dat,after_jiami_len,jiemi_dat);
			printf("p_DX83_CipherDecrypt_func res:%u\r\n",rv);

			for (j = 0; j < after_jiami_len; j++)
			{
				printf("%02X",jiemi_dat[j]);
			}
			printf("\r\n");
		}
	}
#endif
		printf("func addr:%08X %08X %08X %08X\r\n",pinit_sm9,pDX83_ImportKey_func,pjiami_func,pjiemi_func);
		if (dx83_handle) dlclose(dx83_handle);
	}

	#if 0
	SM9_Init();
	rv = DX83_ImportKey(&dx83, 0x48, sm4EncKey, 32);
	printf("rv:%d\r\n",rv);

 	rv = SM4_Encrypt(pdat, strlen((char *)pdat), enc_dat);
    printf("SM4_Encrypt = %d\r\n", rv);

    rv = SM4_Decrypt(enc_dat, rv, jiemi_dat);
    printf("jiemi result:%s(%uBytes)\r\n",jiemi_dat,rv);

	//handshake();
	#endif
}


void dx83_test2(void)
{
	void *dx83_handle = 0;
	handshake_func phandshake_func = 0;
	int ret = 0;

	dx83_handle = dlopen("/usr/lib/libsm9.so",RTLD_NOW);
	printf("dx83_handle:%08X\r\n",dx83_handle);

	if (dx83_handle)
	{
		phandshake_func = dlsym(dx83_handle,"handshake");
		if (phandshake_func)
		{
			ret = phandshake_func();
			printf("handshake result:%d\r\n",ret);
		}

		if (dx83_handle) dlclose(dx83_handle);
	}
}

void dx83_poweron_self_check(void)
{
	void *dx83_handle = 0;
	dx83_func dx83_get_chip_info_func = 0;
	DX83_GetRandom_func p_dx83_get_random_func = 0;

	unsigned char seed[32] = {0};
	unsigned char tmpkey[64] = {0};
    unsigned char rv = 0;

	dx83_handle = dlopen("/usr/lib/libsm9.so",RTLD_NOW);
	printf("dx83_handle:%08X\r\n",dx83_handle);

	if (dx83_handle)
	{
		dx83.sla = 0x30;
		dx83.getflag = 0;

		dx83_get_chip_info_func = dlsym(dx83_handle,"DX83_GetChipInfo");
		if (dx83_get_chip_info_func)
		{
			rv = dx83_get_chip_info_func(&dx83);
			printf("get chip info res:%u flag:%u\r\n",rv,dx83.getflag);			
		}

		p_dx83_get_random_func = dlsym(dx83_handle,"DX83_GetRandom");
		if (p_dx83_get_random_func)
		{
			memset(tmpkey,0xff,32);
			rv = p_dx83_get_random_func(&dx83,seed,32);
			printf("p_dx83_get_random_func res:%u\r\n",rv);
		}

		if (rv == 0)
		{
			set_jiami_chip_status(1);
		}
		else
		{
			set_jiami_chip_status(0);
		}

		if (dx83_handle) dlclose(dx83_handle);
	}
	else
	{
			set_jiami_chip_status(0);
	}
}

/**
 * @description: SM4加密时填充分组以满足算法要求
 * @param src 待填充的分组
 * @param srclen 待填充的分组长度
 * @return 填充后的长度
 */
unsigned int pedding(unsigned char *src, unsigned int srclen)
{
    unsigned int i = 0;
    unsigned int pad_num = 16 - srclen % 16;

	if ((src == 0) || (srclen == 0))
	{
		return 0;
	}

    for (i = 0; i < pad_num; i++)
    {
        src[srclen + i] = pad_num;
    }
    return (srclen + pad_num);
}


/**
 * @brief 
 * 
 * @param sm4_type  SM4_ECB     0
					SM4_CBC     1
					SM4_CFB     2
					SM4_OFB     3
					SM4_CTR     4
 * @param psrc 
 * @param srclen 
 * @param pOut 
 * @param outbuflen 
 * @return unsigned int 
 */
unsigned int sm4_enc(unsigned char sm4_type,void *psrc,unsigned int srclen,void *pOut,unsigned int outbuflen)
{
	void *dx83_handle = 0;
	dx83_func dx83_software_reset_func = 0;
	dx83_func dx83_get_chip_info_func = 0;
	DX83_GetRandom_func p_dx83_get_random_func = 0;
	DX83_VerifyKey_func p_DX83_VerifyKey_func = 0;

	DX83_CipherInit_func p_DX83_CipherInit_func = 0;
	DX83_CipherEncrypt_func p_DX83_CipherEncrypt_func = 0;

	unsigned char seed[32] = {0};
	unsigned char tmpkey[64] = {0};
    unsigned char rv = 0;
	unsigned char tmpiv[16] = {0};
	unsigned int rlen = 0;

	if ((psrc == 0) || (srclen == 0) || (pOut == 0) || (outbuflen < srclen))
	{
		return 0;
	}

	dx83_handle = dlopen("/usr/lib/libsm9.so",RTLD_NOW);
	printf("dx83_handle:%08X\r\n",dx83_handle);
	if (dx83_handle)
	{
		memset(tmpiv,0x11,16);
		rlen = pedding(psrc,srclen);

		dx83_software_reset_func = dlsym(dx83_handle,"DX83_SoftReset");
		if (dx83_software_reset_func)
		{
			dx83.sla = 0x30;
			dx83.getflag = 0;

			rv = dx83_software_reset_func(&dx83);
			printf("dx83_software_reset res:%u\r\n",rv);	
		}
		usleep(100000);

		dx83_get_chip_info_func = dlsym(dx83_handle,"DX83_GetChipInfo");
		if (dx83_get_chip_info_func)
		{
			rv = dx83_get_chip_info_func(&dx83);
			printf("get chip info res:%u flag:%u\r\n",rv,dx83.getflag);			
		}

		p_dx83_get_random_func = dlsym(dx83_handle,"DX83_GetRandom");
		if (p_dx83_get_random_func)
		{
			memset(tmpkey,0xff,32);
			rv = p_dx83_get_random_func(&dx83,seed,32);
			printf("p_dx83_get_random_func res:%u\r\n",rv);
		}

		p_DX83_VerifyKey_func = dlsym(dx83_handle,"DX83_VerifyKey");
		if (p_DX83_VerifyKey_func)
		{
			rv = p_DX83_VerifyKey_func(&dx83,0x08,seed,tmpkey);
			printf("p_DX83_VerifyKey_func res:%u\r\n",rv);
		}

		//SM4 ECB CBC CFB OFB CTR
		p_DX83_CipherInit_func = dlsym(dx83_handle,"DX83_CipherInit");
		if (p_DX83_CipherInit_func)
		{
			rv = p_DX83_CipherInit_func(&dx83,ALG2_SM4_ECB + sm4_type,0x48,tmpiv);
			printf("[%u]p_DX83_CipherInit_func res:%u\r\n",sm4_type,rv);
		}

		p_DX83_CipherEncrypt_func = dlsym(dx83_handle,"DX83_CipherEncrypt");
		if (p_DX83_CipherEncrypt_func)
		{
			rv = p_DX83_CipherEncrypt_func(&dx83,psrc,rlen,pOut);
			printf("p_DX83_CipherEncrypt_func res:%u\r\n",rv);
		}

		if (dx83_handle) dlclose(dx83_handle);
	}
	return rlen;
}
