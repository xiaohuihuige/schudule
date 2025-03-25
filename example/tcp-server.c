#include "net-common.h"
#include "tcp_server.h"

int main()
{
    TcpServer * server = createTcpServer(DEFAULT_IP, SERVER_PORT);

    setTcpServerCallBack(server, NULL, NULL, NULL);

    while (1)
    {
        sleep(1);
    }
    
    destroyTcpServer(server);
}
