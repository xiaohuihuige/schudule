#ifndef __NET_SCHEDULE_POOL_H__
#define __NET_SCHEDULE_POOL_H__

#include "net-common.h"
#include "net-schedule.h"

#define MAX_THREADS 4

typedef struct {
    volatile int shutdown;
    int threadCount;
    int thread_index;
    TaskScheduler **scher;
} SchedulePool;

SchedulePool *threadPoolCreate(int threadCount);

#endif