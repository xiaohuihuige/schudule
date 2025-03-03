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

    writeToCircularBuffer((CircularBuffer *)args, (void *)&msg, sizeof(msg_info));
    LOG("push ------->>>[%s], [%lu]", msg.data, getCircularBufferSize((CircularBuffer *)args)/sizeof(msg_info));
    return 0;
}

int function_timer_pull(void *args)
{
    msg_info msg;
    readFromCircularBuffer((CircularBuffer *)args, (void *)&msg, sizeof(msg_info));
    LOG("pull <<<------ [%s], [%lu]", msg.data, getCircularBufferSize((CircularBuffer *)args)/sizeof(msg_info));
    return 0;
}

int main()
{
    CircularBuffer * buffer = createCircularBuffer(4096 * 2);
    if (buffer == NULL)
        return -1;
    TaskScheduler * scher_push = createTaskScheduler();
    if (scher_push == NULL)
    {
        ERR("create Scheduler");
        return -1;
    }

    TaskScheduler * scher_pull = createTaskScheduler();
    if (scher_push == NULL)
    {
        ERR("create Scheduler");
        return -1;
    }

    addTimerTask(scher_push, 1000, 200, function_timer_push, (void *)buffer);
    addTimerTask(scher_pull, 3000, 500, function_timer_pull, (void *)buffer);

    while (1)
    {
        sleep(1);
    }

    destroyTaskScheduler(scher_push);
    destroyTaskScheduler(scher_pull);
}