#include "net-common.h"
#include "net-tcp-server.h"

int rtmp_revc_msg(void *conn)
{
    //ERR("recv size:%d, msg:%s", readable_bytes(buffer), begin_read(buffer));
    return NET_SUCCESS;
}

int main()
{
    server_ptr server = tcp_start_server("0.0.0.0", 1935, NULL, NULL, rtmp_revc_msg, NULL);
    while (1)
    {
        sleep(1);
    }
    tcp_close_all_connection(server);
}
