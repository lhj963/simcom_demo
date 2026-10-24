#ifndef __HJ1239_H__
#define __HJ1239_H__
#ifdef __cplusplus
extern "C" {
#endif


extern void hj1239_proc_init(unsigned int (*sign_proc)(void *pdat,unsigned int datlen,void *pOut,unsigned int outbuflen));

extern unsigned int hj1239_data_pack(void *pHJ1239FormatData,unsigned int datlen);


extern void *get_beian_msg_body_with_sign_ptr(void);




#ifdef __cplusplus
}
#endif

#endif



