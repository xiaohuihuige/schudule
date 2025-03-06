#ifndef __TCP_SERVER_H__
#define __TCP_SERVER_H__

#include "net-common.h"
#include "net-schedule.h"

typedef struct 
{
    SOCKET fd;
    TaskScheduler * scher;
    EpollEvent * ev;
    char ip[256];
    int port;
    FifoQueue *connects;
} TcpServer;

TcpServer *createTcpServer(const char *ip, int port);
void  destroyTcpServer(TcpServer *tcps);

#endif