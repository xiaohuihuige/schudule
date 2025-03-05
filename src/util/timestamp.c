#include "timestamp.h"

long long get_time_ms(void)
{
    struct timeval currentTime;
    gettimeofday(&currentTime, NULL);
    long long milliseconds = currentTime.tv_sec * 1000LL + currentTime.tv_usec / 1000LL;
    return milliseconds;
}

long long get_time_us(void)
{
    struct timeval currentTime;
    gettimeofday(&currentTime, NULL);
    return (currentTime.tv_sec * 1000LL * 1000LL + currentTime.tv_usec);
}

void get_now_date(char *buffer, int len)
{
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    if (!tm_info)
        return;

    strftime(buffer, len, "%Y-%m-%d %H:%M:%S", tm_info);

    return;
}


void get_now_ms_date(char *buffer, int len) 
{
    struct timeval tv;
    gettimeofday(&tv, NULL); // 获取当前时间，包括微秒

    time_t now = tv.tv_sec; // 获取秒
    struct tm *tm_info = localtime(&now);
    if (!tm_info) {
        return;
    }

    // 格式化日期和时间，包含秒和毫秒
    snprintf(buffer, len, "%04d-%02d-%02d %02d:%02d:%02d:%03ld",
             tm_info->tm_year + 1900, // 年份
             tm_info->tm_mon + 1,     // 月份
             tm_info->tm_mday,        // 日期
             tm_info->tm_hour,        // 小时
             tm_info->tm_min,         // 分钟
             tm_info->tm_sec,         // 秒
             tv.tv_usec / 1000);      // 毫秒
}