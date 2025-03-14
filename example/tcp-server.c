#include "net-common.h"
#include "tcp_server.h"

int rtmp_revc_msg(void *conn)
{
    //ERR("recv size:%d, msg:%s", readable_bytes(buffer), begin_read(buffer));
    return NET_SUCCESS;
}

int main()
{
    TcpServer * server = createTcpServer(DEFAULT_IP, 1935);
    while (1)
    {
        sleep(1);
    }
    
    destroyTcpServer(server);
}
