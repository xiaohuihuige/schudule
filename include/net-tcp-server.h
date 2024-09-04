#ifndef __NET_TCP_SERVER_H__
#define __NET_TCP_SERVER_H__
#include "net-common.h"
#include "net-schedule.h"
#include "net-buffer.h"

typedef int (*recv_func)(void *user);
typedef void *(*init_func)(SOCKET fd, buffer_ptr buffer, sche_ptr scher);
typedef int (*deinit_func)(void *user);
typedef int (*stream_func)(void *user, uint8_t *data, int size, int type);

typedef struct 
{
    void *user;
    init_func   session_init;
    deinit_func session_deinit;
    stream_func session_stream;
    recv_func recvf;
} tcp_session, *session_ptr;

typedef struct 
{
    session_ptr session;
    SOCKET tcp_sockfd;
    ev_ptr ev;
    void *server;
    buffer_ptr buffer;
} tcp_connection_info, *connect_ptr;

typedef struct
{
    SOCKET tcp_sockfd;
    sche_ptr scher;
    ev_ptr ev;
    char ip[256];
    int port;
    task_list *connect_list;
    session_ptr session;
} tcp_server_info, *server_ptr;

server_ptr tcp_start_server(const char *ip, int port, init_func session_init, deinit_func session_deinit, stream_func session_stream, recv_func recvf);
void tcp_stop_server(server_ptr server);
void tcp_close_all_connection(server_ptr server);
void tcp_close_find_connection(server_ptr server, connect_ptr close_conn);
void tcp_push_stream_connection(server_ptr server, uint8_t *data, int size , int type);

#endif // !__NET_TCP_SERVER_H__
