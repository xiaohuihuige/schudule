#include "net-common.h"
#include "net-schedule.h"

typedef struct
{
    char data[256];
} msg_info;

static int number = 1;

int function_timer_push(void *args)
{
    msg_info msg;
    snprintf(msg.data, sizeof(msg.data), "%s__%d","wmh", number);
    number++;

    ring_buffer_put((ring_buffer *)args, (void *)&msg, sizeof(msg_info));
    LOG("push ------->>>[%s], [%lu]", msg.data, ring_buffer_len((ring_buffer *)args)/sizeof(msg_info));
    return 0;
}

int function_timer_pull(void *args)
{
    msg_info msg;
    ring_buffer_get((ring_buffer *)args, (void *)&msg, sizeof(msg_info));
    LOG("pull <<<------ [%s], [%lu]", msg.data, ring_buffer_len((ring_buffer *)args)/sizeof(msg_info));
    return 0;
}

int main()
{
    ring_buffer * buffer = ring_buffer_init(4096 * 2);
    if (buffer == NULL)
        return -1;
    sche_ptr scher_push = net_create_scheduler();
    if (scher_push == NULL)
    {
        ERR("create Scheduler");
        return -1;
    }

    sche_ptr scher_pull = net_create_scheduler();
    if (scher_push == NULL)
    {
        ERR("create Scheduler");
        return -1;
    }

    timer_ptr timer_push = net_add_timer_task(scher_push, 1000, 200, function_timer_push, (void *)buffer);
    timer_ptr timer_pull = net_add_timer_task(scher_pull, 3000, 500, function_timer_pull, (void *)buffer);

    while (1)
    {
        sleep(1);
    }

    net_destroy_scheduler(scher_push);
    net_destroy_scheduler(scher_pull);
}