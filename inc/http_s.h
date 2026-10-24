#ifndef HTTP_S_H
#define HTTP_S_H
#include <stdio.h>


int http_get(char *url, char *rename);
int https_get(char *url, char *rename, int ca_flag, char *cainfo);
int https_get_binary(char *url, char *rename, char *cainfo, char *client_cert_file, char *client_key_file);

#endif

