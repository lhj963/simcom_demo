#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <sys/poll.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <termios.h>
#include <linux/input.h>
#include <linux/ioctl.h>
#include <pthread.h>
#include "GpioControl.h"
#include "LedControl.h"

#define GPIO_SEELP_MODEM_MS 200

#define MAX_BUF 128

#define PIN62_LOCK system_power_lock(GPIO6_LOCKNAME)
#define PIN62_UNLOCK system_power_unlock(GPIO6_LOCKNAME)

void system_power_lock(const char *lock_id)
{
	int fd;

	fd = open("/sys/power/wake_lock", O_WRONLY);
	if (fd < 0)
	{
		DBG_Print(LEVEL2, "wake_lock,error %d\n", fd);
		return;
	}

	write(fd, lock_id, strlen(lock_id));

	close(fd);
}
void system_power_unlock(const char *lock_id)
{
	int fd;
	fd = open("/sys/power/wake_unlock", O_WRONLY);
	if (fd < 0)
	{
		DBG_Print(LEVEL2, "wake_unlock,error %d\n", fd);
		return;
	}

	write(fd, lock_id, strlen(lock_id));

	close(fd);
}

int gpio_file_create(int gpio)
{
	int sfd = -1;
	char checkstr[50] = {0};
	char configstr[10] = {0};
	int reto, len;

	sprintf(checkstr, "/sys/class/gpio/gpio%d/value", gpio);
	if (0 == access(checkstr, F_OK))
	{
		return 0;
	}

	sfd = open("/sys/class/gpio/export", O_WRONLY);
	if (sfd < 0)
	{
		DBG_Print(LEVEL2, "%s:%d,open file error,%d\n", __FUNCTION__, __LINE__, sfd);
		return sfd;
	}

	len = sprintf(configstr, "%d", gpio);
	reto = write(sfd, configstr, len);

	if (reto != len)
	{
		DBG_Print(LEVEL2, "create gpio(%d) files:%d,%d,%d\n", gpio, __LINE__, len, reto);
		close(sfd);
		return reto;
	}
	usleep(1000);
	reto = access(checkstr, F_OK);
	if (0 > reto)
	{
		DBG_Print(LEVEL2, "%s:%d,gpio file(%s)not exist\n", __FUNCTION__, __LINE__, checkstr);
	}

	close(sfd);
	return reto;
}

int gpio_file_remove(int gpio)
{
	int sfd = -1;
	char checkstr[50] = {0};
	char configstr[10] = {0};
	int reto, len;

	sprintf(checkstr, "/sys/class/gpio/gpio%d/value", gpio);
	if (0 > access(checkstr, F_OK))
	{
		return 0;
	}

	sfd = open("/sys/class/gpio/unexport", O_WRONLY);

	if (sfd < 0)
	{
		DBG_Print(LEVEL2, "%s:%d,open file error,%d\n", __FUNCTION__, __LINE__, sfd);
		return sfd;
	}

	len = sprintf(configstr, "%d", gpio);
	reto = write(sfd, configstr, len);

	if (reto != len)
	{
		DBG_Print(LEVEL2, "%s:%d,%d,%d\n", __FUNCTION__, __LINE__, len, reto);
		close(sfd);
		return reto;
	}

	usleep(1000);
	reto = 0;
	if (0 == access(checkstr, F_OK))
	{
		DBG_Print(LEVEL2, "%s:%d,gpio file(%s)still exist\n", __FUNCTION__, __LINE__, checkstr);
		reto = -1;
	}

	close(sfd);
	return reto;
}

int gpio_set_dir(unsigned int gpio, unsigned int out_flag)
{
	int fd;
	char buf[MAX_BUF];

	snprintf(buf, sizeof(buf), "/sys/class/gpio/gpio%d/direction", gpio);

	fd = open(buf, O_WRONLY);
	if (fd < 0)
	{
		DBG_Print(LEVEL2, "gpio(%d)/direction\n", gpio);
		return fd;
	}

	if (out_flag)
		write(fd, "out", 4);
	else
		write(fd, "in", 3);

	close(fd);
	return 0;
}

int gpio_get_dir(unsigned int gpio)
{
	int fd;
	int count;
	char buff[MAX_BUF];
	snprintf(buff, sizeof(buff), "/sys/class/gpio/gpio%d/direction", gpio);

	fd = open(buff, O_RDONLY);
	if (fd < 0)
	{
		DBG_Print(LEVEL2, "open gpio%d/direction faild\n", gpio);
		return -1;
	}
	memset(buff, 0, MAX_BUF);
	count = read(fd, buff, MAX_BUF);
	if (count <= 0)
	{
		DBG_Print(LEVEL2, "read faild");
		close(fd);
		return -1;
	}
	buff[strlen(buff) - 1] = 0;
	if (strncmp(buff, "in", 3) == 0)
	{
		close(fd);
		return 0;
	}
	else
	{
		close(fd);
		return 1;
	}
}

int gpio_set_value(unsigned int gpio, unsigned int value)
{
	int fd;
	char buf[MAX_BUF];

	snprintf(buf, sizeof(buf), "/sys/class/gpio/gpio%d/value", gpio);

	fd = open(buf, O_WRONLY);
	if (fd < 0)
	{
		perror("gpio/set-value\n");
		return fd;
	}

	if (value)
		write(fd, "1", 2);
	else
		write(fd, "0", 2);

	close(fd);
	return 0;
}

int gpio_get_value(unsigned int gpio)
{
	int fd;
	int count;
	char buff[MAX_BUF];
	snprintf(buff, sizeof(buff), "/sys/class/gpio/gpio%d/value", gpio);

	fd = open(buff, O_RDONLY);
	if (fd < 0)
	{
		DBG_Print(LEVEL2, "open gpio%d/value faild\n", gpio);
		return -1;
	}
	memset(buff, 0, MAX_BUF);
	count = read(fd, buff, MAX_BUF);
	if (count <= 0)
	{
		DBG_Print(LEVEL2, "read faild");
		close(fd);
		return -1;
	}
	buff[strlen(buff) - 1] = 0;
	if (strncmp(buff, "0", 2) == 0)
	{
		close(fd);
		return 0;
	}
	else
	{
		close(fd);
		return 1;
	}
}

int gpio_set_isr(unsigned gpio, gpio_edge_type edge)
{
	char fvalue_path[MAX_BUF];
	int fd = -1, ret = -1;
	if (edge < 0 || edge > 3)
	{
		DBG_Print(LEVEL2, "please input correct edge mode\n");
		return -1;
	}
	sprintf(fvalue_path, "/sys/class/gpio/gpio%d/edge", gpio);
	fd = open(fvalue_path, O_RDWR);
	if (fd < 0)
	{
		DBG_Print(LEVEL2, "gpio_set_isr write %s error %d\n", fvalue_path, ret);
		return fd;
	}

	switch (edge)
	{
	case GPIO_IRQ_NONE:
	{
		if ((ret = write(fd, "none", 5)) < 0)
		{
			close(fd);
			DBG_Print(LEVEL2, "gpio_set_isr write %s error,%d\n", fvalue_path, ret);
			return -1;
		}
	}
	break;

	case GPIO_IRQ_RISING:
	{
		if ((ret = write(fd, "rising", 7)) < 0)
		{
			close(fd);
			DBG_Print(LEVEL2, "gpio_set_isr write %s error,%d\n", fvalue_path, ret);
			return -1;
		}
	}
	break;

	case GPIO_IRQ_FALLING:
	{
		if ((ret = write(fd, "falling", 8)) < 0)
		{
			close(fd);
			DBG_Print(LEVEL2, "gpio_set_isr write %s error,%d\n", fvalue_path, ret);
			return -1;
		}
	}
	break;

	case GPIO_IRQ_BOTH:
	{
		if ((ret = write(fd, "both", 5)) < 0)
		{
			close(fd);
			DBG_Print(LEVEL2, "gpio_set_isr write %s error,%d\n", fvalue_path, ret);
			return -1;
		}
	}
	break;
	}
	close(fd);

	return 0;
}
#define MODEM_WAKEUP_MCU_TIME 100
int modem_ri_notify_mcu(void)
{
	PIN62_LOCK;
	autosleep_enable(0);
	gpio_set_value(MODEM_WAKEUP_MCU_GPIO, 1);
	poll(0, 0, MODEM_WAKEUP_MCU_TIME);
	gpio_set_value(MODEM_WAKEUP_MCU_GPIO, 0);
	PIN62_UNLOCK;
	DBG_Print(LEVEL2, "modem_ri_notify_mcu\n");
}
int gpio_can_wakeup(unsigned int gpio, unsigned int value)
{
	int fd;
	char buf[MAX_BUF];

	snprintf(buf, sizeof(buf), "/sys/class/gpio/gpio%d/can_wakeup", gpio);

	fd = open(buf, O_WRONLY);
	if (fd < 0)
	{
		perror("gpio/set-value");
		return fd;
	}

	if (value)
		write(fd, "1", 2);
	else
		write(fd, "0", 2);

	close(fd);
	return 0;
}

int gpio_init()
{
	int ret;
	ret = gpio_file_create(MODEM_WAKEUP_MCU_GPIO);
	ret += gpio_set_dir(MODEM_WAKEUP_MCU_GPIO, 1);
	ret += gpio_set_value(MODEM_WAKEUP_MCU_GPIO, 0);
	return ret;
}

void gpio_dtr_deinit(void)
{
	// if (gpio_file_remove(MCU_SLEEP_WAKEUP_MODEM_GPIO) != 0)
	// DBG_Print(LEVEL2, "gpio DTR Deinit error\n");
}