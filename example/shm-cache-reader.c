#include "net-common.h"
#include "shm-cache-mmap.h"

int main()
{
    shm_ptr sp = shm_mmap_init(SHM_NAME, 1024, READER);
    if (sp == NULL)
        return -1;

    shm_mmap_reader(sp);
    
    shm_mmap_unint(sp); 
    return 0;
}
