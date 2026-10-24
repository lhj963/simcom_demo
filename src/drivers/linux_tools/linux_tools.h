#ifndef __LINUX_TOOLS_H__
#define __LINUX_TOOLS_H__

#ifdef __cplusplus
extern "C" {
#endif

extern unsigned int list_folder(void *floder_path,void (*p_cb_fun)(void *full_path_filename,void *p_user_arg),void *p_user_arg,unsigned int limit_cnt);

extern unsigned int list_folder_no_sort(void *floder_path,void (*p_cb_fun)(void *full_path_filename,void *p_user_arg),void *p_user_arg,unsigned int limit_cnt);

extern void limit_file_cnt_in_folder(void *folder_path,void (*check_file)(void *filename,void *p_user_arg),unsigned int max_file_cnt);

extern unsigned int get_file_cnt_in_folder(void *floder_path);

extern uint64_t get_disk_info(void *disk_path,uint64_t *p_out_total_size,uint64_t *p_out_free_size,uint64_t *p_out_avail_size);

extern unsigned long long get_run_time_ms(void);

extern unsigned char is_dir_exist(void *szpath);

extern unsigned int gzip(void *psrc,unsigned int srclen,void *pOut,unsigned int outbuflen);

extern unsigned int exe_system_cmd(void *cmd,void *response_buf,unsigned int response_buf_len);

#ifdef __cplusplus
}
#endif

#endif

