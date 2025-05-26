#ifndef __TIME_STAMP_H__
#define __TIME_STAMP_H__
#include "net-common.h"

long long get_time_ms(void);
long long get_time_us(void);

void get_now_date(char *buffer, int len);
void get_now_ms_date(char *buffer, int len);

#endif // !__TIME_STAMP_H__
