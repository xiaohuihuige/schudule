#ifndef __TCP_SOCKET_H__
#define __TCP_SOCKET_H__

#include "net-common.h"

#define DEFAULT_IP "127.0.0.1"
typedef int SOCKET;

void SetReuseAddr(SOCKET sockfd);
void SetReusePort(SOCKET sockfd);
void SetNonBlock(SOCKET fd);
void SetBlock(SOCKET fd, int write_timeout);
void SetNoDelay(SOCKET sockfd);
void SetKeepAlive(SOCKET sockfd);
void SetRecvBufSize(SOCKET sockfd, int size);
void SetSendBufSize(SOCKET sockfd, int size);

SOCKET CreateTcpSocket();
int bindTcpSocket(SOCKET sockfd, const char *ip, uint16_t port);
int listenTcpSocket(SOCKET sockfd, int backlog);
SOCKET acceptTcpSocket(SOCKET sockfd);
void closeTcpSocket(SOCKET sockfd);

int GetSocketIp(SOCKET sockfd, char *ip, size_t size);
int GetPeerIp(SOCKET sockfd, char *ip, size_t size);

int GetSocketAddr(SOCKET sockfd, struct sockaddr_in *addr);
uint16_t GetPeerPort(SOCKET sockfd);
int GetPeerAddr(SOCKET sockfd, struct sockaddr_in *addr);
int Connect(SOCKET sockfd, char *ip, uint16_t port, int timeout);

#endif // !__TCP_SOCKET_H__
