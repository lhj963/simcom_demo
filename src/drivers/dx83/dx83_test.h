#ifndef __DX83_TEST_H__
#define __DX83_TEST_H__
#ifdef __cplusplus
extern "C" {
#endif

//old lib
extern void dx83_test(void);
extern void dx83_poweron_self_check(void);
extern unsigned int sm4_enc(unsigned char sm4_type,void *psrc,unsigned int srclen,void *pOut,unsigned int outbuflen);


//new lib
extern void dx83_lib_init(void);
extern void dx83_test3(void);
extern unsigned int sm4_enc1(unsigned char sm4_type,void *psrc,unsigned int srclen,void *pOut,unsigned int outbuflen);
extern unsigned int sm4_dec1(unsigned char sm4_type,void *psrc,unsigned int srclen,void *pOut,unsigned int outbuflen);

extern int handshake_plat(void *pvin, void *ip,unsigned int port);

extern void import_sm4_key(void *key,unsigned int keylen);

extern int unpack_ota_file(void *enc_filename,void *dec_filename);

extern void dx83_press_test(void);

extern unsigned char dx83_get_public_key(void *pOut,unsigned int outbuflen);

extern unsigned int get_sign(void *pdat,unsigned int datlen,void *pOut,unsigned int outbuflen);

extern int car_to_car(void *vin_src,void *vin_des);

#ifdef __cplusplus
}
#endif


#endif

