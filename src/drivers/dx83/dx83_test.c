#include "dx83_test.h"
#include "dlfcn.h"
#include "DX83_API.h"
// #include "sm9.h"
DX83_CHIP dx83;
DX83_CHIP *p_dx83;

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

typedef unsigned char (*SM2_sign_func)(DX83_CHIP *dx83, unsigned char *message, int message_len, unsigned char *sign);

typedef unsigned char (*SM2_sign_UID_func)(DX83_CHIP *dx83, const char *chipid, unsigned char *message, int message_len, unsigned char *sign);

typedef unsigned char (*SM2_getPublicKey_func)(DX83_CHIP *dx83, unsigned char *pubkey);

typedef unsigned char (*SM2_getPublicKey2_func)(DX83_CHIP *dx83, unsigned char *pubkey);

enum KeyType
{
	EncryptMasterPublicKey,
	SignMasterPublicKey,
	EncryptUserPrivateKey,
	SignUserPrivateKey,
	ChangeMasterPublicKey,
	SM4Key,
	UserPublicKey_UserID
};

typedef unsigned char (*SM9_importKey_func)(DX83_CHIP *dx83, char *key, enum KeyType keyType);
typedef int (*new_handshake_func)(DX83_CHIP *dx83, const char *vinCode, char *ip, char *port);
typedef DX83_CHIP *(*SM9_init_func)(void);
typedef unsigned char (*SM4_Decrypt_func)(DX83_CHIP *dx83, unsigned char *cipher, int c_len, unsigned char *text);
typedef unsigned char (*SM4_Encrypt_func)(DX83_CHIP *dx83, unsigned char *text, int t_len, unsigned char *cipher);
typedef int (*getKeys_func)(DX83_CHIP *dx83, const char *vinCode, char *ip, char *port);

typedef int (*car2car_func)(DX83_CHIP *m_dx83, char *m_VIN1, char *m_VIN2);

void dx83_test(void)
{
#if 0
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
	printf("dx83_handle:%08X\r\n",(unsigned int)dx83_handle);

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
		//printf("func addr:%08X %08X %08X %08X\r\n",pinit_sm9,pDX83_ImportKey_func,pjiami_func,pjiemi_func);
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
#endif
}

void dx83_test2(void)
{
	void *dx83_handle = 0;
	handshake_func phandshake_func = 0;
	int ret = 0;

	dx83_handle = dlopen("/usr/lib/libsm9.so", RTLD_NOW);
	printf("dx83_handle:%08X\r\n", (unsigned int)dx83_handle);

	if (dx83_handle)
	{
		phandshake_func = dlsym(dx83_handle, "handshake");
		if (phandshake_func)
		{
			ret = phandshake_func();
			printf("handshake result:%d\r\n", ret);
		}

		if (dx83_handle)
			dlclose(dx83_handle);
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

	dx83_handle = dlopen("/usr/lib/libsm9.so", RTLD_NOW);
	printf("dx83_handle:%08X\r\n", (unsigned int)dx83_handle);

	if (dx83_handle)
	{
		dx83.sla = 0x30;
		dx83.getflag = 0;

		dx83_get_chip_info_func = dlsym(dx83_handle, "DX83_GetChipInfo");
		if (dx83_get_chip_info_func)
		{
			rv = dx83_get_chip_info_func(&dx83);
			printf("get chip info res:%u flag:%u\r\n", rv, dx83.getflag);
		}

		p_dx83_get_random_func = dlsym(dx83_handle, "DX83_GetRandom");
		if (p_dx83_get_random_func)
		{
			memset(tmpkey, 0xff, 32);
			rv = p_dx83_get_random_func(&dx83, seed, 32);
			printf("p_dx83_get_random_func res:%u\r\n", rv);
		}

		if (rv == 0)
		{
			set_jiami_chip_status(1);
		}
		else
		{
			set_jiami_chip_status(0);
		}

		if (dx83_handle)
			dlclose(dx83_handle);
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
unsigned int sm4_enc(unsigned char sm4_type, void *psrc, unsigned int srclen, void *pOut, unsigned int outbuflen)
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

	dx83_handle = dlopen("/usr/lib/libsm9.so", RTLD_NOW);
	// printf("dx83_handle:%08X\r\n",(unsigned int)dx83_handle);
	if (dx83_handle)
	{
		memset(tmpiv, 0x11, 16);
		rlen = pedding(psrc, srclen);

		dx83_software_reset_func = dlsym(dx83_handle, "DX83_SoftReset");
		if (dx83_software_reset_func)
		{
			dx83.sla = 0x30;
			dx83.getflag = 0;

			rv = dx83_software_reset_func(&dx83);
			printf("dx83_software_reset res:%u\r\n", rv);
		}
		usleep(100000);

		dx83_get_chip_info_func = dlsym(dx83_handle, "DX83_GetChipInfo");
		if (dx83_get_chip_info_func)
		{
			rv = dx83_get_chip_info_func(&dx83);
			printf("get chip info res:%u flag:%u\r\n", rv, dx83.getflag);
		}

		p_dx83_get_random_func = dlsym(dx83_handle, "DX83_GetRandom");
		if (p_dx83_get_random_func)
		{
			memset(tmpkey, 0xff, 32);
			rv = p_dx83_get_random_func(&dx83, seed, 32);
			printf("p_dx83_get_random_func res:%u\r\n", rv);
		}

		p_DX83_VerifyKey_func = dlsym(dx83_handle, "DX83_VerifyKey");
		if (p_DX83_VerifyKey_func)
		{
			rv = p_DX83_VerifyKey_func(&dx83, 0x08, seed, tmpkey);
			printf("p_DX83_VerifyKey_func res:%u\r\n", rv);
		}

		// SM4 ECB CBC CFB OFB CTR
		p_DX83_CipherInit_func = dlsym(dx83_handle, "DX83_CipherInit");
		if (p_DX83_CipherInit_func)
		{
			rv = p_DX83_CipherInit_func(&dx83, ALG2_SM4_ECB + sm4_type, 0x48, tmpiv);
			printf("[%u]p_DX83_CipherInit_func res:%u\r\n", sm4_type, rv);
		}

		p_DX83_CipherEncrypt_func = dlsym(dx83_handle, "DX83_CipherEncrypt");
		if (p_DX83_CipherEncrypt_func)
		{
			rv = p_DX83_CipherEncrypt_func(&dx83, psrc, rlen, pOut);
			printf("p_DX83_CipherEncrypt_func res:%u\r\n", rv);
		}

		if (dx83_handle)
			dlclose(dx83_handle);
	}
	return rlen;
}

void *dx83_dll_handle = 0;

void dx83_lib_init(void)
{
	SM9_init_func p_sm9_init_func = 0;
	dx83_func dx83_get_chip_info_func = 0;
	DX83_GetRandom_func p_dx83_get_random_func = 0;
	unsigned char seed[32] = {0};
	unsigned char tmpkey[64] = {0};
	unsigned char rv = 0;
	// DBG_Print(LEVEL2, "1dx83_lib_init\n");
	dx83_dll_handle = dlopen("/usr/lib/libsm9.so", RTLD_NOW);
	// DBG_Print(LEVEL2, "dx83_handle:%08X\r\n", (unsigned int)dx83_dll_handle);
	if (dx83_dll_handle)
	{
		p_sm9_init_func = dlsym(dx83_dll_handle, "SM9_init");
		DBG_Print(LEVEL2, "p_sm9_init_func:%08X\r\n", (unsigned int)p_sm9_init_func);
		if (p_sm9_init_func)
		{
			// DBG_Print(LEVEL2, "2p_sm9_init_func:%08X\r\n", (unsigned int)p_sm9_init_func);
			p_dx83 = p_sm9_init_func();
		}
		// DBG_Print(LEVEL2, "SM9_init finish(sla:%02X flag:%d)\r\n", p_dx83->sla, p_dx83->getflag);
		// DBG_Print(LEVEL2, "9dx83_lib_init\n");
		dx83.sla = 0x30;
		dx83.getflag = 0;
		dx83_get_chip_info_func = dlsym(dx83_dll_handle, "DX83_GetChipInfo");
		if (dx83_get_chip_info_func)
		{
			rv = dx83_get_chip_info_func(&dx83);
			// DBG_Print(LEVEL2, "get chip info res:%u flag:%u\r\n", rv, dx83.getflag);
		}
		// DBG_Print(LEVEL2, "8dx83_lib_init\n");
		p_dx83_get_random_func = dlsym(dx83_dll_handle, "DX83_GetRandom");
		if (p_dx83_get_random_func)
		{
			memset(tmpkey, 0xff, 32);
			rv = p_dx83_get_random_func(&dx83, seed, 32);
			// DBG_Print(LEVEL2, "p_dx83_get_random_func res:%u\r\n", rv);
		}
		// DBG_Print(LEVEL2, "7dx83_lib_init\n");
		if (rv == 0)
		{
			// DBG_Print(LEVEL2, "2dx83_lib_init\n");
			set_jiami_chip_status(1);
		}
		else
		{
			// DBG_Print(LEVEL2, "3dx83_lib_init\n");
			set_jiami_chip_status(0);
		}
	}
	else
	{
		// DBG_Print(LEVEL2, "4dx83_lib_init\n");
		set_jiami_chip_status(0);
	}
}

void dx83_test3(void)
{
	SM9_init_func p_sm9_init_func = 0;
	SM4_Encrypt_func p_SM4_Encrypt_func = 0;
	unsigned char pdat[] = {0x23, 0x23, 0x01, 0x4C, 0x47, 0x44, 0x43, 0x48, 0x41, 0x31, 0x47, 0x38, 0x4D, 0x41, 0x31, 0x32,
							0x37, 0x37, 0x33, 0x33, 0x01, 0x01, 0x00, 0x1C, 0x15, 0x08, 0x0E, 0x08, 0x2F, 0x16, 0x00, 0x01,
							0x38, 0x39, 0x38, 0x36, 0x30, 0x34, 0x39, 0x33, 0x31, 0x39, 0x32, 0x31, 0x39, 0x30, 0x34, 0x39,
							0x34, 0x33, 0x32};
	unsigned char rv = {0};
	unsigned char buf[512] = {0};
	unsigned int i = 0;

	if (dx83_dll_handle)
	{
		p_sm9_init_func = dlsym(dx83_dll_handle, "SM9_init");
		if (p_sm9_init_func)
		{
			p_dx83 = p_sm9_init_func();
		}
		DBG_Print(LEVEL2, "SM9_init finish(sla:%02X flag:%d)\r\n", p_dx83->sla, p_dx83->getflag);

		p_SM4_Encrypt_func = dlsym(dx83_dll_handle, "SM4_Encrypt");
		if (p_SM4_Encrypt_func)
		{
			rv = p_SM4_Encrypt_func(p_dx83, (unsigned char *)pdat, sizeof(pdat), buf);
			DBG_Print(LEVEL2, "dx83_test3 rv:%u\r\n", rv);

			for (i = 0; i < 128; i++)
			{
				DBG_Print(LEVEL2, "%02X", buf[i]);
			}
			DBG_Print(LEVEL2, "\r\n");
		}
	}
}

unsigned int sm4_enc1(unsigned char sm4_type, void *psrc, unsigned int srclen, void *pOut, unsigned int outbuflen)
{
	SM4_Encrypt_func p_SM4_Encrypt_func = 0;
	unsigned char rv = 1;
	unsigned int rlen = 0;
	unsigned int templen = 0;
	unsigned int offset = 0;
	unsigned char tempbuf[240] = {0};

	if ((psrc == 0) || (srclen == 0) || (pOut == 0) || (outbuflen < srclen))
	{
		return 0;
	}

	if (dx83_dll_handle)
	{
		p_SM4_Encrypt_func = dlsym(dx83_dll_handle, "SM4_Encrypt");
		if (p_SM4_Encrypt_func)
		{
			if ((srclen % 16) == 0)
			{
				rlen = srclen + 16; // the last bag need pedding
			}
			else
			{
				rlen = ((srclen / 16) + 1) * 16;
			}

			while (srclen)
			{
				if (srclen >= 224)
				{
					templen = 224;
				}
				else
				{
					templen = srclen;
				}

				memcpy(tempbuf, (unsigned char *)psrc + offset, templen);
				if (srclen == templen) // the last bag need pedding
				{
					// templen = pedding(tempbuf,templen);
				}
				rv = p_SM4_Encrypt_func(p_dx83, tempbuf, templen, (unsigned char *)pOut + offset);
				if (rv)
				{
					rlen = 0;
					break;
				}

				// printf("sm4_enc1 rv:%u enc len:%u\r\n",rv,templen);

				if (srclen >= templen)
				{
					srclen -= templen;
				}
				else
				{
					srclen = 0;
				}
				offset += templen;
			}
		}
	}

	return rlen;
}

int handshake_plat(void *pvin, void *ip, unsigned int port)
{
	int ret = 1;
	getKeys_func p_getKeys_func = 0;
	new_handshake_func p_new_handshake_func = 0;
	char szport[16] = {0};

	if ((pvin == 0) || (ip == 0) || (port == 0) || (dx83_dll_handle == 0))
	{
		return 1;
	}

	sprintf(szport, "%u", port);
	p_getKeys_func = dlsym(dx83_dll_handle, "getKeys");
	if (p_getKeys_func)
	{
		p_getKeys_func(p_dx83, (const char *)pvin, (char *)ip, szport);
	}

	p_new_handshake_func = dlsym(dx83_dll_handle, "handshake");
	if (p_new_handshake_func)
	{
		ret = p_new_handshake_func(p_dx83, (const char *)pvin, (char *)ip, szport);
	}
	return ret;
}

unsigned char dx83_get_public_key(void *pOut, unsigned int outbuflen)
{
	unsigned char ret = 1;
	SM2_getPublicKey_func p_SM2_getPublicKey_func = 0;

#if 0
	SM2_getPublicKey2_func p_SM2_getPublicKey2_func = 0;

	if (dx83_dll_handle)
	{
		p_SM2_getPublicKey2_func = dlsym(dx83_dll_handle,"SM2_getPublicKey2");
		if (p_SM2_getPublicKey2_func && pOut)
		{
			ret = p_SM2_getPublicKey2_func(p_dx83,(unsigned char *)s_public_key);
		}
	}
#else
	if (dx83_dll_handle)
	{
		p_SM2_getPublicKey_func = dlsym(dx83_dll_handle, "SM2_getPublicKey");
		if (p_SM2_getPublicKey_func && pOut)
		{
			ret = p_SM2_getPublicKey_func(p_dx83, (unsigned char *)pOut);
		}
	}
#endif

	return ret;
}

unsigned int get_sign(void *pdat, unsigned int datlen, void *pOut, unsigned int outbuflen)
{
	unsigned char ret = 0;
	// SM2_sign_func p_SM2_sign_func = 0;
	SM2_sign_UID_func p_SM2_sign_UID_func = 0;

	if ((pOut == 0) || (outbuflen == 0) || (pdat == 0) || (datlen == 0) || (dx83_dll_handle == 0))
	{
		return 0;
	}

#if 0
	p_SM2_sign_func = dlsym(dx83_dll_handle,"SM2_sign");
	if (p_SM2_sign_func)
	{
		//typedef unsigned char (*SM2_sign_func)(DX83_CHIP *dx83, unsigned char *message, int message_len, unsigned char *sign);
		ret = p_SM2_sign_func(p_dx83,(unsigned char *)pdat,datlen,(unsigned char *)pOut);
	}
#else
	p_SM2_sign_UID_func = dlsym(dx83_dll_handle, "SM2_sign_UID");
	if (p_SM2_sign_UID_func)
	{
		ret = p_SM2_sign_UID_func(p_dx83, (const char *)sysparam_get_jiami_chip_id(), (unsigned char *)pdat, datlen, (unsigned char *)pOut);
	}

#endif

	if (ret == 0)
	{
		ret = 64;
	}
	else
	{
		ret = 0;
	}
	return ret;
}

void import_sm4_key(void *key, unsigned int keylen)
{
	DX83_ImportKey_func pDX83_ImportKey_func = 0;
	SM9_importKey_func p_SM9_importKey_func = 0;
	unsigned char rv = 0;
	unsigned int i = 0;
	unsigned char *sm4_key = (unsigned char *)key;
#if 0
	unsigned char sm4_key[] = {
        0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10,
        0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10};
#endif
	p_SM9_importKey_func = p_SM9_importKey_func;

	if (dx83_dll_handle)
	{
		pDX83_ImportKey_func = dlsym(dx83_dll_handle, "DX83_ImportKey");
		if (pDX83_ImportKey_func)
		{
			printf("import sm4 key:");
			for (i = 0; i < keylen; i++)
			{
				printf("%02X", sm4_key[i]);
			}
			printf("\r\n");
			rv = pDX83_ImportKey_func(p_dx83, 0x48, sm4_key, keylen);
			printf("pDX83_ImportKey_func res:%u\r\n", rv);
		}
		printf("DX83_ImportKey finish\r\n");

#if 0
		p_SM9_importKey_func = dlsym(dx83_dll_handle,"SM9_importKey");
		if (p_SM9_importKey_func)
		{
			rv = p_SM9_importKey_func(p_dx83,sm4_key,SM4Key);
			printf("import_sm4_key rv:%u\r\n",rv);
		}
#endif
	}
}

typedef long (*decryptPackage_func)(DX83_CHIP *p_chip, char *fileName, unsigned char **res);

int unpack_ota_file(void *enc_filename, void *dec_filename)
{
	if (dx83_dll_handle && enc_filename)
	{
		long rlen = 0;
		unsigned char *pbuf = 0;
		decryptPackage_func p_decryptPackage_func = 0;

		p_decryptPackage_func = dlsym(dx83_dll_handle, "decryptPackage");
		if (p_decryptPackage_func)
		{
			rlen = p_decryptPackage_func(p_dx83, enc_filename, &pbuf);
			if ((rlen > 0) && (rlen < (5 * 1024 * 1024)) && pbuf && dec_filename)
			{
				mywritefile(0, dec_filename, pbuf, rlen);
			}
			if (pbuf)
				free(pbuf);
			printf("dec file size:%u\r\n", (unsigned int)rlen);
			if (rlen > 0)
			{
				return 0;
			}
			else if (rlen == 0)
			{
				return 8;
			}
			else
			{
				return rlen;
			}
		}
	}
	return 1;
}

int car_to_car(void *vin_src, void *vin_des)
{
	// typedef int (*car2car_func)(DX83_CHIP *m_dx83, char *m_VIN1, char *m_VIN2);
	car2car_func pf_car2car = 0;

	if (dx83_dll_handle)
	{
		pf_car2car = dlsym(dx83_dll_handle, "car2car");
		if (pf_car2car)
		{
			return pf_car2car(p_dx83, (char *)vin_src, (char *)vin_des);
		}
	}
	return 1;
}

void dx83_press_test(void)
{
	unsigned int cnt = 0;
	unsigned int rlen = 0;
	unsigned char buf[512] = {0};
	unsigned char pdat[] = {
		0x23,
		0x23,
		0x01,
		0x4C,
		0x47,
		0x44,
		0x43,
		0x48,
		0x41,
		0x31,
		0x47,
		0x38,
		0x4D,
		0x41,
		0x31,
		0x32,
		0x37,
		0x37,
		0x33,
		0x33,
		0x01,
		0x01,
		0x00,
		0x1C,
		0x15,
		0x08,
		0x0E,
		0x08,
		0x2F,
		0x16,
		0x00,
		0x01,
		0x38,
		0x39,
		0x38,
		0x36,
		0x30,
		0x34,
		0x39,
		0x33,
		0x31,
		0x39,
		0x32,
		0x31,
		0x39,
		0x30,
		0x34,
		0x39,
		0x34,
		0x33,
		0x32,
		0x23,
		0x23,
		0x01,
		0x4C,
		0x47,
		0x44,
		0x43,
		0x48,
		0x41,
		0x31,
		0x47,
		0x38,
		0x4D,
		0x41,
		0x31,
		0x32,
		0x37,
		0x37,
		0x33,
		0x33,
		0x01,
		0x01,
		0x00,
		0x1C,
		0x15,
		0x08,
		0x0E,
		0x08,
		0x2F,
		0x16,
		0x00,
		0x01,
		0x38,
		0x39,
		0x38,
		0x36,
		0x30,
		0x34,
		0x39,
		0x33,
		0x31,
		0x39,
		0x32,
		0x31,
		0x39,
		0x30,
		0x34,
		0x39,
		0x34,
		0x33,
		0x32,
		0x23,
		0x23,
		0x01,
		0x4C,
		0x47,
		0x44,
		0x43,
		0x48,
		0x41,
		0x31,
		0x47,
		0x38,
		0x4D,
		0x41,
		0x31,
		0x32,
		0x37,
		0x37,
		0x33,
		0x33,
		0x01,
		0x01,
		0x00,
		0x1C,
		0x15,
		0x08,
		0x0E,
		0x08,
		0x2F,
		0x16,
		0x00,
		0x01,
		0x38,
		0x39,
		0x38,
		0x36,
		0x30,
		0x34,
		0x39,
		0x33,
		0x31,
		0x39,
		0x32,
		0x31,
		0x39,
		0x30,
		0x34,
		0x39,
		0x34,
		0x33,
		0x32,
	};

	while (1)
	{
		usleep(100000);
		rlen = sm4_enc1(0, pdat, sizeof(pdat), buf, 512);
		if (rlen == 0)
		{
			cnt++;
			mywritefile(1, "/data/dx83_err.txt", "sm4 enc1 err\r\n", strlen("sm4 enc1 err\r\n"));
			if (cnt >= 4)
			{
				exit(1);
			}
		}
	}
}