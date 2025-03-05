#include "net-schedule.h"
#include "net-common.h"
#include <unistd.h>

int async_function_trigger(void *args)
{
    LOG("aync function trigger... ");

    sleep(0.5);
    return 200;
}

int sync_function_trigger(void *args)
{
    LOG("sync function trigger...");
    sleep(1);
    return 2001;
}

void async_trigger_task_test()
{
    TaskScheduler * scher = createTaskScheduler();
    if (!scher) {
        ERR("create Scheduler");
        return ;
    }

    for (int i = 0; i < 20; i++) {
        addTriggerTask(scher, async_function_trigger, NULL, ASYNC_FLAGS);
    }
    sleep(10);
    destroyTaskScheduler(scher);
}

void sync_trigger_task_test()
{
    TaskScheduler * scher = createTaskScheduler();
    if (!scher) {
        ERR("create Scheduler");
        return;
    }

    for (int i = 0; i < 20; i++) {
        addTriggerTask(scher, sync_function_trigger, NULL, SYNC_FLAGS);
    }

    destroyTaskScheduler(scher);
}

static void *_eventLoop(void *obj)
{
    LOG("multi_threading aync function trigger... ");
    for (int i = 0; i < 20; i++) {
        addTriggerTask(obj, async_function_trigger, NULL, ASYNC_FLAGS);
    }
    for (int i = 0; i < 20; i++) {
        addTriggerTask(obj, sync_function_trigger, NULL, SYNC_FLAGS);
    }
    return NULL;
}

void multi_threading_async_trigger_task_test()
{
    TaskScheduler * scher = createTaskScheduler();
    pthread_t workerThread[20];
    for (int i = 0; i < 20; i++) {
        CREATE_THREAD(workerThread[i], _eventLoop, (void *)scher, NULL);
    }

    for (int i = 0; i < 20; i++) {
        JOIN_THREAD(workerThread[i]);
    }
    destroyTaskScheduler(scher);
}

void multi_threading_sync_trigger_task_test()
{
    TaskScheduler * scher = createTaskScheduler();
    pthread_t workerThread[20];
    for (int i = 0; i < 20; i++) {
        CREATE_THREAD(workerThread[i], _eventLoop, (void *)scher, NULL);
    }

    for (int i = 0; i < 20; i++) {
        JOIN_THREAD(workerThread[i]);
    }
    destroyTaskScheduler(scher);
}

int main()
{
    //LOG("----------start async_trigger_task_test-------");
    //async_trigger_task_test();
    //LOG("----------start sync_trigger_task_test-------");
    //sync_trigger_task_test();
    LOG("----------start multi_threading_async_trigger_task_test-------");
    multi_threading_async_trigger_task_test();
    LOG("----------start multi_threading_sync_trigger_task_test-------");
    multi_threading_sync_trigger_task_test();
    return EXIT_SUCCESS;
}
