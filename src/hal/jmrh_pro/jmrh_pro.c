#include "jmrh_pro.h"
#include <string.h>

extern unsigned int sm4_dec1(unsigned char sm4_type, void *psrc, unsigned int srclen, void *pOut, unsigned int outbuflen);
extern void *_find_sub_str(void *pbuf, unsigned int ulBufLen, void *SubStr, unsigned int ulSubStrLen);


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

static int jmrh_plain_token_ok(const unsigned char *s, unsigned int n, int is_url)
{
    unsigned int i;

    if (is_url)
    {
        if ((n < 6) || (n >= 256))
        {
            return 0;
        }
        if (memcmp(s, "ftp://", 6) != 0)
        {
            return 0;
        }
    }
    else if (n >= 128)
    {
        return 0;
    }

    for (i = 0; i < n; i++)
    {
        unsigned char c = s[i];
        int ok = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') ||
                 c == '.' || c == '_' || c == '-' || c == '+' || c == '@' || c == '%' || c == '#';

        if (is_url)
        {
            ok = ok || c == ':' || c == '/' || c == '?' || c == '&' || c == '=';
        }
        if (!ok)
        {
            return 0;
        }
    }
    return 1;
}

/* Decrypt a JMRH frame (or a raw SM4 block) before any OTA field is trusted. */
static int jmrh_decrypt_command(const unsigned char *in, unsigned int len, unsigned char *out, unsigned int outcap, unsigned int *outlen)
{
    static const unsigned char magic[4] = {0x03, 0xB6, 0xA1, 0xE4};
    unsigned int i;
    unsigned int n;
    int saw_frame = 0;

    if (outlen)
    {
        *outlen = 0;
    }
    if ((in == 0) || (out == 0) || (outlen == 0) || (len < 16) || (outcap < 16))
    {
        return -1;
    }

    for (i = 0; (i + 33) <= len; i++)
    {
        unsigned int msglen;
        unsigned int clen;

        if (memcmp(in + i, magic, 4) != 0)
        {
            continue;
        }
        saw_frame = 1;
        msglen = ((unsigned int)in[i + 12] << 24) | ((unsigned int)in[i + 13] << 16) |
                 ((unsigned int)in[i + 14] << 8) | (unsigned int)in[i + 15];
        if (msglen < 17)
        {
            continue;
        }
        clen = msglen - 17;
        if ((clen == 0) || (clen % 16) || ((i + 33 + clen) > len) || (clen > outcap))
        {
            continue;
        }
        n = sm4_dec1(0, (void *)(in + i + 33), clen, out, outcap);
        if (n)
        {
            *outlen = n;
            return 0;
        }
    }

    if (saw_frame)
    {
        return -1;
    }
    if (((len % 16) != 0) || (len > outcap))
    {
        return -1;
    }
    n = sm4_dec1(0, (void *)in, len, out, outcap);
    if (n == 0)
    {
        return -1;
    }
    *outlen = n;
    return 0;
}

static void jmrh_parse_ota_plain(unsigned char *pdat, unsigned int len)
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
        unsigned int used;

        purl = _find_sub_str(pdat, len, "ftp", 3);
        if (purl && ((purl - pdat) >= 8))
        {
            urllen = *(purl - 1);
            used = (unsigned int)(purl - pdat);
            if (urllen > (len - used))
            {
                return;
            }
            if (jmrh_plain_token_ok(purl, urllen, 1) == 0)
            {
                return;
            }
            memcpy(url, purl, urllen);
            if ((used + urllen + 1) > len)
            {
                return;
            }
            usrlen = *(purl + urllen);
            if (usrlen > (len - used - urllen - 1))
            {
                return;
            }
            if (jmrh_plain_token_ok(purl + urllen + 1, usrlen, 0) == 0)
            {
                return;
            }
            if (usrlen)
            {
                memcpy(usr, purl + urllen + 1, usrlen);
            }
            if ((used + urllen + 1 + usrlen + 1) > len)
            {
                return;
            }
            pswlen = *(purl + urllen + 1 + usrlen);
            if (pswlen > (len - used - urllen - 1 - usrlen - 1))
            {
                return;
            }
            if (jmrh_plain_token_ok(purl + urllen + 1 + usrlen + 1, pswlen, 0) == 0)
            {
                return;
            }
            if (pswlen)
            {
                memcpy(psw, purl + urllen + 1 + usrlen + 1, pswlen);
            }

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
            purl = _find_sub_str(pdat, len, "http", 4);
            (void)purl;
        }
    }
}

void jmrh_analyze_plat_data(void *pdat, unsigned int len)
{
    unsigned char plain[2048];
    unsigned int plain_len = 0;

    if ((pdat == 0) || (len == 0) || (len > sizeof(plain)))
    {
        return;
    }
    /* The socket buffer is ciphertext. Do not scan it for "ftp" until SM4 succeeds. */
    if (jmrh_decrypt_command((const unsigned char *)pdat, len, plain, sizeof(plain), &plain_len) != 0)
    {
        return;
    }
    jmrh_parse_ota_plain(plain, plain_len);
}




