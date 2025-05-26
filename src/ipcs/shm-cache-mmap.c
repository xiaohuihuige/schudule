#include "shm-cache-mmap.h"

shm_ptr shm_mmap_init(const char *shm_name, int size, int flags)
{
    int code = NET_FAIL;
    shm_ptr sp = NULL;
    do {
        sp = CALLOC(1, shm_mmap);
        if (sp == NULL)
            break;

        if (flags == WRITE)
        {
            sp->shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
            if (sp->shm_fd == -1)
                break;
            
            if (ftruncate(sp->shm_fd, size) == -1) 
                break;
        } else 
        {
            sp->shm_fd = shm_open(shm_name, O_RDWR, 0666);
            if (sp->shm_fd == -1)
                break;
        }

        sp->shm_name = shm_name;
        sp->shm_size = size;
        sp->flags    = flags;
        sp->shm_addr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, sp->shm_fd, 0);
        if (sp->shm_addr == MAP_FAILED)
            break;

        code = NET_SUCCESS;
    } while(0);
    
    if (code != NET_SUCCESS)
        shm_mmap_unint(sp);

    return sp;
}

void shm_mmap_unint(shm_ptr sp)
{
    if (sp == NULL)
        return;

    if (sp->shm_addr)
        munmap(sp->shm_addr, sp->shm_size);

    if (sp->shm_fd > 0)
        close(sp->shm_fd);

    if (sp->flags == READER) 
        if (shm_unlink(sp->shm_name) == -1) 
            ERR("shm_unlink %s", sp->shm_name);

    sp->shm_addr = NULL;
    net_free(sp);
    return;
}

int shm_mmap_write(shm_ptr sp, uint8_t *data, int size)
{
    if (sp == NULL || sp->shm_addr == NULL || sp->shm_size < size)
        return NET_FAIL;
    
    memcpy(sp->shm_addr, data, size);
    
    return NET_SUCCESS;
}


int shm_mmap_reader(shm_ptr sp)
{
    LOG("data %s", sp->shm_addr);
    
    return NET_SUCCESS;
}

