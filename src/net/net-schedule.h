#ifndef __SCHULE_H__
#define __SCHULE_H__

#include "net-common.h"
#include "net-task.h"

#define SHECH_RUNNING   1
#define SHECH_STOP      0
#define SHECH_DEBUG     0

#define SDBG(fmt, args...) \
    if (SHECH_DEBUG)       \
    {                      \
        DBG(fmt, ##args);    \
    }

#define SERR(fmt, args...) \
    if (SHECH_DEBUG)       \
    {                      \
        ERR(fmt, ##args);    \
    }

typedef int (*task_function)(void *arg);
typedef int (*event_function)(int fd, void *arg);

typedef enum {
    EVENT_TIMEOUT  = 1<<0,
    EVENT_READ     = 1<<1,
    EVENT_WRITE    = 1<<2,
    EVENT_SIGNAL   = 1<<3,
    EVENT_PERSIST  = 1<<4,
    EVENT_ET       = 1<<5,
    EVENT_FINALIZE = 1<<6,
    EVENT_CLOSED   = 1<<7,
    EVENT_ERROR    = 1<<8,
    EVENT_EXCEPT   = 1<<9,
} event_flags;

typedef struct
{
    long long task_id;
    int result;
} result_info, *result_ptr;

typedef struct 
{
    long long task_id;
    void *scher;
    void *args;
    int sync_flags;
    task_function function;
} trigger_info, *trigger_ptr;

typedef struct 
{
    void *scher;
    int repeat_ms;
    int first_ms;
    void *args;
    long long time_stamp;
    int async_del_flags;
    task_function function;
} timer_info, *timer_ptr;

typedef struct 
{
    void *scher;
    SOCKET evfd;
    event_flags flags;
    void *args;
    event_function function;
} event_info, *ev_ptr;

typedef struct {
    void *(*init)();
    void (*deinit)(void *ctx);
    int (*add)(void *ctx, ev_ptr event);
    int (*del)(void *ctx, ev_ptr event);
    int (*mod)(void *ctx, ev_ptr event);
    int (*dispatch)(void *ctx, int timeout);
} event_ops;

typedef struct
{
    volatile int loop;
    ev_ptr inner_signal;
    SOCKET inner_fd;
    void *ctx;
    event_ops *ops;
    task_list *trigger_list;
    task_list *timer_list;
    task_list *result_list;
    pthread_t sch_pid;
    pthread_mutex_t lock;
    pthread_cond_t cond;
} scheduler_info, *sche_ptr;

sche_ptr net_create_scheduler(void);
void net_destroy_scheduler(sche_ptr scher);

ev_ptr net_create_reader(sche_ptr scher, SOCKET fd, event_function function, void *args);
void net_delete_reader(ev_ptr event);
void net_async_delete_reader(ev_ptr event);
int net_modify_reader(ev_ptr event);

long long net_add_trigger_task(sche_ptr scher, task_function function, void *args, int sync_flags);

timer_ptr net_add_timer_task(sche_ptr scher, int fist_ms, int repeat_ms, task_function function, void *args);
void net_delete_timer_task(timer_ptr timer);
void net_async_delete_timer_task(timer_ptr timer);
int net_modify_timer_task(timer_ptr timer, int repeat_ms);

int net_get_result(sche_ptr scher, long long task_id);

#endif // !__SCHULE_H__
