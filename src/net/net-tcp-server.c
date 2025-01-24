#include "net-tcp-server.h"

void tcp_close_connection(connect_ptr conn)
{
    if (conn == NULL)
        return;

    LOG("connect close %p", conn);

    if (conn->session && conn->session->session_init)
        conn->session->session_deinit(conn->session->user);

    if (conn->buffer)
        buffer_unint(conn->buffer);

    if (conn->ev)
        net_delete_reader(conn->ev);

    if (conn->tcp_sockfd > 0)
        closeTcpSocket(conn->tcp_sockfd);
}

void tcp_close_find_connection(server_ptr server, connect_ptr close_conn)
{
    if (server == NULL || server->connect_list == NULL)
        return;

    task_list *task_node = NULL;
    task_list *temp_node = NULL;
    list_for_each_entry_safe(task_node, temp_node, &server->connect_list->list, list)
    {
        connect_ptr conn = (connect_ptr)task_node->task;
        if (conn == NULL)
            continue;

        if (close_conn == conn)
        {
            tcp_close_connection(conn);
            net_task_list_del(task_node, tcp_connection_info);
        }
    }
}

void tcp_close_all_connection(server_ptr server)
{
    if (server == NULL || server->connect_list == NULL)
        return;

    task_list *task_node = NULL;
    task_list *temp_node = NULL;
    list_for_each_entry_safe(task_node, temp_node, &server->connect_list->list, list)
    {
        connect_ptr conn = (connect_ptr)task_node->task;
        if (conn == NULL)
            continue;

        tcp_close_connection(conn);

        net_task_list_del(task_node, tcp_connection_info);
    }
}

void tcp_push_stream_connection(server_ptr server, uint8_t *data, int size , int type)
{
    if (server == NULL || server->connect_list == NULL)
        return;

    task_list *task_node = NULL;
    task_list *temp_node = NULL;
    list_for_each_entry_safe(task_node, temp_node, &server->connect_list->list, list)
    {
        connect_ptr conn = (connect_ptr)task_node->task;
        if (conn == NULL)
            continue;

        // if (conn->session->session_stream)
        //     conn->session->session_stream(conn->session->user, data, size, type);
    }
}

int tcp_recv_msg(int fd, void *args)
{
    connect_ptr conn = (connect_ptr)args;
    if (conn == NULL)
        return NET_FAIL;

    server_ptr server = (server_ptr) conn->server;
    int size = buffer_read(conn->buffer, fd);
    if (size <= 0)
    {
        tcp_close_connection(conn);
        net_task_list_find_del(server->connect_list, tcp_connection_info, conn);
        return NET_FAIL;
    }

    if (conn->session && conn->session->recvf)
        conn->session->recvf(conn->session->user);

    return NET_SUCCESS;
}

int tcp_new_connection(int fd, void *args)
{
    server_ptr server = (server_ptr)args;
    if (server == NULL)
        return NET_FAIL;

    connect_ptr conn = (connect_ptr)calloc(1, sizeof(tcp_connection_info));
    if (conn == NULL)
        return NET_FAIL;

    conn->buffer = buffer_init(4096);
    if (conn->buffer == NULL)
    {
        net_free(conn);
        return NET_FAIL;
    }

    conn->tcp_sockfd = acceptTcpSocket(fd);
    if (conn->tcp_sockfd <= 0)
    {
        ERR("accept error");
        buffer_unint(conn->buffer);
        net_free(conn);
        return NET_FAIL;
    }

    conn->server = (void *)server;
    conn->session = server->session;
    if (conn->session && conn->session->session_init)
        conn->session->user = conn->session->session_init(conn->tcp_sockfd, conn->buffer, server->scher, server->gop);

    SetNonBlock(conn->tcp_sockfd);
    SetSendBufSize(conn->tcp_sockfd, 100 * 1024);
    SetKeepAlive(conn->tcp_sockfd);

    LOG("new connection: %d, %p", conn->tcp_sockfd, conn);

    conn->ev = net_create_reader(server->scher, conn->tcp_sockfd, tcp_recv_msg, (void *)conn);
    if (conn->ev == NULL)
    {
        ERR("net_create_reader error %p", conn);
        tcp_close_connection(conn);
        net_free(conn);
        return NET_FAIL;
    }

    net_task_list_add_tail(server->connect_list, (void *)conn);

    return NET_SUCCESS;
}

server_ptr tcp_start_server(const char *ip, int port, 
        init_func session_init, deinit_func session_deinit,recv_func recvf, void *gop)
{
    if (ip == NULL)
        return NULL;

    server_ptr server = (server_ptr)calloc(1, sizeof(tcp_server_info));
    if (server == NULL)
        return NULL;
    
    strncpy(server->ip, ip, strlen(ip));
    server->port    = port;

    server->session = (session_ptr)calloc(1, sizeof(tcp_session));
    if (server->session == NULL)
        return NULL;

    server->session->session_init   = session_init;
    server->session->session_deinit = session_deinit;
    server->session->recvf = recvf;
    server->gop = gop;
    
    server->connect_list = net_task_list_init();
    if (server->connect_list == NULL)
    {
        ERR("create server->connect_list error");
        goto error;
    }

    server->tcp_sockfd = CreateTcpSocket();
    if (server->tcp_sockfd < 0)
    {
        ERR("create socket error");
        goto error;
    }

    SetReuseAddr(server->tcp_sockfd);
    SetReusePort(server->tcp_sockfd);
    SetNonBlock(server->tcp_sockfd);

    int ret = bindTcpSocket(server->tcp_sockfd, server->ip, server->port);
    if (ret == -1)
    {
        ERR("bindTcpSocket error");
        goto error;
    }

    ret = listenTcpSocket(server->tcp_sockfd, 1024);
    if (ret == -1)
    {
        ERR("listenTcpSocket error");
        goto error;
    }

    server->scher = net_create_scheduler();
    if (server->scher == NULL)
    {
        ERR("create Scheduler");
        goto error;
    }

    server->ev = net_create_reader(server->scher, server->tcp_sockfd, tcp_new_connection, (void *)server);
    if (server->ev == NULL)
    {
        ERR("create createReader");
        goto error;
    }

    LOG("start server %p, ip %s, port %d", server, server->ip, server->port);

    return server;

error:
    tcp_stop_server(server);
    ERR("create server failed");
    return NULL;
}

void tcp_stop_server(server_ptr server)
{
    if (server == NULL)
        return;

    tcp_close_all_connection(server);

    if (server->ev)
        net_delete_reader(server->ev);

    if (server->tcp_sockfd > 0)
        closeTcpSocket(server->tcp_sockfd);
    
    net_free(server->session);
    
    if (server->scher)
        net_destroy_scheduler(server->scher);

    LOG("delete server %p", server);

    net_free(server);

    return;
}
