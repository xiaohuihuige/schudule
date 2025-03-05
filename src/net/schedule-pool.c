#include "schedule-pool.h"

SchedulePool *threadPoolCreate(int threadCount)
{
    SchedulePool *scherpool = CALLOC(1, SchedulePool);
    if (!scherpool)
        return NULL;

    scherpool->threadCount  = threadCount;
    scherpool->shutdown     = SHECH_RUNNING;
    scherpool->thread_index = 0;

    TaskScheduler *scher[threadCount];

    for (int i = 0; i < threadCount; i++) {
        scher[i] = createTaskScheduler();
    }

    scherpool->scher = scher;

    return scherpool;
}

void threadPoolAddTask(SchedulePool *pool, TriggerFunc function, void *arg)
{
    addTriggerTask(pool->scher[pool->thread_index], function, arg, ASYNC_FLAGS);
    if (pool->thread_index == pool->threadCount) {
        pool->thread_index = 0;
    }
}

void threadPoolDestroy(SchedulePool *pool) 
{

}