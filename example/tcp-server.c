#include "net-common.h"
#include "tcp_server.h"

void rtmp_revc_msg(Seesion *conn, Buffer *buffer)
{
    LOG("recv size:%d, %p, %s", buffer->length, conn, buffer->data);

    char *message = "Hello, server!";
    send(conn->fd, message, strlen(message), 0);
}

int main()
{
    TcpServer * server = createTcpServer(DEFAULT_IP, SERVER_PORT);

    setTcpServerCallBack(server, NULL, rtmp_revc_msg, NULL);

    while (1)
    {
        sleep(1);
    }
    
    destroyTcpServer(server);
}
