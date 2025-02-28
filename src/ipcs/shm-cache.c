#include "shm-cache.h"
#include "circular_buffer.h"

shm_cache_ptr shm_cache_init(uint32_t size)
{
    if (size < 0)
        return NULL;

    int code = NET_FAIL;
    shm_cache_ptr cache = NULL;

    do {
        cache = (shm_cache_ptr)calloc(1, sizeof(shm_cache_info));
        if (cache == NULL)
            break;

        cache->memory = (uint8_t *)calloc(1, size);
        if (cache->memory == NULL)
            break;

        cache->record_list = net_task_list_init();
        if (cache->record_list == NULL)
            break;

        cache->size        = size;
        cache->read_index  = 0;
        cache->write_index = 0;
        pthread_mutex_init(&cache->lock, NULL);
        code = NET_SUCCESS;
    } while(0);

    if (code != NET_SUCCESS)
        shm_cache_unint(cache);

    return cache;
}

void shm_cache_unint(shm_cache_ptr cache)
{
    if (cache)
    {
        pthread_mutex_destroy(&cache->lock);
        net_free(cache->memory);
        net_task_list_each_free(cache->record_list, cache_record);
        net_free(cache);
    }
}

static int shm_cache_get_distance(shm_cache_ptr cache)
{
    if (cache == NULL)
         return NET_FAIL;
    if (list_empty(&cache->record_list->list))
        return cache->size;
    if (cache->read_index > cache->write_index)
        return cache->read_index - cache->write_index;
    else if (cache->read_index < cache->write_index)
        return (cache->size - cache->write_index + cache->read_index);
    return NET_SUCCESS;
}

static record_ptr shm_cache_get_next(task_list *head)
{   
    task_list *task_node = net_task_list_pop_head(head);
    if (task_node == NULL)
        return NULL;
        
    record_ptr record = (record_ptr)task_node->task;
    if (record == NULL)
        return NULL;

    net_free(task_node);

    return record;
}

static int _shm_cache_put(shm_cache_ptr cache, uint8_t *data, int size)
{
    if (cache == NULL)
         return NET_FAIL;

    int available_len = shm_cache_get_distance(cache);
    if (available_len < 0)
        return NET_FAIL;

    if (available_len >= size)
    {
        uint32_t left_over = size;
        uint32_t write_len = 0;

        record_ptr record = (record_ptr)calloc(1, sizeof(cache_record));
        if (record == NULL)
            return NET_FAIL;

        record->size = size;
        record->start = cache->write_index;
    
        write_len = min(size, cache->size - cache->write_index);
        memcpy(cache->memory + cache->write_index, data, write_len);
        left_over -= write_len;
        memcpy(cache->memory, data + write_len, left_over);
        if (left_over > 0)
        {
            cache->write_index = left_over;
        } else
        {
            cache->write_index += size;
        }
        record->end = cache->write_index;
        net_task_list_add_tail(cache->record_list, (void *)record);
    } else if (available_len < size)
    {
        record_ptr record = shm_cache_get_next(cache->record_list);
        if (record == NULL)
            return NET_FAIL;

        cache->read_index = record->end;
        net_free(record);
        return _shm_cache_put(cache, data, size);
    }
    return NET_SUCCESS;
}

static cache_buffer *_shm_cache_get(shm_cache_ptr cache)
{
    if (!cache)
        return NULL;
    
    record_ptr record = NULL;
    uint8_t *b = NULL;

    do {
        record = shm_cache_get_next(cache->record_list);
        if (!record)
            break;
        
        b = (uint8_t *)calloc(1, record->size);
        if (!b)
            break;
        
        if (record->start > record->end)
        {   
            memcpy(b, cache->memory + record->start, cache->size - record->start);
            memcpy(b + cache->size - record->start, cache->memory, record->end);
        } else 
            memcpy(b, cache->memory + record->start, record->size);
        
        cache->read_index = record->end;
    } while(0);

    net_free(record);

    return (cache_buffer *)b;
}

cache_buffer *shm_cache_get(shm_cache_ptr cache)
{
    pthread_mutex_lock(&cache->lock);
    cache_buffer *b = _shm_cache_get(cache);
    pthread_mutex_unlock(&cache->lock);
    return b;
}

int shm_cache_put(shm_cache_ptr cache, uint8_t *data, int size, int type)
{
    if (cache == NULL || data == NULL || size <= 0)
        return NET_FAIL;
    
    cache_buffer *b = (cache_buffer *)calloc(1, sizeof_buffer(size));
    if (b == NULL)
        return NET_FAIL;
    
    b->size = size;
    b->type = type;

    memcpy(b->payload, data, size);

    pthread_mutex_lock(&cache->lock);
    int code = _shm_cache_put(cache, (uint8_t *)b, sizeof_buffer(size));
    pthread_mutex_unlock(&cache->lock);

    net_free(b);
    return code;
}

