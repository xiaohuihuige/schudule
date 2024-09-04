#ifndef __SHM_QUEUE_H__
#define __SHM_QUEUE_H__

#include <semaphore.h>
#include <stddef.h>

#define QUEUE_SIZE 10        // 队列大小

typedef struct {
    int buffer[QUEUE_SIZE];
    int head;
    int tail;
    sem_t mutex;              // 互斥锁，用于保护缓冲区访问
    sem_t slots;              // 信号量，用于跟踪可用的插槽
    sem_t items;              // 信号量，用于跟踪缓冲区中的项目数量
} shared_queue_t;

// 初始化共享队列
void queue_init(shared_queue_t *queue);

// 销毁共享队列
void queue_destroy(shared_queue_t *queue);

// 向队列中添加元素
void queue_enqueue(shared_queue_t *queue, int item);

// 从队列中移除元素
int queue_dequeue(shared_queue_t *queue);

#endif
