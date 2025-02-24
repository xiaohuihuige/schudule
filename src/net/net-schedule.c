#include "net-schedule.h"
#include "net-signal.h"

extern const struct event_ops epollops;

static int _triggerEvent(sche_ptr scher)
{
    assert(scher);

    pthread_mutex_lock(&scher->lock);
    task_list *task_node = net_task_list_pop_head(scher->trigger_list);
    if (!task_node) {
        pthread_mutex_unlock(&scher->lock);
        return 0;
    }
    pthread_mutex_unlock(&scher->lock);

    trigger_ptr handle = (trigger_ptr)task_node->task;
    if (handle && handle->function) {
        SDBG("doing function... task_id %lld", handle->task_id);
        
        handle->function(handle->args);
        
        if (handle->sync_flags) {
            sche_ptr scher = (sche_ptr)handle->scher;
            pthread_cond_signal(&scher->cond);
        }
    }

    net_free(handle);
    net_free(task_node);

    return 1;
}

static int _timerEvent(sche_ptr scher)
{
    assert(scher);

    long long start_time = get_time_ms();

    task_list *task_node = NULL;
    task_list *temp_node = NULL;
    list_for_each_entry_safe(task_node, temp_node, &scher->timer_list->list, list) {
        timer_ptr handle = (timer_ptr)task_node->task;

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
                pthread_mutex_lock(&scher->lock);
                net_task_list_del(task_node, timer_info);
                pthread_mutex_unlock(&scher->lock);
            }
            else if (handle->repeat_ms > 0 && interval_time >= handle->repeat_ms) {
                handle->function(handle->args);
                handle->time_stamp = get_time_ms();
            }
        }

        if (handle->async_del_flags == 1) {
            pthread_mutex_lock(&scher->lock);
            net_task_list_del(task_node, timer_info);
            pthread_mutex_unlock(&scher->lock);
        }
    }

    long long end_time = get_time_ms();

    return end_time - start_time;
}

static void *_eventLoop(void *obj)
{
    assert(obj);

    sche_ptr scher = (sche_ptr)obj;

    int timeout = -1;
    while (scher->loop == SHECH_RUNNING) {

        timeout = _triggerEvent(scher);

        timeout += _timerEvent(scher);

        if (scher->ops->dispatch(scher->ctx, timeout)) 
            SERR("dispatch failed");
    }
    return NULL;
}

long long net_add_trigger_task(sche_ptr scher, task_function function, void *args, int sync_flags)
{
    assert(scher);

    trigger_ptr trigger = (trigger_ptr)calloc(1, sizeof(trigger_info));
    if (!trigger)
        return NET_FAIL;
    
    long long task_id   = get_time_us();
    trigger->sync_flags = sync_flags;
    trigger->args       = args;
    trigger->function   = function;
    trigger->scher      = scher;
    trigger->task_id    = task_id;

    SDBG("add trigger task... task_id %lld", trigger->task_id);

    pthread_mutex_lock(&scher->lock);
    net_task_list_add_tail(scher->trigger_list, (void *)trigger);
    pthread_mutex_unlock(&scher->lock);

    net_send_signal(scher->inner_fd);

    if (sync_flags) {
        pthread_mutex_lock(&scher->lock);
        pthread_cond_wait(&scher->cond, &scher->lock);
        pthread_mutex_unlock(&scher->lock);
    }

    return task_id;
}

timer_ptr net_add_timer_task(sche_ptr scher, int fist_ms, int repeat_ms, task_function function, void *args)
{
    assert(scher);

    timer_ptr timer = (timer_ptr )calloc(1, sizeof(timer_info));
    if (!timer)
        return NULL;
    
    timer->scher       = (void *)scher;
    timer->args        = args;
    timer->first_ms    = fist_ms;
    timer->function    = function;
    timer->repeat_ms   = repeat_ms;
    timer->time_stamp  = get_time_ms();
    timer->async_del_flags = 0;

    pthread_mutex_lock(&scher->lock);
    net_task_list_add_tail(scher->timer_list, (void *)timer);
    pthread_mutex_unlock(&scher->lock);

    net_send_signal(scher->inner_fd);

    SDBG("create timer %p, first %d, repeat %d", timer, fist_ms, repeat_ms);

    return timer;
}

void net_delete_timer_task(timer_ptr timer)
{
    assert(timer);

    sche_ptr scher = (sche_ptr)timer->scher;
    if (!scher)
        return;

    if (list_empty(&scher->timer_list->list))
        return;

    SDBG("delete timer task %p", timer);

    pthread_mutex_lock(&scher->lock);
    net_task_list_find_del(scher->timer_list, timer_info, timer);
    pthread_mutex_unlock(&scher->lock);
}

void net_async_delete_timer_task(timer_ptr timer)
{
    assert(timer);

    sche_ptr scher = (sche_ptr)timer->scher;
    if (!scher)
        return;

    if (list_empty(&scher->timer_list->list))
        return;

    task_list *task_pos = NULL;
    pthread_mutex_lock(&scher->lock);
    net_task_list_find(scher->timer_list, task_pos, timer_info, timer);
    if (task_pos && task_pos->task == timer) {
        ((timer_ptr)task_pos->task)->async_del_flags = 1;
    }
    pthread_mutex_unlock(&scher->lock);

    SDBG("async delete timer task %p", timer);
    return;
}

int net_modify_timer_task(timer_ptr timer, int repeat_ms)
{
    assert(timer);

    sche_ptr scher = (sche_ptr)timer->scher;
    if (!scher)
        return NET_FAIL;

    if (list_empty(&scher->timer_list->list))
        return NET_FAIL;

    task_list *task_pos = NULL;
    pthread_mutex_lock(&scher->lock);
    net_task_list_find(scher->timer_list, task_pos, timer_info, timer);
    if (task_pos && task_pos->task == timer) {
        ((timer_ptr)task_pos->task)->repeat_ms = repeat_ms;
    }
    pthread_mutex_unlock(&scher->lock);
    SDBG("modify repeat time %p, repeat %d", timer, repeat_ms);
    return NET_SUCCESS;
}

ev_ptr net_create_reader(sche_ptr scher, SOCKET fd, event_function function, void *args)
{
    assert(scher);

    ev_ptr event = (ev_ptr )calloc(1, sizeof(event_info));
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

void net_delete_reader(ev_ptr event)
{
    assert(event);

    sche_ptr scher = (sche_ptr)event->scher;
    if (!scher)
        return;

    SDBG("delete reader event, %p", event);
    scher->ops->del(scher->ctx, event);
    net_free(event);
}

int net_modify_reader(ev_ptr event)
{
    assert(event);
        
    sche_ptr scher = (sche_ptr)event->scher;
    if (!scher)
        return NET_FAIL;

    SDBG("modify reader event %p", event);

    return scher->ops->mod(scher->ctx, event);
}


sche_ptr net_create_scheduler(void)
{
    sche_ptr scher = (sche_ptr)calloc(1, sizeof(scheduler_info));
    if (!scher)
        return NULL;
    

    scher->loop = SHECH_STOP;

    do {

        pthread_mutex_init(&scher->lock, NULL);

        pthread_cond_init(&scher->cond, NULL);

        scher->ops = (void *)&epollops;
        scher->ctx = scher->ops->init();
        if (!scher->ctx)
            break;
 
        scher->timer_list = net_task_list_init();
        if (!scher->timer_list)
            break;
    
        scher->trigger_list = net_task_list_init();
        if (!scher->trigger_list)
            break;

        scher->inner_fd = net_create_signal();
        if (scher->inner_fd <= 0)
            break;

        scher->inner_signal = net_create_reader(scher, scher->inner_fd, net_recv_signal, NULL);
        if (!scher->inner_signal)
            break;

        if (NET_SUCCESS != pthread_create(&scher->sch_pid, NULL, _eventLoop, (void *)scher))
            break;
    
        scher->loop = SHECH_RUNNING;

        SDBG("scheduler started %p", scher);
    } while (0);

    if (scher->loop == SHECH_STOP) {
        SERR("scheduler init failed");
        net_destroy_scheduler(scher);
        return NULL;
    }

    return scher;
}

void net_destroy_scheduler(sche_ptr scher)
{
    assert(scher);

    scher->loop = SHECH_STOP;

    if (scher->sch_pid > 0) {
        net_send_signal(scher->inner_fd);
        pthread_join(scher->sch_pid, NULL);
    }

    pthread_mutex_destroy(&scher->lock);
    pthread_cond_destroy(&scher->cond);

    if (scher->trigger_list)
        net_task_list_each_free(scher->trigger_list, trigger_info);
    
    if (scher->timer_list)
        net_task_list_each_free(scher->timer_list, timer_info);
    
    if (scher->inner_signal) {
        net_delete_reader(scher->inner_signal);
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

