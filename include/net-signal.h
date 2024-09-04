#ifndef __NET_SIGNAL_H__
#define __NET_SIGNAL_H__

#include "net-common.h"

SOCKET net_create_signal(void);
int net_send_signal(SOCKET signal_fd);
int net_recv_signal(SOCKET signal_fd, void *arg);
void net_close_signal(SOCKET signal_fd);

#endif // !__NET_SIGNAL_H__
