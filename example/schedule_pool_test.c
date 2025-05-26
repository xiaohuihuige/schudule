#include "net-schedule.h"
#include "net-common.h"
#include <unistd.h>
#include "schedule-pool.h"


int thread_pool_test(void *args)
{
    LOG("thread_pool_test... ");
    return 0;
}

int main()
{
    SchedulePool * pool = threadPoolCreate(MAX_THREADS);
    if (!pool)
        return EXIT_FAILURE;

    for (int i = 0; i < 50; i++)
        threadPoolAddTask(pool, thread_pool_test, NULL);

    sleep(5);
    
    threadPoolDestroy(pool);

    return EXIT_SUCCESS;
}