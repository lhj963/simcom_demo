#include <stdio.h>
#include <stdlib.h>

#include <openssl/ssl.h>
#include <string.h>
#include <unistd.h>


#include "simcom_common.h"

#ifdef FEATURE_DEMO_MQTTS
#include <mosquitto.h>
/*
#define HOST "test.mosquitto.org" //test server
#define PORT 1883 //for mqtt
#define PORTS 8884 //for mqtts
*/
#define KEEP_ALIVE 60
#define MSG_MAX_SIZE 512

bool session = true;

//CALLBACK
void my_message_callback(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message)
{
    if (message->payloadlen)
    {
        printf("%s %s\n", message->topic, (char *)message->payload);
    }
    else
    {
        printf("%s (null)\n", message->topic);
    }
    fflush(stdout);
}

void my_connect_callback(struct mosquitto *mosq, void *userdata, int result)
{
    if (!result)
    {
        /* Subscribe to broker information topics on successful connect. */
        mosquitto_subscribe(mosq, NULL, "SIMCOM:", 2);
    }
    else
    {
        fprintf(stderr, "Connect failed\n");
    }
}

void my_subscribe_callback(struct mosquitto *mosq, void *userdata, int mid, int qos_count, const int *granted_qos)
{
    int i;
    printf("Subscribed (mid: %d): %d", mid, granted_qos[0]);
    for (i = 1; i < qos_count; i++)
    {
        printf(", %d", granted_qos[i]);
    }
    printf("\n");
}

void my_log_callback(struct mosquitto *mosq, void *userdata, int level, const char *str)
{
    /* Pring all log messages regardless of level. */
    printf("%s\n", str);
}

//MQTT_SUB
int mqtt_sub(char *host, int port)
{
    struct mosquitto *mosq = NULL;
    int ret;

    if (NULL==host)
    {
        printf("Enter a valid server domain name or IP address!\n");
        return -1;
    }

    //libmosquitto 库初始化
    mosquitto_lib_init();
    //创建mosquitto客户端
    mosq = mosquitto_new(NULL, session, NULL);
    if (!mosq)
    {
        printf("create client failed..\n");
        mosquitto_lib_cleanup();
        return -1;
    }
    //设置回调函数，需要时可使用
    //mosquitto_log_callback_set(mosq, my_log_callback);
    mosquitto_connect_callback_set(mosq, my_connect_callback);
    mosquitto_message_callback_set(mosq, my_message_callback);
    //mosquitto_subscribe_callback_set(mosq, my_subscribe_callback);
    //客户端连接服务器
    if (mosquitto_connect(mosq, host, port, KEEP_ALIVE))
    {
        fprintf(stderr, "Unable to connect.\n");
        return -1;
    }
    printf("Connect server ok\n");
    //循环处理网络消息
    ret = mosquitto_loop_forever(mosq, -1, 1);
    printf("mosquitto_loop_forever ret = %d\n", ret);

    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();

    return 0;
}

//MQTT_PUB
int mqtt_pub(char *host, int port)
{
    char buff[MSG_MAX_SIZE + 1];
    struct mosquitto *mosq = NULL;

    if (NULL==host)
    {
        printf("Enter a valid server domain name or IP address!\n");
        return -1;
    }

    //libmosquitto 库初始化
    mosquitto_lib_init();
    //创建mosquitto客户端
    mosq = mosquitto_new(NULL, session, NULL);
    if (!mosq)
    {
        printf("create client failed..\n");
        mosquitto_lib_cleanup();
        return -1;
    }
    //连接服务器
    if (mosquitto_connect(mosq, host, port, KEEP_ALIVE))
    {
        fprintf(stderr, "Unable to connect.\n");
        return -1;
    }
    printf("Connect server ok\n");
    printf("Now please publish information to server.\n");
    //开启一个线程，在线程里不停的调用 mosquitto_loop() 来处理网络信息
    int loop = mosquitto_loop_start(mosq);
    if (loop != MOSQ_ERR_SUCCESS)
    {
        printf("mosquitto loop error\n");
        return -1;
    }
    while (fgets(buff, MSG_MAX_SIZE + 1, stdin) != NULL)
    {
        if (buff[0] == '\n' && strlen(buff) == 1)
        {
            continue;
        }
        else if (buff[strlen(buff) - 1] == '\n')
        {
            buff[strlen(buff) - 1] = 0;
        }
        /*发布消息*/
        mosquitto_publish(mosq, NULL, "SIMCOM:", strlen(buff) + 1, buff, 0, 0);
        memset(buff, 0, sizeof(buff));
    }
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();
    return 0;
}

//MQTTS_SUB
int mqtts_sub(char *host, int port, char *cafile, char *certfile, char *keyfile)
{
    struct mosquitto *mosq = NULL;
    int ret;

    if (NULL == host || NULL == cafile || NULL== certfile || NULL == keyfile)
    {
        printf("parameter error!\n");
        return -1;
    }

    if (access(cafile, F_OK) != 0)
    {
        printf("%s does not exist!\n", cafile);
        return -1;
    }

    if (access(certfile, F_OK) != 0)
    {
        printf("%s does not exist!\n", certfile);
        return -1;
    }

    if (access(keyfile, F_OK) != 0)
    {
        printf("%s does not exist!\n", keyfile);
        return -1;
    }

    //libmosquitto 库初始化
    mosquitto_lib_init();
    //创建mosquitto客户端
    mosq = mosquitto_new(NULL, session, NULL);
    if (!mosq)
    {
        printf("create client failed..\n");
        mosquitto_lib_cleanup();
        return -1;
    }
    
    //设置回调函数，需要时可使用
    //开启后会打印调试log
    //mosquitto_log_callback_set(mosq, my_log_callback);
    mosquitto_connect_callback_set(mosq, my_connect_callback);
    mosquitto_message_callback_set(mosq, my_message_callback);
    //mosquitto_subscribe_callback_set(mosq, my_subscribe_callback);

    ret = mosquitto_tls_set(mosq, cafile, NULL, certfile, keyfile, NULL);
    if (ret == 0)
    {
        printf("mosquitto_tls_set successful!\n");
    }
    else
    {
        printf("mosquitto_tls_set failed! error code: %d\n", ret);
        return -1;
    }

    //客户端连接服务器
    if (mosquitto_connect(mosq, host, port, KEEP_ALIVE))
    {
        fprintf(stderr, "Unable to connect.\n");
        return -1;
    }
    printf("Connect server ok\n");
    //循环处理网络消息
    ret = mosquitto_loop_forever(mosq, -1, 1);
    printf("mosquitto_loop_forever ret = %d\n", ret);
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();

    return 0;
}

//MQTTS_PUB
int mqtts_pub(char *host, int port, char *cafile, char *certfile, char *keyfile)
{
    char buff[MSG_MAX_SIZE + 1];
    struct mosquitto *mosq = NULL;
    int ret;

    if (NULL == host || NULL== cafile || NULL== certfile || NULL== keyfile)
    {
        printf("parameter error!\n");
        return -1;
    }

    if (access(cafile, F_OK) != 0)
    {
        printf("%s does not exist!\n", cafile);
        return -1;
    }

    if (access(certfile, F_OK) != 0)
    {
        printf("%s does not exist!\n", certfile);
        return -1;
    }

    if (access(keyfile, F_OK) != 0)
    {
        printf("%s does not exist!\n", keyfile);
        return -1;
    }

    //libmosquitto 库初始化
    mosquitto_lib_init();
    //创建mosquitto客户端
    mosq = mosquitto_new(NULL, session, NULL);
    if (!mosq)
    {
        printf("create client failed..\n");
        mosquitto_lib_cleanup();
        return -1;
    }
    //开启后会打印调试log
    //mosquitto_log_callback_set(mosq, my_log_callback);
    ret = mosquitto_tls_set(mosq, cafile, NULL, certfile, keyfile, NULL);
    if (ret == 0)
    {
        printf("mosquitto_tls_set successful!\n");
    }
    else
    {
        printf("mosquitto_tls_set failed! error code: %d\n", ret);
        return -1;
    }

    //连接服务器
    if (mosquitto_connect(mosq, host, port, KEEP_ALIVE))
    {
        fprintf(stderr, "Unable to connect.\n");
        return -1;
    }
    printf("Connect server ok\n");
    printf("Now please publish information to server.\n");
    //开启一个线程，在线程里不停的调用 mosquitto_loop() 来处理网络信息
    int loop = mosquitto_loop_start(mosq);
    if (loop != MOSQ_ERR_SUCCESS)
    {
        printf("mosquitto loop error\n");
        return -1;
    }
    while (fgets(buff, MSG_MAX_SIZE + 1, stdin) != NULL)
    {
        if (buff[0] == '\n' && strlen(buff) == 1)
        {
            continue;
        }
        else if (buff[strlen(buff) - 1] == '\n')
        {
            buff[strlen(buff) - 1] = 0;
        }
        /*发布消息*/
        mosquitto_publish(mosq, NULL, "SIMCOM:", strlen(buff) + 1, buff, 0, 0);
        memset(buff, 0, sizeof(buff));
    }
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();
    return 0;
}
#endif
