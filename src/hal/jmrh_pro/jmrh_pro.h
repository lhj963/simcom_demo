#ifndef __JMRH_PRO_H__
#define __JMRH_PRO_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct OTAParam
{
    char *url;
    char *usr;
    char *psw;
}OTAParam;


extern void jmrh_init(unsigned int (*penc_func)(void *psrc,unsigned int srclen,void *pOut,unsigned int outbuflen));

extern unsigned int jmrh_pack(void *pgb17691pack,unsigned int packlen,void *pOut,unsigned int outbuflen);

extern void jmrh_analyze_plat_data(void *pdat,unsigned int len);

extern void reg_jmrh_ota_cb_func(void (*pf)(OTAParam *));

#ifdef __cplusplus
}
#endif

#endif

