#include "linux_tools.h"
// #include <zlib.h>

int filter_fn(const struct dirent *ent)
{
    if (ent->d_type != DT_REG)
        return 0;

    return (strncmp(ent->d_name, "can", 3) == 0);
}

int my_alphasort(const struct dirent **__e1,
                 const struct dirent **__e2)
{
    struct dirent *p1 = 0;
    struct dirent *p2 = 0;

    p1 = (struct dirent *)(*__e1);
    p2 = (struct dirent *)(*__e2);
    return strcmp(p2->d_name, p1->d_name);
}

unsigned int list_folder(void *floder_path, void (*p_cb_fun)(void *full_path_filename, void *p_user_arg), void *p_user_arg, unsigned int limit_cnt)
{
#if 1
    struct stat dirstat;
    char filename[256] = {0};
    unsigned int cnt = 0;

    if (floder_path == 0)
    {
        return 0;
    }

    if (stat((char *)floder_path, &dirstat) == -1)
    {
        // DBG_Print(LEVEL2,"cannot access %s\r\n",(char *)floder_path);
        return 0;
    }

    if (dirstat.st_mode & __S_IFDIR)
    {
        int n;
        struct dirent **namelist; // struct dirent * namelist[];

        n = scandir((char *)floder_path, &namelist, 0, my_alphasort);
        if (n < 0)
        {
            // perror("scandir");
            DBG_Print(LEVEL2, "scandir error\r\n");
        }
        else
        {
            while (n--)
            {
                if (strcmp(namelist[n]->d_name, ".") && strcmp(namelist[n]->d_name, ".."))
                {
                    // printf("[scanfile]%s\n", namelist[n]->d_name);
                    snprintf(filename, sizeof(filename) - 1, "%s/%s", (char *)floder_path, namelist[n]->d_name);

                    cnt++;
                    if ((limit_cnt == 0) || (cnt <= limit_cnt))
                    {
                        if (p_cb_fun)
                        {
                            p_cb_fun(filename, p_user_arg);
                        }
                    }
                }
                free(namelist[n]);
            }
            free(namelist);
        }
    }
    return cnt;
#else
    DIR *pdir;
    struct dirent *pDirent = NULL;
    struct stat dirstat;
    char filename[256] = {0};
    unsigned int cnt = 0;

    if (floder_path == 0)
    {
        return 0;
    }

    if (stat((char *)floder_path, &dirstat) == -1)
    {
        // DBG_Print(LEVEL2,"cannot access %s\r\n",(char *)floder_path);
        return 0;
    }

    if (dirstat.st_mode & __S_IFDIR)
    {
        pdir = opendir((char *)floder_path);
        while ((pDirent = readdir(pdir)) != NULL)
        {
            if (strcmp(pDirent->d_name, ".") && strcmp(pDirent->d_name, ".."))
            {
                snprintf(filename, sizeof(filename) - 1, "%s/%s", floder_path, pDirent->d_name);

                if (p_cb_fun)
                {
                    p_cb_fun(filename);
                }
                cnt++;
                if (limit_cnt && (cnt >= limit_cnt))
                {
                    break;
                }
            }
        }
        closedir(pdir);
    }
    return cnt;
#endif
}

unsigned int list_folder_no_sort(void *floder_path, void (*p_cb_fun)(void *full_path_filename, void *p_user_arg), void *p_user_arg, unsigned int limit_cnt)
{
    DIR *pdir;
    struct dirent *pDirent = NULL;
    struct stat dirstat;
    char filename[256] = {0};
    unsigned int cnt = 0;

    if (floder_path == 0)
    {
        return 0;
    }

    if (stat((char *)floder_path, &dirstat) == -1)
    {
        DBG_Print(LEVEL2, "2cannot access %s\r\n", (char *)floder_path);
        return 0;
    }

    if (dirstat.st_mode & __S_IFDIR)
    {
        pdir = opendir((char *)floder_path);
        if (pdir)
        {
            while ((pDirent = readdir(pdir)) != NULL)
            {
                if (strcmp(pDirent->d_name, ".") && strcmp(pDirent->d_name, ".."))
                {
                    cnt++;
                    if ((limit_cnt == 0) || (cnt <= limit_cnt))
                    {
                        if (p_cb_fun)
                        {
                            snprintf(filename, sizeof(filename) - 1, "%s/%s", (char *)floder_path, pDirent->d_name);
                            p_cb_fun(filename, p_user_arg);
                        }
                    }

                    if (limit_cnt && (cnt >= limit_cnt))
                    {
                        break;
                    }
                }
            }
            closedir(pdir);
        }
    }
    return cnt;
}

void del_file(void *filename, void *p_user_arg)
{
    if (filename)
    {
        DBG_Print(LEVEL2, "[del file]%s\r\n", (char *)filename);
        remove((char *)filename);
    }
}

void limit_file_cnt_in_folder(void *folder_path, void (*check_file)(void *filename, void *p_user_arg), unsigned int max_file_cnt)
{
    unsigned int file_cnt = 0;

    // first check the file
    list_folder_no_sort(folder_path, check_file, 0, 0);

    // calc the file total cnt
    file_cnt = list_folder_no_sort(folder_path, 0, 0, 0);

    DBG_Print(LEVEL2, "%s file cnt:%u\r\n", folder_path ? folder_path : " ", file_cnt);
    if (file_cnt > max_file_cnt)
    {
        list_folder(folder_path, del_file, 0, file_cnt - max_file_cnt);
    }
}

unsigned int get_file_cnt_in_folder(void *floder_path)
{
    return list_folder_no_sort(floder_path, 0, 0, 0);
}

uint64_t get_disk_info(void *disk_path, uint64_t *p_out_total_size, uint64_t *p_out_free_size, uint64_t *p_out_avail_size)
{
    struct statfs diskInfo;
    uint64_t total_size = 0;
    uint64_t free_size = 0;
    uint64_t avail_size = 0;

    uint32_t total_size_mb = 0;
    uint32_t free_size_mb = 0;
    uint32_t avail_size_mb = 0;

    if (disk_path == 0)
    {
        return 1;
    }

    statfs((char *)disk_path, &diskInfo);

    total_size = (uint64_t)diskInfo.f_blocks * diskInfo.f_bsize;
    free_size = (uint64_t)diskInfo.f_bfree * diskInfo.f_bsize;
    avail_size = (uint64_t)diskInfo.f_bavail * diskInfo.f_bsize;

    if (p_out_total_size)
    {
        *p_out_total_size = total_size;
    }

    if (p_out_free_size)
    {
        *p_out_free_size = free_size;
    }

    if (p_out_avail_size)
    {
        *p_out_avail_size = avail_size;
    }

    total_size_mb = total_size >> 20;
    free_size_mb = free_size >> 20;
    avail_size_mb = avail_size >> 20;

    // printf("%lldBytes %lldBytes %lldBytes\r\n",total_size,free_size,avail_size);
    DBG_Print(LEVEL2, "total:%uMB\r\n", total_size_mb);
    DBG_Print(LEVEL2, "free:%uMB\r\n", free_size_mb);
    DBG_Print(LEVEL2, "avail:%uMB\r\n", avail_size_mb);

    return 0;
}

unsigned long long get_run_time_ms(void)
{
    unsigned long long run_time_ms = 0;
    struct timespec time1 = {0, 0};

    if (clock_gettime(CLOCK_MONOTONIC, &time1) == 0)
    {
        // printf("CLOCK_MONOTONIC: %lu, %lu\n", time1.tv_sec, time1.tv_nsec);

        run_time_ms = time1.tv_sec * 1000;
        run_time_ms += (time1.tv_nsec / 1000000);
    }
    else
    {
        DBG_Print(LEVEL2, "get CLOCK_MONOTONIC time error\r\n");
    }
    return run_time_ms;
}

void sleep_ms(unsigned int ms)
{
#if 0
	struct timeval tv = {0};

	tv.tv_sec = 0;
	tv.tv_usec = ms * 1000;
	select(0,NULL,NULL,NULL,&tv);
#else
    usleep(ms * 1000);
#endif
}

unsigned char is_dir_exist(void *szpath)
{
    struct stat dirstat;

    if (szpath == 0)
    {
        return 0;
    }

    if (stat((char *)szpath, &dirstat) == -1)
    {
        // DBG_Print(LEVEL2,"cannot access %s\r\n",(char *)szpath);
        return 0;
    }

    return 1;
}

unsigned int gzip(void *psrc, unsigned int srclen, void *pOut, unsigned int outbuflen)
{
    unsigned int rlen = 0;
#if 0
    z_stream z = {0};
    int windowBits = 15;
    int GZIP_ENCODING = 16;

    z.next_in = (unsigned char *)psrc;
    z.avail_in = srclen;
    z.next_out = (unsigned char *)pOut;
    z.avail_out = outbuflen;

    //printf("total %u bytes\n", z.avail_in);
#if 0
#define Z_NO_COMPRESSION 0 无压缩
#define Z_BEST_SPEED 1 速度优先
#define Z_BEST_COMPRESSION 9 压缩比优先
#endif

#if 1
    if (deflateInit2(&z, Z_DEFAULT_COMPRESSION, Z_DEFLATED, windowBits | GZIP_ENCODING, 8, Z_DEFAULT_STRATEGY) != Z_OK)
    {
        return -1;
    }
#else
    /* 使用最高压缩比 */
    if (deflateInit(&z, 6) != Z_OK) {
        printf("deflateInit failed!\n");
        return -1;
    }
#endif

    if (deflate(&z, Z_NO_FLUSH) != Z_OK) {
        printf("deflate Z_NO_FLUSH failed!\n");
        return -1;
    }

    if (deflate(&z, Z_FINISH) != Z_STREAM_END) {
        printf("deflate Z_FINISH failed!\n");
        return -1;
    }

    if (deflateEnd(&z) != Z_OK) {
        printf("deflateEnd failed!\n");
        return -1;
    }

    rlen = outbuflen - z.avail_out;

    printf("[gzip]total:%u after compressed:%uBytes\r\n", srclen,rlen);
#endif
    return rlen;
}

unsigned int exe_system_cmd(void *cmd, void *response_buf, unsigned int response_buf_len)
{
    FILE *fp;
    unsigned int rlen = 0;

    if (response_buf && response_buf_len)
    {
        memset(response_buf, 0, response_buf_len);
    }

    if ((cmd == 0) || (strlen((char *)cmd) == 0))
    {
        return 0;
    }

    fp = popen((char *)cmd, "r");
    if (fp)
    {
        if (response_buf && response_buf_len)
        {
            rlen = fread((char *)response_buf, 1, response_buf_len, fp);
        }
        pclose(fp);
    }
    return rlen;
}
