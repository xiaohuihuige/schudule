#ifndef __NET_SCHEDULE_POOL_H__
#define __NET_SCHEDULE_POOL_H__

#include "net-common.h"
#include "net-schedule.h"

#define MAX_THREADS 4

typedef struct {
    int threadCount;
    int thread_index;
    Mutex myMutex;
    TaskScheduler **scher;
} SchedulePool;

SchedulePool *threadPoolCreate(int threadCount);
void threadPoolAddTask(SchedulePool *pool, TriggerFunc function, void *arg);
void threadPoolDestroy(SchedulePool *pool);

#endif