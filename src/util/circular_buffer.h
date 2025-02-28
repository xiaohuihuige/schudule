#ifndef _LINUX_KFIFO_H
#define _LINUX_KFIFO_H
#include "net-common.h"

// 判断x是否是2的次方
#define is_power_of_2(x) ((x) != 0 && (((x) & ((x)-1)) == 0))
// 取a和b中最小值
#define min(a, b) (((a) < (b)) ? (a) : (b))

typedef struct 
{
    void *buffer;            // 缓冲区
    uint32_t size;           // 大小
    uint32_t in;             // 入口位置
    uint32_t out;            // 出口位置
    pthread_mutex_t lock;    // 互斥锁
} CircularBuffer;

CircularBuffer *createCircularBuffer(uint32_t size);
void destroyCircularBuffer(CircularBuffer *ring_buf);
uint32_t writeToCircularBuffer(CircularBuffer *ring_buf, void *buffer, uint32_t size);
uint32_t readFromCircularBuffer(CircularBuffer *ring_buf, void *buffer, uint32_t size);
uint32_t getCircularBufferSize(CircularBuffer *ring_buf);

#endif