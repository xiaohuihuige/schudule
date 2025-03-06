#include "tcp_server.h"

static int _recvTcpBuffer(int fd, void *args)
{
    assert(args);

    Buffer * buffer = MALLOC(Buffer, sizeof(Buffer) + REVC_MTU);
    int size = Read(fd, buffer->data, REVC_MTU);
    if (size <= 0) {
       return NET_FAIL; 
    }
    return NET_SUCCESS;

}

static int _createTcpConnection(int fd, void *args)
{
    assert(args);

   
    return NET_SUCCESS;
}

TcpServer *createTcpServer(const char *ip, int port)
{
    assert(ip);

    TcpServer *tcps = CALLOC(1, TcpServer);
    if (!tcps)
        return NULL;

    tcps->port = port;
    strncpy(tcps->ip, ip, strlen(ip));
    
    do {

        tcps->connects = createFifiQueue();
        if (!tcps->connects)
            break;

        tcps->fd = CreateTcpSocket();
        if (tcps->fd <= 0)
            break;

        SetReuseAddr(tcps->fd);
        SetReusePort(tcps->fd);
        SetNonBlock(tcps->fd);
        
        int ret = bindTcpSocket(tcps->fd, tcps->ip, tcps->port);
        if (ret <= -1)
            break;

        ret = listenTcpSocket(tcps->fd, 1024);
        if (ret <= -1)
            break;

        tcps->scher = createTaskScheduler();
        if (!tcps->scher)
            break;

        tcps->ev = createReader(tcps->scher, tcps->fd, _createTcpConnection, (void *)tcps); 
        if (!tcps->ev)
            break;

    } while (0);

    return tcps;
}

void destroyTcpServer(TcpServer *tcps)
{

}
