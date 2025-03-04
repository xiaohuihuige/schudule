#ifndef __SCHULE_H__
#define __SCHULE_H__

#include "net-common.h"
#include "fifoqueue.h"

#define SHECH_RUNNING   1
#define SHECH_STOP      0
#define SHECH_DEBUG     0

#define ASYNC_FLAGS     0
#define SYNC_FLAGS      1 


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

typedef int (*TriggerFunc)(void *arg);
typedef int (*EventFunc)(int fd, void *arg);

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
} EventFlags;

typedef struct 
{
    long long task_id;
    void *scher;
    void *args;
    int sync_flags;
    TriggerFunc function;
    Cond condition;
} TriggerEvent;

typedef struct 
{
    void *scher;
    int repeat_ms;
    int first_ms;
    void *args;
    long long time_stamp;
    int async_del_flags;
    TriggerFunc function;
} TaskTimer;

typedef struct 
{
    void *scher;
    SOCKET evfd;
    EventFlags flags;
    void *args;
    EventFunc function;
} EpollEvent;

typedef struct {
    void *(*init)();
    void (*deinit)(void *context);
    int (*add)(void *context, EpollEvent * event);
    int (*del)(void *context, EpollEvent * event);
    int (*mod)(void *context, EpollEvent * event);
    int (*dispatch)(void *context, int timeout);
} event_ops;

typedef struct
{
    volatile int isRunning;
    EpollEvent * signalEvents;
    SOCKET signalFd;
    void *context;
    event_ops *ops;
    FifoQueue *taskQueue;
    FifoQueue *timerQueue;
    pthread_t workerThread;
    Mutex myMutex;
    Cond condition;
} TaskScheduler;

TaskScheduler * createTaskScheduler(void);
void destroyTaskScheduler(TaskScheduler * scher);

EpollEvent * createReader(TaskScheduler * scher, SOCKET fd, EventFunc function, void *args);
void deleteReader(EpollEvent * event);
void asyncDeleteReader(EpollEvent * event);
int modifyReader(EpollEvent * event);

long long addTriggerTask(TaskScheduler * scher, TriggerFunc function, void *args, int sync_flags);

TaskTimer * addTimerTask(TaskScheduler * scher, int fist_ms, int repeat_ms, TriggerFunc function, void *args);
void deleteTimerTask(TaskTimer * timer);
void asyncDeleteTimerTask(TaskTimer * timer);
int modifyTimerTask(TaskTimer * timer, int repeat_ms);

#endif // !__SCHULE_H__
