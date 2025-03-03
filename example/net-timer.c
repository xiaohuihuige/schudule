#include "net-common.h"
#include "net-schedule.h"

TaskTimer * timer_delete1 = NULL;

int function_timer_repeat(void *args)
{
    LOG("function_timer time:[%lld]", get_time_ms());
    return 0;
}

int function_timer_once(void *args)
{
    LOG("function_timer_once time:[%lld]", get_time_ms());
    return 0;
}

int function_timer_delete(void *args)
{
    LOG("function_timer_delete time:[%lld]", get_time_ms());
    ///asyncDeleteTimerTask(timer_delete1);
    deleteTimerTask(timer_delete1);
    timer_delete1 = NULL;
    return 0;
}

int main(int argc, char *argv[]) 
{
    TaskScheduler * scher = createTaskScheduler();
    if (scher == NULL)
    {
        ERR("create Scheduler");
        return -1;
    }

    LOG("time:[%lld]", get_time_ms());

    timer_delete1 = addTimerTask(scher, 2000, 2000, function_timer_delete, NULL);

    addTimerTask(scher, 4000, 0, function_timer_once, NULL);

    TaskTimer * timer_repeat = addTimerTask(scher, 3000, 1000, function_timer_repeat, NULL);

    LOG("time:[%lld]", get_time_ms());
    LOG("list_count_nodes:[%ld]", list_count_nodes(&scher->timerTaskQueue->list));

    int number = 50;
    while (number > 10)
    {
        sleep(1);
        number--;
        if(number == 40)
        {
            modifyTimerTask(timer_repeat, 3000);

            LOG("list_count_nodes:[%ld]", list_count_nodes(&scher->timerTaskQueue->list));
        }

        if (number == 30)
        {
            modifyTimerTask(timer_repeat, 200);
        }

        if (number == 20)
        {
            //deleteTimerTask(timer_repeat);
            asyncDeleteTimerTask(timer_repeat);
        }
    }

    LOG("list_count_nodes:[%ld]", list_count_nodes(&scher->timerTaskQueue->list));

    destroyTaskScheduler(scher);
}