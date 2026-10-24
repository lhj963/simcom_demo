#include "jmrh_pro.h"


typedef struct tagJMRH_PROStruction
{
    unsigned int seque;
    unsigned int (*enc_func)(void *psrc,unsigned int srclen,void *pOut,unsigned int outbuflen);

}JMRH_PROStruction;

JMRH_PROStruction s_stJMRH_PROStruction = {0};


void jmrh_init(unsigned int (*penc_func)(void *psrc,unsigned int srclen,void *pOut,unsigned int outbuflen))
{
    s_stJMRH_PROStruction.enc_func = penc_func;
}


unsigned int jmrh_pack(void *pgb17691pack,unsigned int packlen,void *pOut,unsigned int outbuflen)
{
    unsigned char *ptr = (unsigned char *)pOut;
    unsigned int rlen = 0;

    unsigned char cmd = 0;
    unsigned char ver = 0x01;
    unsigned int datalen_after_enc = 0;
    unsigned char *penc = 0;
    unsigned char vin[17] = {0};

    if ((pgb17691pack == 0) || (pOut == 0) || (outbuflen < (4+1+1+1+4+1+17+17+4)))
    {
        return 0;
    }

    cmd = *((unsigned char *)pgb17691pack + 2);
    //pvin = (unsigned char *)pgb17691pack + 3;
    memcpy(vin,(unsigned char *)pgb17691pack + 3,17);
    //ver = *((unsigned char *)pgb17691pack + 20);

    penc = ptr + (4+1+1+1+4+1+17+4);

    //magic
    ptr[rlen++] = (unsigned char)(62283748 >> 24);
    ptr[rlen++] = (unsigned char)(62283748 >> 16);
    ptr[rlen++] = (unsigned char)(62283748 >> 8);
    ptr[rlen++] = 62283748 & 0x00FF;

    //ver
    ptr[rlen++] = ver;

    //serial type
    ptr[rlen++] = 0x01;

    //cmd
    ptr[rlen++] = cmd;

    //seque
    ptr[rlen++] = s_stJMRH_PROStruction.seque >> 24;
    ptr[rlen++] = s_stJMRH_PROStruction.seque >> 16;
    ptr[rlen++] = s_stJMRH_PROStruction.seque >> 8;
    ptr[rlen++] = s_stJMRH_PROStruction.seque;
    s_stJMRH_PROStruction.seque++;

    //srv
    ptr[rlen++] = 0x00;

    //msglen + 17
    if (s_stJMRH_PROStruction.enc_func)
    {
        datalen_after_enc = s_stJMRH_PROStruction.enc_func(pgb17691pack,packlen,penc,outbuflen - (4+1+1+1+4+1+17+4));
    }

    ptr[rlen++] = (datalen_after_enc + 17) >> 24;
    ptr[rlen++] = (datalen_after_enc + 17) >> 16;
    ptr[rlen++] = (datalen_after_enc + 17) >> 8;
    ptr[rlen++] = (datalen_after_enc + 17);

    memcpy(ptr + rlen,vin,17);
    rlen += 17;

    if (datalen_after_enc)
    {
        //memcpy(ptr + rlen,penc,datalen_after_enc);
    }
    rlen += datalen_after_enc;

    return rlen;
}

void (*jmrh_ota_cb_func)(OTAParam *);

void reg_jmrh_ota_cb_func(void (*pf)(OTAParam *))
{
    jmrh_ota_cb_func = pf;
}

void jmrh_analyze_plat_data(void *pdat,unsigned int len)
{
    if (len > (8 + 1 + 1 + 1 + 16 + 16))
    {
        OTAParam stOTAParam = {0};

        unsigned char *purl = 0;
        char url[256] = {0};
        char usr[128] = {0};
        char psw[128] = {0};
        unsigned char urllen = 0;
        unsigned char usrlen = 0;
        unsigned char pswlen = 0;

        purl = _find_sub_str(pdat,len,"ftp",3);
        if (purl && ((purl - (unsigned char *)pdat) >= 8))
        {
            urllen = *(purl - 1);
            if (urllen > (len - (unsigned int)(purl - (unsigned char *)pdat))) return ;
            if (urllen) memcpy(url,purl,urllen);
            usrlen = *(purl + urllen);
            if (usrlen > (len - (unsigned int)(purl - (unsigned char *)pdat) - urllen - 1)) return ;
            if (usrlen >= 128) return ;
            if (usrlen) memcpy(usr,purl + urllen + 1,usrlen);
            pswlen = *(purl + urllen + 1 + usrlen);
            if (pswlen > (len - (unsigned int)(purl - (unsigned char *)pdat) - urllen - 1 - usrlen - 1)) return ;
            if (pswlen >= 128) return ;
            if (pswlen) memcpy(psw,purl + urllen + 1 + usrlen + 1,pswlen);

            if (jmrh_ota_cb_func)
            {
                stOTAParam.url = url;
                stOTAParam.usr = usr;
                stOTAParam.psw = psw;
                jmrh_ota_cb_func(&stOTAParam);
            }
        }
        else
        {
            purl = _find_sub_str(pdat,len,"http",4);
        }
    }

}




