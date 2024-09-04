#include "net-signal.h"

SOCKET net_create_signal(void)
{
    int inner_fd = eventfd(0, 0);
    if (inner_fd == -1)
    {
        ERR("eventfd failed %d", errno);
        return NET_FAIL;
    }
    return inner_fd;
}

int net_send_signal(SOCKET signal_fd)
{
    if (signal_fd <= 0)
        return NET_FAIL;

    uint64_t notify = '1';
    if (sizeof(uint64_t) != write(signal_fd, &notify, sizeof(uint64_t)))
    {
        ERR("write error");
        return NET_FAIL;
    }   
    return NET_SUCCESS;
}

int net_recv_signal(SOCKET signal_fd, void *arg)
{
    if (signal_fd <= 0)
        return NET_FAIL;
        
    uint64_t notify;
    if (sizeof(uint64_t) != read(signal_fd, &notify, sizeof(uint64_t)))
    {
        ERR("read notify failed %d", errno);
        return NET_FAIL;
    }
    return NET_SUCCESS;
}

void net_close_signal(SOCKET signal_fd)
{
    if (signal_fd > 0)
        close(signal_fd);
}