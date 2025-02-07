#include "net-log.h"
#include "net-schedule.h"
#include "color.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/syscall.h>

#define LOG_BUF_SIZE        (1024)
#define LOG_IOVEC_MAX       (10)
#define FILENAME_LEN        (256)
#define FILESIZE_LEN        (10*1024*1024UL)
#define LOG_BUF_SIZE        (1024)
#define LOG_TIME_SIZE       (32)
#define LOG_LEVEL_SIZE      (32)
#define LOG_TAG_SIZE        (32)
#define LOG_PNAME_SIZE      (32)
#define LOG_TEXT_SIZE       (256)
#define LOG_LEVEL_DEFAULT   LOG_INFO
#define LOG_IO_OPS

typedef enum
{
    LOG_FUNCLINE_BIT = 0,
    LOG_TID_BIT      = 1,
    LOG_PID_BIT      = 2,
    LOG_TIMESTAMP_BIT = 3,
    LOG_TAG_BIT      = 4,
} flags;

static const char *log_level_str[] = 
{
    "EMERG",
    "ALERT",
    "CRIT",
    "ERR",
    "WARN",
    "NOTICE",
    "INFO",
    "DEBUG",
    "VERBOSE",
};

net_log g_log_info = 
{
    .thread_once = PTHREAD_ONCE_INIT,
    .log_prefix  = 0,
    .type        = 1,
    .log_fd      = STDERR_FILENO,
};

void log_get_time(char *str, int len)
{
    if (str == NULL || len <= 0)
        return;

    char date_fmt[20];
    char date_ms[32];
   
    struct timeval tv;
    struct tm now_tm;
    gettimeofday(&tv, NULL);
    time_t now_sec = tv.tv_sec;
    int now_ms = tv.tv_usec/1000;
    localtime_r(&now_sec, &now_tm);

    strftime(date_fmt, 20, "%Y-%m-%d %H:%M:%S", &now_tm);
    snprintf(date_ms, sizeof(date_ms), "%03d", now_ms);
    snprintf(str, len, "[%s.%s]", date_fmt, date_ms);
}

static int _log_print(int lvl, const char *tag,
                      const char *file, int line,
                      const char *func, const char *msg)
{
    int i = -1;
    struct iovec vec[LOG_IOVEC_MAX];
    char s_time[LOG_TIME_SIZE]; //时间
    char s_lvl[LOG_LEVEL_SIZE]; //等级
    char s_tag[LOG_TAG_SIZE];   //类型
    char s_pid[LOG_PNAME_SIZE]; //进程pid
    char s_tid[LOG_PNAME_SIZE]; //线程id
    char s_file[LOG_TEXT_SIZE]; //日志定位
    char s_msg[LOG_BUF_SIZE];   //内容

    pthread_mutex_lock(&g_log_info.lock);
    log_get_time(s_time, sizeof(s_time));

    switch (lvl)
    {
    case LOG_EMERG:
    case LOG_ALERT:
    case LOG_CRIT:
    case LOG_ERR:
        snprintf(s_lvl, sizeof(s_lvl),
                 B_RED("[%7s]"), log_level_str[lvl]);
        snprintf(s_msg, sizeof(s_msg), RED("%s"), msg);
        break;
    case LOG_WARNING:
        snprintf(s_lvl, sizeof(s_lvl),
                 B_YELLOW("[%7s]"), log_level_str[lvl]);
        snprintf(s_msg, sizeof(s_msg), YELLOW("%s"), msg);
        break;
    case LOG_INFO:
        snprintf(s_lvl, sizeof(s_lvl),
                 B_GREEN("[%7s]"), log_level_str[lvl]);
        snprintf(s_msg, sizeof(s_msg), GREEN("%s"), msg);
        break;
    case LOG_DEBUG:
        snprintf(s_lvl, sizeof(s_lvl),
                 B_WHITE("[%7s]"), log_level_str[lvl]);
        snprintf(s_msg, sizeof(s_msg), WHITE("%s"), msg);
        break;
    default:
        snprintf(s_lvl, sizeof(s_lvl),
                 "[%7s]", log_level_str[lvl]);
        snprintf(s_msg, sizeof(s_msg), "%s", msg);
        break;
    }

    snprintf(s_tid, sizeof(s_tid), "[tid:%d]", (int)gettid());
    snprintf(s_pid, sizeof(s_pid), "[pid:%d]", getpid());
    snprintf(s_file, sizeof(s_file), "[%s:%3d: %s] ", file, line, func);
    snprintf(s_tag, sizeof(s_tag), "[%s]", tag);

    set_bit(g_log_info.log_prefix, LOG_PID_BIT);
    set_bit(g_log_info.log_prefix ,LOG_PID_BIT);
    set_bit(g_log_info.log_prefix, LOG_TID_BIT);
    set_bit(g_log_info.log_prefix, LOG_TAG_BIT);
    set_bit(g_log_info.log_prefix, LOG_FUNCLINE_BIT);

    if (get_bit(g_log_info.log_prefix, LOG_PID_BIT))
    {
        vec[++i].iov_base = (void *)s_time;
        vec[i].iov_len = strlen(s_time);
    }

    if (get_bit(g_log_info.log_prefix ,LOG_PID_BIT)) {
        vec[++i].iov_base = (void *)s_pid;
        vec[i].iov_len = strlen(s_pid);
    }

    if (get_bit(g_log_info.log_prefix, LOG_TID_BIT)) {
        vec[++i].iov_base = (void *)s_tid;
        vec[i].iov_len = strlen(s_tid);
    }

    vec[++i].iov_base = (void *)s_lvl;
    vec[i].iov_len = strlen(s_lvl);

    if (get_bit(g_log_info.log_prefix, LOG_TAG_BIT)) {
        vec[++i].iov_base = (void *)s_tag;
        vec[i].iov_len = strlen(s_tag);
    }

    if (get_bit(g_log_info.log_prefix, LOG_FUNCLINE_BIT)) {
        vec[++i].iov_base = (void *)s_file;
        vec[i].iov_len = strlen(s_file);
    }

    vec[++i].iov_base = (void *)s_msg;
    vec[i].iov_len = strlen(s_msg);

    writev(g_log_info.log_fd, vec, i+1);
    
    pthread_mutex_unlock(&g_log_info.lock);

    return 0;
}

int log_print(int lvl, const char *tag, const char *file, int line,
        const char *func, const char *fmt, ...)
{
    va_list ap;
    char buf[LOG_BUF_SIZE] = {0};
    va_start(ap, fmt);
    int n = vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

    if (n < 0) {
        fprintf(stderr, "vsnprintf errno:%d\n", errno);
        return -1;
    }

    return _log_print(lvl, tag, file, line, func, buf);
}



