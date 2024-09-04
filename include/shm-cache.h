#ifndef __SHM_CACHE_H__
#define __SHM_CACHE_H__
#include "net-common.h"
#include "net-task.h"

typedef struct 
{
    int type;
    int size;
    uint8_t payload[];
} cache_buffer;

#define sizeof_buffer(size)  (sizeof(cache_buffer) + sizeof(uint8_t) * (size))

typedef struct 
{
    int start;
    int end;
    int size;
} cache_record, *record_ptr;

typedef struct 
{
    int read_index;
    int write_index;
    uint8_t *memory;
    uint32_t size;
    task_list *record_list;
    pthread_mutex_t lock;
} shm_cache_info, *shm_cache_ptr;

shm_cache_ptr shm_cache_init(uint32_t size);
void shm_cache_unint(shm_cache_ptr cache);

int shm_cache_put(shm_cache_ptr cache, uint8_t *data, int size, int type);
cache_buffer *shm_cache_get(shm_cache_ptr cache);


#endif // !__SHM_CACHE_H__
