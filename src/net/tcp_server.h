#ifndef __TCP_SERVER_H__
#define __TCP_SERVER_H__

#include "net-common.h"
#include "net-schedule.h"
#include "buffer.h"

typedef struct TcpServer TcpServer;
typedef struct Seesion   Seesion;

typedef struct 
{
    void *(*init)(Seesion *conn);
    void (*recv)(void *seesion, Buffer *buffer);
    void (*uinit)(void *seesion);
} SeesionFunc;

struct Seesion
{
    SOCKET fd;
    EpollEvent * ev;
    void *args;
    TcpServer *tcps;
};

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
                        void *(*init)(Seesion *conn), 
                        void (*recv)(void *seesion, Buffer *buffer), 
                        void (*uinit)(void *seesion));
#endif
