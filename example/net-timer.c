#include "net-common.h"
#include "net-schedule.h"

timer_ptr timer_delete1 = NULL;

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
    ///net_async_delete_timer_task(timer_delete1);
    net_delete_timer_task(timer_delete1);
    timer_delete1 = NULL;
    return 0;
}

int main(int argc, char *argv[]) 
{
    sche_ptr scher = net_create_scheduler();
    if (scher == NULL)
    {
        ERR("create Scheduler");
        return -1;
    }

    LOG("time:[%lld]", get_time_ms());

    timer_delete1 = net_add_timer_task(scher, 2000, 2000, function_timer_delete, NULL);

    timer_ptr timer_once = net_add_timer_task(scher, 4000, 0, function_timer_once, NULL);

    timer_ptr timer_repeat = net_add_timer_task(scher, 3000, 1000, function_timer_repeat, NULL);

    LOG("time:[%lld]", get_time_ms());
    LOG("list_count_nodes:[%ld]", list_count_nodes(&scher->timer_list->list));

    int number = 50;
    while (number > 10)
    {
        sleep(1);
        number--;
        if(number == 40)
        {
            net_modify_timer_task(timer_repeat, 3000);

            LOG("list_count_nodes:[%ld]", list_count_nodes(&scher->timer_list->list));
        }

        if (number == 30)
        {
            net_modify_timer_task(timer_repeat, 200);
        }

        if (number == 20)
        {
            //net_delete_timer_task(timer_repeat);
            net_async_delete_timer_task(timer_repeat);
        }
    }

    LOG("list_count_nodes:[%ld]", list_count_nodes(&scher->timer_list->list));

    net_destroy_scheduler(scher);
}