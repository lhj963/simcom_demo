#ifndef __DX83_TEST_OLD_H__
#define __DX83_TEST_OLD_H__
#ifdef __cplusplus
extern "C" {
#endif

extern void dx83_test(void);

extern void dx83_poweron_self_check(void);

extern unsigned int sm4_enc(unsigned char sm4_type,void *psrc,unsigned int srclen,void *pOut,unsigned int outbuflen);


#ifdef __cplusplus
}
#endif


#endif

