#ifndef __SHM_CACHE_MMAP_H__
#define __SHM_CACHE_MMAP_H__
#include "net-common.h"

#define SHM_NAME "/my_shm"

enum {
    WRITE  = 0,
    READER = 1,
};

typedef struct 
{
    const char *shm_name;
    int shm_fd;
    int shm_size;
    char *shm_addr;
    int flags;
} shm_mmap, *shm_ptr;

shm_ptr shm_mmap_init(const char *shm_name, int size, int flags);
void shm_mmap_unint(shm_ptr sp);

int shm_mmap_write(shm_ptr sp, uint8_t *data, int size);
int shm_mmap_reader(shm_ptr sp);

#endif
