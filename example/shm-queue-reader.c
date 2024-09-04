#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include "shm-queue.h"
#include "shm-cache-mmap.h"

#define SHM_SIZE sizeof(shared_queue_t)

int main() 
{
    shm_ptr sp = shm_mmap_init(SHM_NAME, SHM_SIZE, WRITE);
    if (sp == NULL)
        return -1;

    shared_queue_t * queue = (shared_queue_t *)sp->shm_addr;

    // 消费者从队列中读取数据
    for (int i = 0; i < 20; ++i) {
        int item = queue_dequeue(queue);
        LOG("Consuming: %d", item);
        sleep(1);
    }

    queue_destroy(queue); 

    shm_mmap_unint(sp);

    return 0;
}
