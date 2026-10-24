#include "ADC.h"
#define ADC "/sys/class/88pm80xadc/88pm80xadc/adc"
#define VBAT "/sys/class/88pm80xadc/88pm80xadc/vbat"
#ifndef READ_SUCCESS
#define READ_SUCCESS 0
#endif
#ifndef READ_FAILD
#define READ_FAILD -1
#endif
#ifndef OPENFILE_FAILD
#define OPENFILE_FAILD -2
#endif


int get_adc_voltage(int *voltage)
{
    int fd = -1;
    int ret = -1;
    char buf[128] = {0};

    fd = open(ADC, O_RDONLY);
    if (fd < 0)
    {
        printf("open file error.\n");
        return  OPENFILE_FAILD;
    }
    ret = read(fd, buf, sizeof(buf));
    if (ret < 0)
    {
        printf("read file error.\n");
        close(fd);
        return READ_FAILD;
    }
    *voltage = atoi(buf);
    close(fd);
    return READ_SUCCESS;
}

int get_vbat_voltage(int *voltage)
{
    int fd = -1;
    int ret = -1;
    char buf[128] = {0};

    fd = open(VBAT, O_RDONLY );
    if (fd < 0)
    {
        printf("open file error.\n");
        return  OPENFILE_FAILD;
    }
    ret = read(fd, buf, sizeof(buf));
    if (ret < 0)
    {
        printf("read file error.\n");
        close(fd);
        return READ_FAILD;
    }
    *voltage = atoi(buf);
    close(fd);
    return READ_SUCCESS;
}


