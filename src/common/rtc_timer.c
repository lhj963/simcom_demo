#include <errno.h>
#include <fcntl.h>
#include <linux/rtc.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/timerfd.h>
#include <time.h>
#include <unistd.h>

#define CLOCK_REALTIME 0

int main(int argc, char *argv[])
{
    time_t t = time(NULL);
    struct tm now = *localtime(&t);
    int runtime = 60 * 60; // 1 hour
    int nsleep = 2 * 60;   // 2 min
    int timerfd = timerfd_create(CLOCK_REALTIME, 0);
    struct itimerspec new_value;

    FILE *fp = fopen("/dev/rtc0", "w");
    if (fp == NULL)
    {
        return -1;
    }

    fprintf(fp, "date: %d-%d-%d %d:%d:%d\n", now.tm_year + 1900, now.tm_mon + 1, now.tm_mday, now.tm_hour, now.tm_min, now.tm_sec);

    if (timerfd < 0)
    {
        fprintf(stderr, "rtc_test exit -1\n");
        fclose(fp);
        return -1;
    }

    new_value.it_interval.tv_sec = nsleep;
    new_value.it_interval.tv_nsec = 0;
    new_value.it_value.tv_sec = nsleep;
    new_value.it_value.tv_nsec = 0;
    if (timerfd_settime(timerfd, 0, &new_value, NULL) != 0)
    {
        fclose(fp);
        close(timerfd);
        fprintf(stderr, "rtc_test exit -2\n");
        return -2;
    }
    while (runtime > 0)
    {
        char c[8] = {0};
        int rc;
        rc = read(timerfd, c, 8);
        if (rc <= 0)
        {
            fprintf(fp, "rtc read fail\n");
            close(timerfd);
            fclose(fp);
            return -1;
        }
        runtime -= nsleep;

        t = time(NULL);
        now = *localtime(&t);
        fprintf(fp, "date: %d-%d-%d %d:%d:%d\n", now.tm_year + 1900, now.tm_mon + 1, now.tm_mday, now.tm_hour, now.tm_min, now.tm_sec);
    }
    fprintf(stderr, "rtc_test exit 0\n");
    fclose(fp);
    close(timerfd);
    return 0;
}
