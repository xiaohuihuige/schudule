#include "net-common.h"
#include "shm-cache.h"
#include "net-schedule.h"
static int number = 0;

int function_timer_put(void *args)
{
    if (args == NULL)
        return -1;

    char data[256] = {0};
    if ((number % 2) == 1)
    {
        if (number % 3 == 0)
        {
            snprintf(data, sizeof(data), "function_timer_put time:[%lld]", get_time_ms()); 
        } else 
        {
            snprintf(data, sizeof(data), "wum time213421654:[%d]", number);    
        }
    } else 
    {
        snprintf(data, sizeof(data), "--------wumh---------- time:[%lld]", get_time_ms()); 
    }
    number++;
    shm_cache_put((shm_cache_ptr)args, (uint8_t *)data, strlen(data), number);
    return 0;
}

int function_timer_get(void *args)
{
    if (args == NULL)
        return -1;

    cache_buffer *b = shm_cache_get((shm_cache_ptr)args);
    LOG("%d function_timer_get %s, %d, %d", (int)gettid() % 10, b->payload, b->size, b->type);
    net_free(b);

    return 0;
}

int main()
{
    shm_cache_ptr cache = shm_cache_init(4096); //8192
    if (cache == NULL)
        return -1;

    sche_ptr scher_put = net_create_scheduler();
    if (scher_put == NULL)
    {
        ERR("create Scheduler");
        return -1;
    }

    sche_ptr scher_get = net_create_scheduler();
    if (scher_get == NULL)
    {
        ERR("create Scheduler");
        return -1;
    }


    net_add_timer_task(scher_put, 0, 20, function_timer_put, (void *)cache);

    net_add_timer_task(scher_get, 1000, 20, function_timer_get, (void *)cache);

    while (1)
    {
        sleep(1);
    }
}
