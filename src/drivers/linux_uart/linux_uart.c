#include "linux_uart.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>

#define MAX_CHANNEL_CNT 8

typedef struct tagUartStruction
{
	int fd;
	unsigned int (*uart_data_arrive_cb)(void *, unsigned int);
	pthread_mutex_t lock;

} UartStruction;

UartStruction s_astUartStruction[MAX_CHANNEL_CNT] = {0};

typedef void (*uart_recv_t)(void);
uart_recv_t p_uart_recv_thread = 0;

void reg_uart_recv_thread_callback(void (*pfunc)(void))
{
	p_uart_recv_thread = pfunc;
}

int uart_send_bytes(unsigned char channel, void *buffer, unsigned int length)
{
	int nBytes = 0;

	if ((s_astUartStruction[channel].fd == 0) || (s_astUartStruction[channel].fd == -1))
	{
		return 0;
	}

	if ((buffer == 0) || (length == 0))
	{
		return 0;
	}

	pthread_mutex_lock(&(s_astUartStruction[channel].lock));
	nBytes = write(s_astUartStruction[channel].fd, (char *)buffer, length);
	pthread_mutex_unlock(&(s_astUartStruction[channel].lock));

	return nBytes;
}

#define MAX_UART_RECV_MCU_DATA_BUF_LEN (1024 * 20)

void myprint_hex(void *pdat, unsigned int datlen)
{
	if (pdat && datlen)
	{
		unsigned int i = 0;

		for (i = 0; i < datlen; i++)
		{
			printf("%02X", *((unsigned char *)pdat + i));
		}
	}
	printf("\r\n");
}

void *uart_recv_proc(void *arg)
{
	struct timeval tv = {0};
	static char buff[MAX_UART_RECV_MCU_DATA_BUF_LEN + 256] = {0};
	int fd = 0;
	fd_set rset;
	int status = 0;
	int ret = 0;
	int rlen = 0;
	unsigned long long no_uart_data_cnt = 0;
	unsigned char channel = (unsigned int)arg;

	fd = s_astUartStruction[channel].fd;
	while (1)
	{
		FD_ZERO(&rset);
		FD_SET(fd, &rset);
		tv.tv_sec = 0;
		tv.tv_usec = 150000;

		status = select(fd + 1, &rset, NULL, NULL, &tv);
		if (status > 0)
		{
			if (FD_ISSET(fd, &rset))
			{
				rlen = read(fd, buff + ret, MAX_UART_RECV_MCU_DATA_BUF_LEN - ret);
				// printf("uart recv:");
				// myprint_hex(buff + ret,rlen);
				if (rlen > 0)
				{
					ret += rlen;
					no_uart_data_cnt = 0;
				}
				else
				{
					no_uart_data_cnt++;
				}
			}
			else
			{
				no_uart_data_cnt++;
			}
		}
		else
		{
			no_uart_data_cnt++;
		}

		if ((no_uart_data_cnt >= 1) || (ret >= MAX_UART_RECV_MCU_DATA_BUF_LEN))
		{
			no_uart_data_cnt = 0;
			if (ret > 0)
			{
				if (s_astUartStruction[channel].uart_data_arrive_cb)
				{
					ret = s_astUartStruction[channel].uart_data_arrive_cb(buff, ret);
					// printf("remain:");
					// myprint_hex(buff,ret);
				}
				else
				{
					ret = 0;
				}
			}
		}

		if (p_uart_recv_thread)
			p_uart_recv_thread();
	}

	return NULL;
}

/*******************************************************************************
	Function  Definition
*******************************************************************************/
int _uart_set_speed(int fd, int speed)
{
	struct termios tio;

	if (0 != tcgetattr(fd, &tio))
	{
		printf("autolink_uart_set_speed : tcgetattr call failed\n");
		return -1;
	}

	tcflush(fd, TCIOFLUSH);

	switch (speed)
	{
	case 115200:
		cfsetispeed(&tio, B115200);
		cfsetospeed(&tio, B115200);
		break;
	case 57600:
		cfsetispeed(&tio, B57600);
		cfsetospeed(&tio, B57600);
		break;
	case 38400:
		cfsetispeed(&tio, B38400);
		cfsetospeed(&tio, B38400);
		break;
	case 9600:
		cfsetispeed(&tio, B9600);
		cfsetospeed(&tio, B9600);
		break;
	case 921600:
		cfsetispeed(&tio, B921600);
		cfsetospeed(&tio, B921600);
		break;
	case 1000000:
		cfsetispeed(&tio, B1000000);
		cfsetospeed(&tio, B1000000);
		break;
		break;
	default:
		cfsetispeed(&tio, B19200);
		cfsetospeed(&tio, B19200);
	}

	if (0 != tcsetattr(fd, TCSANOW, &tio))
	{
		printf("autolink_uart_set_speed : tcsetattr call failed\n");
		return -1;
	}

	tcflush(fd, TCIOFLUSH);

	return 0;
}

/*******************************************************************************
	Function  Definition
*******************************************************************************/
int _uart_set_parity(int fd, int databits, int stopbits, int parity)
{
	struct termios tio;

	if (0 != tcgetattr(fd, &tio))
	{
		printf("autolink_uart_set_parity : tcgetattr call failed\n");
		return -1;
	}

	tio.c_cflag &= ~CSIZE;

	switch (databits)
	{
	case 7:
		tio.c_cflag |= CS7;
		break;
	case 8:
		tio.c_cflag |= CS8;
		break;
	default:
		printf("autolink_uart_set_parity : unsupported data size\n");
		return -1;
	}

	switch (parity)
	{
	case 'n':
	case 'N':
		tio.c_cflag &= ~PARENB;
		tio.c_iflag &= ~INPCK;
		break;
	case 'o':
	case 'O':
		tio.c_cflag |= (PARODD | PARENB);
		tio.c_iflag |= INPCK;
		break;
	case 'e':
	case 'E':
		tio.c_cflag |= PARENB;
		tio.c_cflag &= ~PARODD;
		tio.c_iflag |= INPCK;
		break;
	case 'S':
	case 's':
		tio.c_cflag &= ~PARENB;
		tio.c_cflag &= ~CSTOPB;
		break;
	default:
		printf("autolink_uart_set_parity : unsupported parity\n");
		return -1;
	}

	switch (stopbits)
	{
	case 1:
		tio.c_cflag &= ~CSTOPB;
		break;
	case 2:
		tio.c_cflag |= CSTOPB;
		break;
	default:
		printf("autolink_uart_set_parity : unsupported stop bits\n");
		return -1;
	}

	// Raw Mode
	tio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	tio.c_oflag &= ~OPOST;
	tio.c_cflag |= (CLOCAL | CREAD);
	tio.c_iflag &= ~(ICRNL | PARMRK | IGNCR | IUCLC);
	tio.c_iflag &= ~(IXON | IXOFF | IXANY);

	/* Set input parity option */
	if (parity != 'n')
	{
		tio.c_iflag |= INPCK;
	}

	tcflush(fd, TCIOFLUSH);

	tio.c_cc[VTIME] = 0;
	tio.c_cc[VMIN] = 0;

	if (0 != tcsetattr(fd, TCSANOW, &tio))
	{
		printf("autolink_uart_set_parity : tcsetattr call failed\n");
		return -1;
	}

	return 0;
}

/*******************************************************************************
	Function  Definition
*******************************************************************************/

int open_uart(unsigned char channel, void *dev_path, unsigned int bps, unsigned char databits, unsigned char stopbits, char iParity,
			  unsigned int (*uart_data_arrive)(void *, unsigned int))
{
	// int iter = 0;
	int baud = bps;
	pthread_t ntid = 0;
	int fd = 0;

	if (channel >= MAX_CHANNEL_CNT)
	{
		return 1;
	}

	if (dev_path == 0)
	{
		return 2;
	}

	//"/dev/ttyHSL1"
	fd = open((char *)dev_path, O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd < 0)
	{
		DBG_Print(LEVEL0, "%s open false\r\n", dev_path);
		return 3;
	}

	DBG_Print(LEVEL2, "dev:%s bps:%u\r\n", (char *)dev_path, baud);
	if (0 != _uart_set_speed(fd, baud))
	{
		DBG_Print(LEVEL0, "set speed error\r\n");
		return 4;
	}

	if (0 != _uart_set_parity(fd, 8, 1, 'N'))
	{
		DBG_Print(LEVEL0, "set parity error.\r\n");
		return 5;
	}

	s_astUartStruction[channel].fd = fd;
	s_astUartStruction[channel].uart_data_arrive_cb = uart_data_arrive;

	//  none blocking
	//  fcntl(fd, F_SETFL, FNDELAY);

	pthread_create(&ntid, NULL, uart_recv_proc, (void *)((unsigned int)channel));
	return 0;
}
