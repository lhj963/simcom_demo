#ifndef __TBOX_SRC_H__
#define __TBOX_SRC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <semaphore.h>

#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <netdb.h>

#include <sys/select.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <linux/serial.h>

#include <sys/inotify.h>

#include <linux/rtc.h>
#include <dirent.h> 


#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#include "./hal/hal.h"

#include "./drivers/drivers.h"

#include "./apps/apps.h"



#ifdef __cplusplus
}
#endif

#endif

