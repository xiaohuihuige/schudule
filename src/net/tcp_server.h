#ifndef __TCP_SERVER_H__
#define __TCP_SERVER_H__

#include "net-common.h"
#include "net-schedule.h"

typedef struct TcpServer TcpServer;

typedef struct 
{
    void *(*init)(void *args);
    void (*recv)(void *args, void *buffer);
    void (*uinit)(void *args);
} SeesionFunc;

typedef struct 
{
    SOCKET fd;
    EpollEvent * ev;
    void *args;
    TcpServer *tcps;
} Seesion;

struct TcpServer 
{
    SOCKET fd;
    TaskScheduler * scher;
    EpollEvent * ev;
    char ip[256];
    int port;
    FifoQueue *connects;
    SeesionFunc *func;
    Mutex myMutex;
};

TcpServer *createTcpServer(const char *ip, int port);
void destroyTcpServer(TcpServer *tcps);
void setTcpServerCallBack(TcpServer *tcps, 
                        void *(*init)(void *args), 
                        void (*recv)(void *args, void *buffer), 
                        void (*uinit)(void *args));
#endif