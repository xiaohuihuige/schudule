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