#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include "shm-queue.h"

void queue_init(shared_queue_t *queue) 
{
    queue->head = 0;
    queue->tail = 0;
    sem_init(&queue->mutex, 1, 1);       // 初始化互斥锁
    sem_init(&queue->slots, 1, QUEUE_SIZE); // 初始化可用插槽计数
    sem_init(&queue->items, 1, 0);       // 初始化缓冲区项目计数
}

void queue_destroy(shared_queue_t *queue) 
{
    sem_destroy(&queue->mutex);
    sem_destroy(&queue->slots);
    sem_destroy(&queue->items);
}

void queue_enqueue(shared_queue_t *queue, int item) 
{
    sem_wait(&queue->slots);             // 等待一个空插槽
    sem_wait(&queue->mutex);             // 加锁

    // 将项目添加到队列中
    queue->buffer[queue->tail] = item;
    queue->tail = (queue->tail + 1) % QUEUE_SIZE;

    sem_post(&queue->mutex);             // 解锁
    sem_post(&queue->items);             // 增加项目计数
}

int queue_dequeue(shared_queue_t *queue) 
{
    sem_wait(&queue->items);             // 等待一个项目
    sem_wait(&queue->mutex);             // 加锁

    // 从队列中移除项目
    int item = queue->buffer[queue->head];
    queue->head = (queue->head + 1) % QUEUE_SIZE;

    sem_post(&queue->mutex);             // 解锁
    sem_post(&queue->slots);             // 增加插槽计数

    return item;
}
