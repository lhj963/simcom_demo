#ifndef __UBUS_CONTEXT_H__
#define __UBUS_CONTEXT_H__
#include <stdbool.h>

unsigned char SimcomUbusInit(void);
void SimcomUbusDeinit(void);
struct ubus_context *SimcomGetUbusCtx(void);
struct ubus_context *SimcomGetUbusIndCtx(void);
struct ubus_context *SimcomGetUbusIndCtx(void);
void SimcombusIndCtxExit();
int Simcom_call_ubusinvoke(struct blob_buf * blob_ptr,const char *server_name,const char *method_name,bool flag,void **getdata,int timeout);
void _reset_blob_buf(struct blob_buf *blob_ptr);
void _free_blob_buf(struct blob_buf *blob_ptr);


#endif
