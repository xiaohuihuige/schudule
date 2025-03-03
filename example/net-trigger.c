#include "net-schedule.h"
#include "net-common.h"
#include <unistd.h>

int async_function_trigger(void *args)
{
    LOG("aync function trigger...");
    sleep(1);
    return 200;
}

int sync_function_trigger(void *args)
{
    LOG("sync function trigger...");
    sleep(3);
    return 2001;
}

int main()
{
    TaskScheduler * scher = createTaskScheduler();
    if (scher == NULL)
    {
        ERR("create Scheduler");
        return -1;
    }

    for (int i = 0; i < 20; i++)
    {
        addTriggerTask(scher, async_function_trigger, NULL, 0);
    }

    addTriggerTask(scher, sync_function_trigger, NULL, 1);
    int number = 5;
    while (number > 0)
    {
        sleep(1);
        number--;
    }
    destroyTaskScheduler(scher);
}