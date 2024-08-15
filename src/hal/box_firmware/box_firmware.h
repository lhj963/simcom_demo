#ifndef __BOX_FIRMWARE_H__
#define __BOX_FIRMWARE_H__

#ifdef __cplusplus
extern "C" {
#endif


extern unsigned int box_firmware_pack(void *p_mcu_dat,unsigned int mcu_dat_len,
										void *p_mpu_dat,unsigned int mpu_dat_len,
										void *p_cfg_dat,unsigned int cfg_dat_len,
										unsigned char libcnt,
										void **p_lib_dat,unsigned int *lib_dat_len,
										void **lib_name,
										void *pOut,unsigned int outbuflen);


extern int box_firmware_unpack(void *pdat,unsigned int datlen,
										void (*mcu_dat_callback)(void *pdata,unsigned int datalen),
										void (*mpu_dat_callback)(void *pdata,unsigned int datalen),
										void (*cfg_dat_callback)(void *pdata,unsigned int datalen),
										void (*lib_dat_callback)(void *filename,void *pdata,unsigned int datalen)
										);

extern void test_pack_firmware(void);


#ifdef __cplusplus
}
#endif
#endif


