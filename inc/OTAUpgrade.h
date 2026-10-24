#ifndef _OAT_UPGRADE_H_
#define _OAT_UPGRADE_H_

#include <sys/time.h>
#include <sys/select.h>
#include <time.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <syscall.h>
#include <sys/ioctl.h>
#include <linux/reboot.h>
#include <libubox/blob.h>
#include <libubox/blobmsg.h>
#include <libubox/ustream.h>
#include <libubus.h>

typedef enum {
    UPDATE_STATE_UPDATING,
    UPDATE_STATE_UPDATED,
    UPDATE_STATE_FAILED,
    UPDATE_STATE_IDLE,
}UPDATE_STATE;
    
/*****************************************************************************
* Function Name : upgrade_through_sd
* Description   : SD方式OTA升级
* Input         : url : The path of fota package
* Output        : None
* Return        : 0: success     其他: fail
* Auther        : zhurui
* Date          : 2022.01.11
*****************************************************************************/
int upgrade_through_sd(char *url);

/*****************************************************************************
* Function Name : upgrade_through_http
* Description   : http方式OTA升级
* Input         : url : The url of fota package
* Output        : None
* Return        : 0: success     -1: fail
* Auther        : zhurui
* Date          : 2022.06.29
*****************************************************************************/
int upgrade_through_http(char *url);

/*****************************************************************************
* Function Name : query_update_state
* Description   : Query update state
* Input         : None
* Output        : UPDATE_STATE
* Return        : -1: fail
* Auther        : zhurui
* Date          : 2022.06.17
*****************************************************************************/
int query_update_state(UPDATE_STATE *state);
#endif
