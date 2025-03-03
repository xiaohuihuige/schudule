#include "net-schedule.h"
#include "net-signal.h"

extern const struct event_ops epollops;

static int _triggerEvent(TaskScheduler * scher)
{
    assert(scher);

    MUTEX_LOCK(&scher->myMutex);
    task_list *task_node = net_task_list_pop_head(scher->triggerTaskQueue);
    if (!task_node) {
        MUTEX_UNLOCK(&scher->myMutex);
        return 0;
    }
    MUTEX_UNLOCK(&scher->myMutex);

    TriggerEvent * handle = (TriggerEvent *)task_node->task;
    if (handle && handle->function) {
        SDBG("doing function... task_id %lld", handle->task_id);
        
        handle->function(handle->args);
        
        if (handle->sync_flags) {
            TaskScheduler * scher = (TaskScheduler *)handle->scher;
            COND_SIGNAL(&scher->myCond);
        }
    }

    net_free(handle);
    net_free(task_node);

    return 1;
}

static int _timerEvent(TaskScheduler * scher)
{
    assert(scher);

    long long start_time = get_time_ms();

    task_list *task_node = NULL;
    task_list *temp_node = NULL;
    list_for_each_entry_safe(task_node, temp_node, &scher->timerTaskQueue->list, list) {
        TaskTimer * handle = (TaskTimer *)task_node->task;

        long long interval_time = start_time - handle->time_stamp;
        if (handle->first_ms == 0) {
            handle->first_ms = -1;
            handle->function(handle->args);
            handle->time_stamp = get_time_ms();
        }

        if (handle->first_ms > 0 && interval_time >= handle->first_ms) {
            handle->first_ms = -1;
            handle->function(handle->args);
            handle->time_stamp = get_time_ms();
        } else if (handle->first_ms < 0) {
            if (handle->repeat_ms <= 0) {
                MUTEX_LOCK(&scher->myMutex);
                net_task_list_del(task_node, TaskTimer);
                MUTEX_UNLOCK(&scher->myMutex);
            }
            else if (handle->repeat_ms > 0 && interval_time >= handle->repeat_ms) {
                handle->function(handle->args);
                handle->time_stamp = get_time_ms();
            }
        }

        if (handle->async_del_flags == 1) {
            MUTEX_LOCK(&scher->myMutex);
            net_task_list_del(task_node, TaskTimer);
            MUTEX_UNLOCK(&scher->myMutex);
        }
    }

    long long end_time = get_time_ms();

    return end_time - start_time;
}

static void *_eventLoop(void *obj)
{
    assert(obj);

    TaskScheduler * scher = (TaskScheduler *)obj;

    int timeout = -1;
    while (scher->loop == SHECH_RUNNING) {

        timeout = _triggerEvent(scher);

        timeout += _timerEvent(scher);

        if (scher->ops->dispatch(scher->ctx, timeout)) 
            SERR("dispatch failed");
    }
    return NULL;
}

long long addTriggerTask(TaskScheduler * scher, TriggerFunc function, void *args, int sync_flags)
{
    assert(scher);

    TriggerEvent * trigger = (TriggerEvent *)calloc(1, sizeof(TriggerEvent));
    if (!trigger)
        return NET_FAIL;
    
    long long task_id   = get_time_us();
    trigger->sync_flags = sync_flags;
    trigger->args       = args;
    trigger->function   = function;
    trigger->scher      = scher;
    trigger->task_id    = task_id;

    SDBG("add trigger task... task_id %lld", trigger->task_id);

    MUTEX_LOCK(&scher->myMutex);
    net_task_list_add_tail(scher->triggerTaskQueue, (void *)trigger);
    MUTEX_UNLOCK(&scher->myMutex);

    net_send_signal(scher->inner_fd);

    if (sync_flags) {
        MUTEX_LOCK(&scher->myMutex);
        COND_WAIT(&scher->myCond, &scher->myMutex);
        MUTEX_UNLOCK(&scher->myMutex);
    }

    return task_id;
}

TaskTimer * addTimerTask(TaskScheduler * scher, int fist_ms, int repeat_ms, TriggerFunc function, void *args)
{
    assert(scher);

    TaskTimer * timer = (TaskTimer * )calloc(1, sizeof(TaskTimer));
    if (!timer)
        return NULL;
    
    timer->scher       = (void *)scher;
    timer->args        = args;
    timer->first_ms    = fist_ms;
    timer->function    = function;
    timer->repeat_ms   = repeat_ms;
    timer->time_stamp  = get_time_ms();
    timer->async_del_flags = 0;

    MUTEX_LOCK(&scher->myMutex);
    net_task_list_add_tail(scher->timerTaskQueue, (void *)timer);
    MUTEX_UNLOCK(&scher->myMutex);

    net_send_signal(scher->inner_fd);

    SDBG("create timer %p, first %d, repeat %d", timer, fist_ms, repeat_ms);

    return timer;
}

void deleteTimerTask(TaskTimer * timer)
{
    assert(timer);

    TaskScheduler * scher = (TaskScheduler *)timer->scher;
    if (!scher)
        return;

    if (list_empty(&scher->timerTaskQueue->list))
        return;

    SDBG("delete timer task %p", timer);

    MUTEX_LOCK(&scher->myMutex);
    net_task_list_find_del(scher->timerTaskQueue, TaskTimer, timer);
    MUTEX_UNLOCK(&scher->myMutex);
}

void asyncDeleteTimerTask(TaskTimer * timer)
{
    assert(timer);

    TaskScheduler * scher = (TaskScheduler *)timer->scher;
    if (!scher)
        return;

    if (list_empty(&scher->timerTaskQueue->list))
        return;

    task_list *task_pos = NULL;
    MUTEX_LOCK(&scher->myMutex);
    net_task_list_find(scher->timerTaskQueue, task_pos, TaskTimer, timer);
    if (task_pos && task_pos->task == timer) {
        ((TaskTimer *)task_pos->task)->async_del_flags = 1;
    }
    MUTEX_UNLOCK(&scher->myMutex);

    SDBG("async delete timer task %p", timer);
    return;
}

int modifyTimerTask(TaskTimer * timer, int repeat_ms)
{
    assert(timer);

    TaskScheduler * scher = (TaskScheduler *)timer->scher;
    if (!scher)
        return NET_FAIL;

    if (list_empty(&scher->timerTaskQueue->list))
        return NET_FAIL;

    task_list *task_pos = NULL;

    MUTEX_LOCK(&scher->myMutex);

    net_task_list_find(scher->timerTaskQueue, task_pos, TaskTimer, timer);
    if (task_pos && task_pos->task == timer) {
        ((TaskTimer *)task_pos->task)->repeat_ms = repeat_ms;
    }

    MUTEX_UNLOCK(&scher->myMutex);

    SDBG("modify repeat time %p, repeat %d", timer, repeat_ms);
    return NET_SUCCESS;
}

EpollEvent * createReader(TaskScheduler * scher, SOCKET fd, EventFunc function, void *args)
{
    assert(scher);

    EpollEvent * event = (EpollEvent * )calloc(1, sizeof(EpollEvent));
    if (!event) 
        return NULL;

    event->args = args;
    event->function = function;
    event->evfd = fd;
    event->flags |= EVENT_READ;
    event->flags |= EVENT_PERSIST;
    event->scher = (void *)scher;

    if (scher->ops->add(scher->ctx, event)) {
        net_free(event);
        SERR("add epoll error");
        return NULL;
    }

    SDBG("create reader event, %p", event);

    return event;
}

void deleteReader(EpollEvent * event)
{
    assert(event);

    TaskScheduler * scher = (TaskScheduler *)event->scher;
    if (!scher)
        return;

    SDBG("delete reader event, %p", event);
    scher->ops->del(scher->ctx, event);
    net_free(event);
}

int modifyReader(EpollEvent * event)
{
    assert(event);
        
    TaskScheduler * scher = (TaskScheduler *)event->scher;
    if (!scher)
        return NET_FAIL;

    SDBG("modify reader event %p", event);

    return scher->ops->mod(scher->ctx, event);
}

TaskScheduler * createTaskScheduler(void)
{
    TaskScheduler * scher = (TaskScheduler *)calloc(1, sizeof(TaskScheduler));
    if (!scher)
        return NULL;
    

    scher->loop = SHECH_STOP;

    do {

        MUTEX_INIT(&scher->myMutex);

        COND_INIT(&scher->myCond);

        scher->ops = (void *)&epollops;
        scher->ctx = scher->ops->init();
        if (!scher->ctx)
            break;
 
        scher->timerTaskQueue = net_task_list_init();
        if (!scher->timerTaskQueue)
            break;
    
        scher->triggerTaskQueue = net_task_list_init();
        if (!scher->triggerTaskQueue)
            break;

        scher->inner_fd = net_create_signal();
        if (scher->inner_fd <= 0)
            break;

        scher->inner_signal = createReader(scher, scher->inner_fd, net_recv_signal, NULL);
        if (!scher->inner_signal)
            break;

        CREATE_THREAD(scher->sch_pid, _eventLoop, (void *)scher, NULL);

        scher->loop = SHECH_RUNNING;

        SDBG("scheduler started %p", scher);
    } while (0);

    if (scher->loop == SHECH_STOP) {
        SERR("scheduler init failed");
        destroyTaskScheduler(scher);
        return NULL;
    }

    return scher;
}

void destroyTaskScheduler(TaskScheduler * scher)
{
    assert(scher);

    scher->loop = SHECH_STOP;

    if (scher->sch_pid > 0) {
        net_send_signal(scher->inner_fd);
        JOIN_THREAD(scher->sch_pid);
    }

    MUTEX_DESTROY(&scher->myMutex);

    COND_DESTROY(&scher->myCond);

    if (scher->triggerTaskQueue)
        net_task_list_each_free(scher->triggerTaskQueue, TriggerEvent);
    
    if (scher->timerTaskQueue)
        net_task_list_each_free(scher->timerTaskQueue, TaskTimer);
    
    if (scher->inner_signal) {
        deleteReader(scher->inner_signal);
        scher->inner_signal = NULL;
    }
    
    if (scher->inner_fd > 0) {
        net_close_signal(scher->inner_fd);
        scher->inner_fd = 0;
    }
   
    if (scher->ctx) {
        scher->ops->deinit(scher->ctx);
        scher->ctx = NULL;
        scher->ops = NULL;
    }

    SDBG("delete scheduler %p", scher);

    SDBG("delete scheduler successfully"); 

    net_free(scher);

    return;
}

