#ifndef __TCP_SOCKET_H__
#define __TCP_SOCKET_H__

#include "net-common.h"

#define DEFAULT_IP "0.0.0.0"// 服务器IP地址
#define SERVER_PORT 1935      // 服务器端口号
#define NETWORK_CARD "ens33"
typedef int SOCKET;

int getHostAddrs(const char *card, char *get_ip, size_t size);
void SetReuseAddr(SOCKET sockfd);
void SetReusePort(SOCKET sockfd);
void SetNonBlock(SOCKET fd);
void SetBlock(SOCKET fd, int write_timeout);
void SetNoDelay(SOCKET sockfd);
void SetKeepAlive(SOCKET sockfd);
void SetRecvBufSize(SOCKET sockfd, int size);
void SetSendBufSize(SOCKET sockfd, int size);
int getSendBufSize(SOCKET sockfd);

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

int Read(SOCKET sockfd, char *data, uint32_t len);
int Fread(FILE *fp, char *data, uint32_t len);

SOCKET CreateServer(const char *ip, uint16_t port, int backlog);
int errorReSend(SOCKET sockfd);
int Send(SOCKET sockfd, uint8_t *data, int len, int time);

#endif // !__TCP_SOCKET_H__
