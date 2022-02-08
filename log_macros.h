#ifndef _ACAP_LOG_MACROS_H_
#define _ACAP_LOG_MACROS_H_

//#include <syslog.h>

#define info(...)                                                              \
    G_STMT_START                                                               \
    {                                                                          \
        if (Logger::instance() != nullptr) {                                   \
            if (Logger::instance()->should_log_to_console()) {                 \
                g_print(__VA_ARGS__);                                          \
                g_print("\n");                                                 \
            }                                                                  \
        }                                                                      \
    }                                                                          \
    G_STMT_END

#define error(...)                                                             \
    G_STMT_START                                                               \
    {                                                                          \
                g_printerr(__VA_ARGS__);                                       \
                g_printerr("\n");                                              \
    }                                                                          \
    G_STMT_END

#define warning(...)                                                           \
    G_STMT_START                                                               \
    {                                                                          \
                g_printerr(__VA_ARGS__);                                       \
                g_printerr("\n");                                              \
    }                                                                          \
    G_STMT_END

#define debug(...)                                                             \
    G_STMT_START                                                               \
    {                                                                          \
            g_print(__VA_ARGS__);                                              \
            g_print("\n");                                                     \
    }                                                                          \
    G_STMT_END

#define FENTER  debug ("--> %s:%d", __func__, __LINE__)
#define FEXIT   debug ("<-- %s:%d", __func__, __LINE__)

#define FENTER_LAMBDA  debug ("--> %s:%d", __PRETTY_FUNCTION__, __LINE__)
#define FEXIT_LAMBDA   debug ("<-- %s:%d", __PRETTY_FUNCTION__, __LINE__)

#endif /* _ACAP_LOG_MACROS_H_ */
