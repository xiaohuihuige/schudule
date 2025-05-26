#include "circular_buffer.h"

// 初始化缓冲区
CircularBuffer *createCircularBuffer(uint32_t size)
{
    if (!is_power_of_2(size)) {
        if (size < 2)
            size = 2;

        // 向上取2的次幂
        int i = 0;
        for (; size != 0; i++)
            size >>= 1;

        size = 1U << i;
    }

    if (size < 0)
        return NULL;

    CircularBuffer *ring_buf = (CircularBuffer *)calloc(1, sizeof(CircularBuffer));
    if (!ring_buf) {
        ERR("Failed to malloc memory,errno:%u,reason:%s", errno, strerror(errno));
        return NULL;
    }

    ring_buf->buffer = (void *)malloc(size);
    if (ring_buf->buffer) {
        net_free(ring_buf);
        return NULL;
    }

    ring_buf->size = size;
    ring_buf->in = 0;
    ring_buf->out = 0;
    pthread_mutex_init(&ring_buf->lock, NULL);

    return ring_buf;
}

// 释放缓冲区
void destroyCircularBuffer(CircularBuffer *ring_buf)
{
    if (ring_buf) {
        net_free(ring_buf->buffer);
        pthread_mutex_destroy(&ring_buf->lock);
        net_free(ring_buf);
    }
}

uint32_t getCircularBufferSize(CircularBuffer *ring_buf)
{
    if (!ring_buf)
        return 0;

    uint32_t len = 0;
    pthread_mutex_lock(&ring_buf->lock);
    len = ring_buf->in - ring_buf->out;
    pthread_mutex_unlock(&ring_buf->lock);
    return len;
}

uint32_t readFromCircularBuffer(CircularBuffer *ring_buf, void *buffer, uint32_t size)
{
    if (!ring_buf || !buffer)
        return 0;

    pthread_mutex_lock(&ring_buf->lock);

    uint32_t len = 0;
    size = min(size, ring_buf->in - ring_buf->out);
    len = min(size, ring_buf->size - (ring_buf->out & (ring_buf->size - 1)));
    memcpy(buffer, ring_buf->buffer + (ring_buf->out & (ring_buf->size - 1)), len);
    memcpy(buffer + len, ring_buf->buffer, size - len);
    ring_buf->out += size;

    if (ring_buf->in == ring_buf->out)
        ring_buf->in = ring_buf->out = 0;
        
    pthread_mutex_unlock(&ring_buf->lock);
    return size;
}

uint32_t writeToCircularBuffer(CircularBuffer *ring_buf, void *buffer, uint32_t size)
{
    if (ring_buf == NULL || buffer == NULL)
        return 0;

    pthread_mutex_lock(&ring_buf->lock);

    uint32_t len = 0;
    size = min(size, ring_buf->size - ring_buf->in + ring_buf->out);

    len = min(size, ring_buf->size - (ring_buf->in & (ring_buf->size - 1)));

    memcpy(ring_buf->buffer + (ring_buf->in & (ring_buf->size - 1)), buffer, len);

    memcpy(ring_buf->buffer, buffer + len, size - len);

    ring_buf->in += size;

    pthread_mutex_unlock(&ring_buf->lock);
    return size;
}
