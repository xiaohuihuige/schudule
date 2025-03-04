#include "net-schedule.h"
#include "net-common.h"
#include <unistd.h>

int async_function_trigger(void *args)
{
    LOG("aync function trigger...");
    char buffer[80] = {0};

    get_now_date(buffer, sizeof(buffer));

    LOG("time 5%s", buffer);

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
    if (!scher) {
        ERR("create Scheduler");
        return -1;
    }

    addTriggerTask(scher, sync_function_trigger, NULL, SYNC_FLAGS);
    
    for (int i = 0; i < 20; i++) {
        addTriggerTask(scher, async_function_trigger, NULL, ASYNC_FLAGS);
    }

    addTriggerTask(scher, sync_function_trigger, NULL, SYNC_FLAGS);

    int number = 5;
    while (number > 0) {
        sleep(1);
        number--;
    }

    destroyTaskScheduler(scher);
}