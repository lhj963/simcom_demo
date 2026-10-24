#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include "simcom_sdk_common.h"


#define SSPCONTROL_SSP_BUS "/dev/spidev1.0"

typedef enum _SSP_MODE_E
{
    SSP_MODE_0 = 0,
    SSP_MODE_1,
    SSP_MODE_2,
    SSP_MODE_3,
}
SSP_MODE_E;

typedef struct _SSPCONTROL_MODE_S
{
    SSP_MODE_E emode : 2;
    uint8_t isCsHigh : 1;
    uint8_t isLsb : 1;
    uint8_t is3wire : 1;
    uint8_t isLoop : 1;
    uint8_t isNoCs : 1;
    uint8_t isReady : 1;
}
SSPCONTROL_MODE_S;

typedef union _SSPCONTROL_MODE_U
{
    uint8_t byte;
    SSPCONTROL_MODE_S smode;
}
SSPCONTROL_MODE_U;

typedef struct _SSPCONTROL_S
{
    SSPCONTROL_MODE_U umode;
    uint8_t bits;
    uint32_t speed;
    uint16_t delay;
}
SSPCONTROL_S;

static inline int sim_ssp_open(void)
{
    return open(SSPCONTROL_SSP_BUS, O_RDWR);
}

static inline int sim_ssp_close(int fd)
{
    return close(fd);
}

static int sim_ssp_setMode(int fd, SSPCONTROL_S *ctl)
{
    int ret;

    if ((fd <= 0) || !ctl)
    {
        return -1;
    }

    if (-1 ==ioctl(fd, SPI_IOC_WR_MODE, &ctl->umode.byte))
    {
        return -2;
    }
    if (-1 ==ioctl(fd, SPI_IOC_RD_MODE, &ctl->umode.byte))
    {
        return -2;
    }

    if (-1 == ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &ctl->bits))
    {
        return -2;
    }
    if (-1 == ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &ctl->bits))
    {
        return -2;
    }

    if (-1 == ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &ctl->speed))
    {
        return -2;
    }
    if (-1 == ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &ctl->speed))
    {
        return -2;
    }

    return 0;
}

static int sim_ssp_transfer(int fd, SSPCONTROL_S *ctl, uint8_t *send, uint8_t *recv, uint32_t len)
{
    if ((fd <= 0) || !ctl || !send || !recv || !len)
    {
        return -1;
    }

    struct spi_ioc_transfer trans =
    {
        .tx_buf = (unsigned long)send,
        .rx_buf = (unsigned long)recv,
        .len = len,
        .delay_usecs = ctl->delay,
        .speed_hz = ctl->speed,
        .bits_per_word = ctl->bits,
    };

    if (ioctl(fd, SPI_IOC_MESSAGE(1), &trans) < 1)
    {
        SDK_ERR("sim_ssp ioctl fail \r\n");
        return -2;
    }

    return 0;
}

int sim_ssp_bytes(SSPCONTROL_S *ctl, uint8_t *send, uint8_t *recv, uint32_t len)
{
    int fd, ret;

    if (!ctl || !send || !recv || !len)
    {
        return -1;
    }

    fd = sim_ssp_open();
    if (fd < 0)
    {
        SDK_ERR("sim_ssp_open fail! please check spidev1.0 dts\r\n");
        return -2;
    }

    ret = sim_ssp_setMode(fd, ctl);
    if (ret)
    {
        SDK_ERR("sim_ssp_setMode fail \r\n");
        sim_ssp_close(fd);
        return ret;
    }

    ret = sim_ssp_transfer(fd, ctl, send, recv, len);
    SDK_INFO("sim_ssp_transfer ok \r\n");
    sim_ssp_close(fd);
    return ret;
}

