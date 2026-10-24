#ifndef _SC_SDK_LOG_H
#define _SC_SDK_LOG_H

#define SC_LOG_NONE_LEVEL                  0     /* no log printed at all */
#define SC_LOG_ERR_LEVEL                   1     /* current app-module error */
#define SC_LOG_WARNING_LEVEL               2     /* using default parameters */
#define SC_LOG_INFO_LEVEL                  3     /* running messages */
#define SC_LOG_DEBUG_LEVEL                 4     /* debugging messages */


#ifdef CONFIG_DIAG_LOG_ENABLE
#include <include/log.h>

#define SC_DIAG_LOG(LEVEL, format, args ...) \
{ \
    sc_log_init(); \
    switch(LEVEL) \
    { \
        case SC_LOG_ERR_LEVEL: \
        { \
            RERRMSG(format, ##args); \
            break; \
        } \
        case SC_LOG_WARNING_LEVEL: \
        { \
            RWARNMSG(format, ##args); \
            break; \
        } \
        case SC_LOG_INFO_LEVEL: \
        { \
            RINFOMSG(format, ##args); \
            break; \
        } \
        case SC_LOG_DEBUG_LEVEL: \
        { \
            RDBGMSG(format, ##args); \
            break; \
        } \
        default: \
        { \
            break; \
        } \
    } \
}

#define SDK_ERR(format, args...)    SC_DIAG_LOG(SC_LOG_ERR_LEVEL, format, ##args)
#define SDK_WARN(format, args...)   SC_DIAG_LOG(SC_LOG_WARNING_LEVEL, format, ##args)
#define SDK_INFO(format, args...)   SC_DIAG_LOG(SC_LOG_INFO_LEVEL, format, ##args)
#define SDK_DBG(format, args...)    SC_DIAG_LOG(SC_LOG_DEBUG_LEVEL, format, ##args)
#else
#define SDK_LOG(format, args...)    fprintf(stderr, "[%s,%d] " format "\r\n", __FUNCTION__, __LINE__, ## args);
#define SDK_ERR(format, args...)    SDK_LOG(format, ##args)
#define SDK_WARN(format, args...)   SDK_LOG(format, ##args)
#define SDK_INFO(format, args...)   SDK_LOG(format, ##args)
#define SDK_DBG(format, args...)    SDK_LOG(format, ##args)
#endif
#endif /* _SC_SDK_LOG_H */
