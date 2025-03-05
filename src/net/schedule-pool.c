#include "schedule-pool.h"

SchedulePool *threadPoolCreate(int threadCount)
{
    SchedulePool *scherpool = CALLOC(1, SchedulePool);
    if (!scherpool)
        return NULL;

    scherpool->threadCount  = threadCount;
    scherpool->thread_index = 0;

    MUTEX_INIT(&scherpool->myMutex);

    scherpool->scher = (TaskScheduler **)malloc(threadCount * sizeof(TaskScheduler *));

    for (int i = 0; i < threadCount; i++) {
        scherpool->scher[i] = createTaskScheduler();
    }

    LOG("Create thread successfull %p", scherpool);

    return scherpool;
}

void threadPoolAddTask(SchedulePool *pool, TriggerFunc function, void *arg)
{
    assert(pool);

    MUTEX_LOCK(&pool->myMutex);
    addTriggerTask(pool->scher[pool->thread_index], function, arg, ASYNC_FLAGS);
    if (pool->thread_index == pool->threadCount) {
        pool->thread_index = 0;
    }
    MUTEX_UNLOCK(&pool->myMutex);
}

void threadPoolDestroy(SchedulePool *pool) 
{
    assert(pool);

    MUTEX_DESTROY(&pool->myMutex);

    for (int i = 0; i < pool->threadCount; i++) {
        if (pool->scher[i])
            destroyTaskScheduler(pool->scher[i]);
    }

    LOG("Destroy thread successfull %p", pool);

    FREE(pool);
}