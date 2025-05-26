#include "net-common.h"
#include "shm-cache-mmap.h"

int main()
{
    shm_ptr sp = shm_mmap_init(SHM_NAME, 1024, WRITE);
    if (sp == NULL)
        return -1;

    shm_mmap_write(sp, (uint8_t *)"wumighui si handson", sizeof("wumighui si handson"));
    
    return 0;
}
