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
    sche_ptr scher = net_create_scheduler();
    if (scher == NULL)
    {
        ERR("create Scheduler");
        return -1;
    }

    for (int i = 0; i < 20; i++)
    {
        net_add_trigger_task(scher, async_function_trigger, NULL, 0);
    }

    long long task_id = net_add_trigger_task(scher, sync_function_trigger, NULL, 1);
    int number = 5;
    while (number > 0)
    {
        sleep(1);
        int result = net_get_result(scher, task_id);
        ERR("get function result %d", result);
        number--;
    }
    net_destroy_scheduler(scher);
}