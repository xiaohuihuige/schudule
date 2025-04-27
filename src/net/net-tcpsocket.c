#include "net-tcpsocket.h"

int getHostAddrs(const char *card, char *get_ip, size_t size)
{
    if (!card || !get_ip)
        return EXIT_FAILURE;

    struct ifaddrs *ifaddr, *ifa;
    char ip[INET_ADDRSTRLEN];

    // 获取网络接口地址
    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        return EXIT_FAILURE;
    }

    // 遍历所有接口
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) { // 只处理IPv4地址
        if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET) {
            if (inet_ntop(AF_INET, &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr, ip, sizeof(ip)) != NULL) {
                if (strcmp(ifa->ifa_name, card) == 0) {
                    snprintf(get_ip, size, "%s", ip); 
                    break;
                }
            } else {
                perror("inet_ntop");
            }
        }
    }
    freeifaddrs(ifaddr);
    return EXIT_SUCCESS;
}

void SetBlock(SOCKET fd, int write_timeout)
{
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags&(~O_NONBLOCK));

    if (write_timeout > 0)
    {
        struct timeval tv = { 
            write_timeout/1000, 
            ( write_timeout % 1000 )*1000 
        };
        setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (char*)&tv, sizeof tv);
	}
}

void SetNoDelay(SOCKET sockfd)
{
#ifdef TCP_NODELAY
    int on = 1;
    int ret = setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (char *)&on, sizeof(on));
#endif
}

void SetReuseAddr(SOCKET sockfd)
{
    int on = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof on);
}

void SetReusePort(SOCKET sockfd)
{
    int on = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, (const char*)&on, sizeof(on));
}

void SetNonBlock(SOCKET sockfd)
{
    int flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
}

void SetKeepAlive(SOCKET sockfd)
{
    int on = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, (char *)&on, sizeof(on));
}

void SetRecvBufSize(SOCKET sockfd, int size)
{
    setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, (char *)&size, sizeof(size));
}

void SetSendBufSize(SOCKET sockfd, int size)
{
    setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, (char *)&size, sizeof(size));
}

SOCKET CreateTcpSocket()
{
    SOCKET sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd <=0 )
    {
        return -1;
    } 
    return sockfd;
}

int bindTcpSocket(SOCKET sockfd, const char *ip, uint16_t port)
{
    struct sockaddr_in addr = {0};			  
	addr.sin_family = AF_INET;		  
	addr.sin_addr.s_addr = inet_addr(ip); 
	addr.sin_port = htons(port);  

    if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		ERR(" <socket=%d> bind <%s:%u> failed", sockfd, ip, port);
		return -1;
	}

    return 0;
}

int listenTcpSocket(SOCKET sockfd, int backlog)
{
    if (listen(sockfd, backlog) == -1) {
		ERR("<socket=%d> listen failed", sockfd);
		return -1;
	}

	return 0;
}

SOCKET acceptTcpSocket(SOCKET sockfd)
{
	struct sockaddr_in addr = {0};
	socklen_t addrlen = sizeof addr;
	int socket_fd = accept(sockfd, (struct sockaddr*)&addr, &addrlen);
	return socket_fd;
}

void closeTcpSocket(SOCKET sockfd)
{
    close(sockfd);
}

int GetPeerIp(SOCKET sockfd, char *ip, size_t size)
{
    struct sockaddr_in addr = { 0 };
    socklen_t addrlen = sizeof(struct sockaddr_in);
    if (getpeername(sockfd, (struct sockaddr *)&addr, &addrlen) == 0)
    {
        snprintf(ip, size, "%s", inet_ntoa(addr.sin_addr));
        return 0;
    }

    snprintf(ip, size, "%s", "0.0.0.0");

    return 0;
}

int GetSocketIp(SOCKET sockfd, char *ip, size_t size)
{
    struct sockaddr_in addr = {0};
    char str[INET_ADDRSTRLEN] = "127.0.0.1";
    if (GetSocketAddr(sockfd, &addr) == 0) {
        inet_ntop(AF_INET, &addr.sin_addr, str, sizeof(str));
    }
    snprintf(ip, size, "%s", str);
    return 0;
}

int GetSocketAddr(SOCKET sockfd, struct sockaddr_in* addr)
{
    socklen_t addrlen = sizeof(struct sockaddr_in);
    return getsockname(sockfd, (struct sockaddr*)addr, &addrlen);
}

uint16_t GetPeerPort(SOCKET sockfd)
{
    struct sockaddr_in addr = { 0 };
    socklen_t addrlen = sizeof(struct sockaddr_in);
    if (getpeername(sockfd, (struct sockaddr *)&addr, &addrlen) == 0)
    {
        return ntohs(addr.sin_port);
    }
    return 0;
}

int GetPeerAddr(SOCKET sockfd, struct sockaddr_in *addr)
{
    socklen_t addrlen = sizeof(struct sockaddr_in);
    return getpeername(sockfd, (struct sockaddr *)addr, &addrlen);
}

int errorReSend(SOCKET sockfd)
{
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
        // 资源暂时不可用，使用 select() 等待可写
        fd_set write_fds;
        FD_ZERO(&write_fds);
        FD_SET(sockfd, &write_fds);

        struct timeval timeout;
        timeout.tv_sec = 1; // 1秒超时
        timeout.tv_usec = 0;

        int result = select(sockfd + 1, NULL, &write_fds, NULL, &timeout);
        if (result > 0)
            return NET_SUCCESS;

        ERR("select() failed or timeout occurred...");

        return NET_FAIL;
    }
    return NET_SUCCESS;
}

int Connect(SOCKET sockfd, char *ip, uint16_t port, int timeout)
{
	int is_connected = 0;

	if (timeout > 0) {
		SetNonBlock(sockfd);
	}

	struct sockaddr_in addr = { 0 };
	socklen_t addrlen = sizeof(addr);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(ip);

	if (connect(sockfd, (struct sockaddr*)&addr, addrlen) == -1) {		
		if (timeout > 0) {
            is_connected = -1;
			fd_set fd_write;
			FD_ZERO(&fd_write);
			FD_SET(sockfd, &fd_write);
			struct timeval tv = { timeout / 1000, timeout % 1000 * 1000 };
			select((int)sockfd + 1, NULL, &fd_write, NULL, &tv);
			if (FD_ISSET(sockfd, &fd_write)) {
                is_connected = 0;
			}
			SetBlock(sockfd, 0);
		}
		else {
            is_connected = -1;
		}		
	}
	
	return is_connected;
}

int Read(SOCKET sockfd, char *data, uint32_t len)
{
    return recv(sockfd, data, len, 0);
}

int Fread(FILE *fp, char *data, uint32_t len)
{
    return fread(data, 1, len, fp);
}

SOCKET CreateServer(const char *ip, uint16_t port, int backlog)
{
    SOCKET fd = CreateTcpSocket();
    if (fd <= 0)
        return NET_FAIL;

    SetReuseAddr(fd);
    SetReusePort(fd);
    SetNonBlock(fd);

    int ret = bindTcpSocket(fd, ip, port);
    if (ret <= -1)
        return NET_FAIL;

    ret = listenTcpSocket(fd, backlog);
    if (ret <= -1)
        return NET_FAIL;

    return fd;
}
