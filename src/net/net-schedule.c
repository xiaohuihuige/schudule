#include "net-schedule.h"
#include "net-signal.h"
#include "log.h"

extern const struct event_ops epollops;

static int _triggerEvent(TaskScheduler * scher)
{
    assert(scher);

    MUTEX_LOCK(&scher->myMutex);
    FifoQueue *task_node = dequeue(scher->taskQueue);
    if (!task_node) {
        MUTEX_UNLOCK(&scher->myMutex);
        return 0;
    }
    MUTEX_UNLOCK(&scher->myMutex);

    TriggerEvent * trigger = (TriggerEvent *)task_node->task;
    if (trigger) {
        DBG("doing function... task_id %lld, flag %d", trigger->task_id, trigger->sync_flags);
        if (trigger->function)
            trigger->function(trigger->args);
        
        if (trigger->sync_flags) {
            COND_SIGNAL(&trigger->condition);
        } else {
            FREE(trigger);
        }
    }

    FREE(task_node);

    return 1;
}

static void _wakeupCond(TaskScheduler * scher)
{
    assert(scher);

    FifoQueue *task_node = NULL;
    FifoQueue *temp_node = NULL;
    list_for_each_entry_safe(task_node, temp_node, &scher->taskQueue->list, list) {
        TriggerEvent * trigger = (TriggerEvent *)task_node->task;
        if (trigger && trigger->sync_flags) {
            MUTEX_LOCK(&scher->myMutex);
            COND_BROADCAST(&trigger->condition);
            MUTEX_UNLOCK(&scher->myMutex);
        }
    }
}

static int _timerEvent(TaskScheduler * scher)
{
    assert(scher);

    long long start_time = get_time_ms();

    FifoQueue *task_node = NULL;
    FifoQueue *temp_node = NULL;
    list_for_each_entry_safe(task_node, temp_node, &scher->timerQueue->list, list) {
        TaskTimer * timer = (TaskTimer *)task_node->task;
        if (!timer) 
            continue;

        long long interval_time = start_time - timer->time_stamp;
        if (timer->first_ms == 0) {
            timer->first_ms = -1;
            timer->function(timer->args);
            timer->time_stamp = get_time_ms();
        }

        if (timer->first_ms > 0 && interval_time >= timer->first_ms) {
            timer->first_ms = -1;
            timer->function(timer->args);
            timer->time_stamp = get_time_ms();
        } else if (timer->first_ms < 0) {
            if (timer->repeat_ms <= 0) {
                MUTEX_LOCK(&scher->myMutex);
                deleteFifoQueueTask(task_node, TaskTimer);
                MUTEX_UNLOCK(&scher->myMutex);
            }
            else if (timer->repeat_ms > 0 && interval_time >= timer->repeat_ms) {
                timer->function(timer->args);
                timer->time_stamp = get_time_ms();
            }
        }

        if (timer->async_del_flags) {
            MUTEX_LOCK(&scher->myMutex);
            deleteFifoQueueTask(task_node, TaskTimer);
            MUTEX_UNLOCK(&scher->myMutex);
        }
    }

    return get_time_ms() - start_time;
}

static void *_eventLoop(void *obj)
{
    assert(obj);

    TaskScheduler * scher = (TaskScheduler *)obj;

    int timeout = -1;
    while (scher->isRunning == SHECH_RUNNING) {

        timeout = _triggerEvent(scher);

        timeout += _timerEvent(scher);

        if (scher->ops->dispatch(scher->context, timeout)) 
            ERR("dispatch failed");
    }
    return NULL;
}

long long addTriggerTask(TaskScheduler * scher, TriggerFunc function, void *args, int sync_flags)
{
    assert(scher);

    TriggerEvent * trigger = CALLOC(1, TriggerEvent);
    if (!trigger)
        return NET_FAIL;

    long long task_id   = get_time_us();

    trigger->sync_flags = sync_flags;
    trigger->args       = args;
    trigger->function   = function;
    trigger->scher      = scher;
    trigger->task_id    = task_id ;

    if (sync_flags) 
        COND_INIT(&trigger->condition);

    //printf("add trigger task... task_id %lld, flag %d \n", trigger->task_id, trigger->sync_flags);

    MUTEX_LOCK(&scher->myMutex);
    enqueue(scher->taskQueue, (void *)trigger);
    MUTEX_UNLOCK(&scher->myMutex);

    sendSignal(scher->signalFd);

    if (sync_flags) {
        MUTEX_LOCK(&scher->myMutex);
        COND_WAIT(&trigger->condition, &scher->myMutex);
        MUTEX_UNLOCK(&scher->myMutex);

        COND_DESTROY(&trigger->condition);
        FREE(trigger);
    }

    return task_id;
}

TaskTimer * addTimerTask(TaskScheduler * scher, int fist_ms, int repeat_ms, TriggerFunc function, void *args)
{
    assert(scher);

    TaskTimer * timer = CALLOC(1, TaskTimer);
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
    enqueue(scher->timerQueue, (void *)timer);
    MUTEX_UNLOCK(&scher->myMutex);

    sendSignal(scher->signalFd);

    DBG("create timer %p, first %d, repeat %d", timer, fist_ms, repeat_ms);

    return timer;
}

void deleteTimerTask(TaskTimer *timer)
{
    assert(timer);

    TaskScheduler * scher = (TaskScheduler *)timer->scher;
    if (!scher)
        return;

    if (list_empty(&scher->timerQueue->list))
        return;

    DBG("delete timer task %p", timer);

    MUTEX_LOCK(&scher->myMutex);
    FindDeleteFifoQueueTask(scher->timerQueue, TaskTimer, timer);
    FREE(timer);
    MUTEX_UNLOCK(&scher->myMutex);
}

void asyncDeleteTimerTask(TaskTimer * timer)
{
    assert(timer);

    TaskScheduler * scher = (TaskScheduler *)timer->scher;
    if (!scher)
        return;

    if (list_empty(&scher->timerQueue->list))
        return;

    FifoQueue *task_pos = NULL;
    MUTEX_LOCK(&scher->myMutex);
    findFifoQueueTask(scher->timerQueue, task_pos, TaskTimer, timer);
    if (task_pos && task_pos->task == timer) {
        ((TaskTimer *)task_pos->task)->async_del_flags = 1;
    }
    MUTEX_UNLOCK(&scher->myMutex);

    DBG("async delete timer task %p", timer);
    return;
}

int modifyTimerTask(TaskTimer * timer, int repeat_ms)
{
    assert(timer);

    TaskScheduler * scher = (TaskScheduler *)timer->scher;
    if (!scher)
        return NET_FAIL;

    if (list_empty(&scher->timerQueue->list))
        return NET_FAIL;

    FifoQueue *task_pos = NULL;

    MUTEX_LOCK(&scher->myMutex);

    findFifoQueueTask(scher->timerQueue, task_pos, TaskTimer, timer);
    if (task_pos && task_pos->task == timer) {
        ((TaskTimer *)task_pos->task)->repeat_ms = repeat_ms;
    }

    MUTEX_UNLOCK(&scher->myMutex);

    DBG("modify repeat time %p, repeat %d", timer, repeat_ms);
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

    if (scher->ops->add(scher->context, event)) {
        FREE(event);
        ERR("add epoll error");
        return NULL;
    }

    DBG("create reader event, %p", event);

    return event;
}

void deleteReader(EpollEvent * event)
{
    assert(event);

    TaskScheduler * scher = (TaskScheduler *)event->scher;
    if (!scher)
        return;

    DBG("delete reader event, %p", event);
    scher->ops->del(scher->context, event);
    FREE(event);
}

int modifyReader(EpollEvent * event)
{
    assert(event);
        
    TaskScheduler * scher = (TaskScheduler *)event->scher;
    if (!scher)
        return NET_FAIL;

    DBG("modify reader event %p", event);

    return scher->ops->mod(scher->context, event);
}

TaskScheduler * createTaskScheduler(void)
{
    TaskScheduler * scher = CALLOC(1, TaskScheduler);
    if (!scher)
        return NULL;
    
    scher->isRunning = SHECH_STOP;

    do {

        MUTEX_INIT(&scher->myMutex);

        scher->ops = (void *)&epollops;
        scher->context = scher->ops->init();
        if (!scher->context)
            break;
 
        scher->timerQueue = createFifiQueue();
        if (!scher->timerQueue)
            break;
    
        scher->taskQueue = createFifiQueue();
        if (!scher->taskQueue)
            break;

        scher->signalFd = createSignal();
        if (scher->signalFd <= 0)
            break;

        scher->signalEvents = createReader(scher, scher->signalFd, recvSignal, NULL);
        if (!scher->signalEvents)
            break;

        CREATE_THREAD(scher->workerThread, _eventLoop, (void *)scher, NULL);

        scher->isRunning = SHECH_RUNNING;

        DBG("task scheduler started %p", scher);
    } while (0);

    if (!scher->isRunning) {
        ERR("scheduler init failed");
        destroyTaskScheduler(scher);
        return NULL;
    }

    return scher;
}

void destroyTaskScheduler(TaskScheduler * scher)
{
    assert(scher);

    scher->isRunning = SHECH_STOP;
    
    if (scher->workerThread > 0) {
        sendSignal(scher->signalFd);
        JOIN_THREAD(scher->workerThread);
    }

    MUTEX_DESTROY(&scher->myMutex);
    
    if (scher->taskQueue) {
        _wakeupCond(scher);
        destroyFifoQueue(scher->taskQueue, TriggerEvent);
    }
    
    if (scher->timerQueue) {
        destroyFifoQueue(scher->timerQueue, TaskTimer);
    }
    
    if (scher->signalEvents) {
        deleteReader(scher->signalEvents);
        scher->signalEvents = NULL;
    }
    
    closeSignal(scher->signalFd);
    scher->signalFd = 0;
    
    if (scher->context) {
        scher->ops->deinit(scher->context);
        scher->context = NULL;
        scher->ops = NULL;
    }

    DBG("delete scheduler %p", scher);

    DBG("delete scheduler successfully"); 

    FREE(scher);

    return;
}

