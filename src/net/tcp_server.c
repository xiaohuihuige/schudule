#include "tcp_server.h"

static void _closeTcpConnection(Seesion *conn)
{
    assert(conn);

    if (conn->tcps->func && conn->tcps->func->uinit) 
        conn->tcps->func->uinit(conn->args);
    
    if (conn->ev)
        deleteReader(conn->ev);

    if (conn->fd > 0)
        closeTcpSocket(conn->fd);

    LOG("close tcp connection %d, %p", conn->fd, conn);

    return;
}

static void _closeAllTcpConnection(TcpServer *tcps)
{
    assert(tcps);
    if (!tcps->connects)
        return;

    FifoQueue *task_node = NULL;
    FifoQueue *temp_node = NULL;
    list_for_each_entry_safe(task_node, temp_node, &tcps->connects->list, list)
    {
        Seesion *conn = (Seesion *)task_node->task;
        if (!conn)
            continue;

        _closeTcpConnection(conn);

        deleteFifoQueueTask(task_node, Seesion);
    }

    FREE(tcps->connects);

    return;
}

static int _recvTcpBuffer(int fd, void *args)
{
    assert(args);

    Seesion *conn = (Seesion *)args;

    Buffer * buffer = createBuffer(REVC_MTU);
    int size = Read(fd, (char *)buffer->data, REVC_MTU);
    if (size <= 0) {
        _closeTcpConnection(conn);
        if (conn->tcps->connects) {
            MUTEX_LOCK(&conn->tcps->myMutex);
            FindDeleteFifoQueueTask(conn->tcps->connects, Seesion, (Seesion *)args);
            MUTEX_UNLOCK(&conn->tcps->myMutex);
            FREE(buffer);
        }
        return NET_FAIL; 
    }

    buffer->length = size;

    if (conn->tcps->func && conn->tcps->func->recv) 
        conn->tcps->func->recv(conn->args, buffer);

    FREE(buffer);

    return NET_SUCCESS;
}

static int _createTcpConnection(int fd, void *args)
{
    assert(args);

    Seesion *conn = CALLOC(1, Seesion);
    if (!conn) 
        return NET_FAIL;
   
    conn->fd = acceptTcpSocket(fd);
    if (conn->fd <= 0) {
        FREE(conn);
        return NET_FAIL;
    }

    SetNonBlock(conn->fd);
    SetSendBufSize(conn->fd, 1024 * 1024);
    SetKeepAlive(conn->fd);

    conn->tcps = (TcpServer *)args;

    if (conn->tcps->func && conn->tcps->func->init) {
        conn->args = conn->tcps->func->init(conn);
        if (!conn->args) 
            goto error;
    }

    conn->ev = createReader(conn->tcps->scher, conn->fd, _recvTcpBuffer, (void *)conn);
    if (!conn->ev) 
        goto error;

    LOG("new connection: %d, %p", conn->fd, conn);

    MUTEX_LOCK(&conn->tcps->myMutex);
    enqueue(conn->tcps->connects, (void *)conn);
    MUTEX_UNLOCK(&conn->tcps->myMutex);

    return NET_SUCCESS;

error:
    ERR("createReader fail %p", conn);

    _closeTcpConnection(conn);

    if (conn->tcps->connects) {
        MUTEX_LOCK(&conn->tcps->myMutex);
        FindDeleteFifoQueueTask(conn->tcps, Seesion, conn);
        MUTEX_UNLOCK(&conn->tcps->myMutex);
    }

    FREE(conn);

    return NET_FAIL;
}

void setTcpServerCallBack(TcpServer *tcps, 
                void *(*init)(Seesion *conn), 
                void (*recv)(void *seesion, Buffer *buffer), 
                void (*uinit)(void *seesion))
{
    assert(tcps);

    FREE(tcps->func);

    tcps->func = CALLOC(1, SeesionFunc);
    if (!tcps->func)
        return;

    tcps->func->init  = init;
    tcps->func->recv  = recv;
    tcps->func->uinit = uinit;

    return;
}

void setParentClassServer(TcpServer *tcps, void *parent)
{
    if (!tcps || !parent)
        return;

    FREE(tcps->parent);

    tcps->parent = parent;
}

TcpServer *createTcpServer(const char *ip, int port)
{
    assert(ip);

    TcpServer *tcps = CALLOC(1, TcpServer);
    if (!tcps)
        return NULL;

    tcps->port = port;
    snprintf(tcps->ip, sizeof(tcps->ip), "%s", ip);

    MUTEX_INIT(&tcps->myMutex);

    tcps->connects = createFifiQueue();
    if (!tcps->connects)
        goto error;

    tcps->fd = CreateServer(tcps->ip, tcps->port, 1024);
    if (tcps->fd <= 0)
        goto error;

    tcps->scher = createTaskScheduler();
    if (!tcps->scher)
        goto error;

    tcps->ev = createReader(tcps->scher, tcps->fd, _createTcpConnection, (void *)tcps); 
    if (!tcps->ev)
        goto error;

    LOG("start server %p, ip %s, port %d", tcps, tcps->ip, tcps->port);

    return tcps;

error:
    ERR("create server failed");
    destroyTcpServer(tcps);
    return NULL;
}

void destroyTcpServer(TcpServer *tcps)
{
    assert(tcps);

    MUTEX_LOCK(&tcps->myMutex);

    if (tcps->connects) 
        _closeAllTcpConnection(tcps);

    if (tcps->ev) {
        deleteReader(tcps->ev);
        tcps->ev = NULL;
    }

    if (tcps->fd > 0) {
        closeTcpSocket(tcps->fd);
        tcps->fd = -1;
    }
  
    if (tcps->scher) {
        destroyTaskScheduler(tcps->scher);
        tcps->scher = NULL;
    }
    
    MUTEX_UNLOCK(&tcps->myMutex);

    FREE(tcps->func);

    MUTEX_DESTROY(&tcps->myMutex);

    LOG("delete server %p", tcps);

    FREE(tcps);

    return;
}
