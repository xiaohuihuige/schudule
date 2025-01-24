#ifndef __NET_LOG_H__
#define __NET_LOG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "net-common.h"

#define LOG_TAG "tag"

typedef enum
{
    LOG_EMERG   = 0,  /* system is unusable */
	LOG_ALERT   = 1,  /* action must be taken immediately */
	LOG_CRIT    = 2,  /* critical conditions */
	LOG_ERR     = 3,  /* error conditions */
    LOG_WARNING = 4,  /* warning conditions */
    LOG_NOTICE  = 5,  /* normal but significant condition */
    LOG_INFO    = 6,  /* informational */
    LOG_DEBUG   = 7,  /* debug-level messages */
    LOG_VERB    = 8,  /* verbose messages */
} log_level;

typedef enum {
    LOG_STDIN   = 0, /*stdin*/
    LOG_STDOUT  = 1, /*stdout*/
    LOG_STDERR  = 2, /*stderr*/
    LOG_FILE    = 3,
    LOG_RSYSLOG = 4,
    LOG_MAX_OUTPUT = 255
} log_type;


typedef struct 
{
    int split_size;
    int log_fd;
    int log_prefix;
    int level;
    int type;
    char path[256];
    pthread_mutex_t lock;
    pthread_once_t thread_once;
} net_log, *net_log_ptr;

// int  log_init(int type, const char *ident);
// void log_deinit();
int log_init();
// void log_set_level(int level);
// void log_set_split_size(int size);
// void log_set_rotate(int enable);
// int  log_set_path(const char *path);
int log_print(int lvl, const char *tag, const char *file, int line,
        const char *func, const char *fmt, ...);

#define loge(...) log_print(LOG_ERR, LOG_TAG, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define logw(...) log_print(LOG_WARNING, LOG_TAG, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define logi(...) log_print(LOG_INFO, LOG_TAG, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define logd(...) log_print(LOG_DEBUG, LOG_TAG, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define logv(...) log_print(LOG_VERB, LOG_TAG, __FILE__, __LINE__, __func__, __VA_ARGS__)

#ifdef __cplusplus
}
#endif
#endif // !__NET_LOG_H__
