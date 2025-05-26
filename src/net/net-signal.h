#ifndef __NET_SIGNAL_H__
#define __NET_SIGNAL_H__

#include "net-common.h"

inline static SOCKET createSignal(void)
{
    int signalFd = eventfd(0, 0);
    if (signalFd == -1) {
        ERR("eventfd failed %d", errno);
        return NET_FAIL;
    }
    return signalFd;
}

inline static int sendSignal(SOCKET signal_fd)
{
    assert(signal_fd > 0);

    uint64_t notify = '1';
    if (sizeof(uint64_t) != write(signal_fd, &notify, sizeof(uint64_t))) {
        ERR("write error");
        return NET_FAIL;
    }   
    return NET_SUCCESS;
}

inline static int recvSignal(SOCKET signal_fd, void *arg)
{
    assert(signal_fd > 0);
        
    uint64_t notify;
    if (sizeof(uint64_t) != read(signal_fd, &notify, sizeof(uint64_t))) {
        ERR("read notify failed %d", errno);
        return NET_FAIL;
    }
    return NET_SUCCESS;
}

inline static void closeSignal(SOCKET signal_fd)
{
    if (signal_fd > 0)
        close(signal_fd);
}

#endif // !__NET_SIGNAL_H__
