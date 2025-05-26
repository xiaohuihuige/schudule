#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "net-tcpsocket.h"
#include "net-common.h"

int main() {

    SOCKET sockfd = CreateTcpSocket();
    if (sockfd <= -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    if (0 < Connect(sockfd, DEFAULT_IP, SERVER_PORT, 100)) {
        close(sockfd);
        ERR("Connect error");
        exit(EXIT_FAILURE);
    }
    
    // 发送数据给服务器
    char *message = "Hello, server!";

    for (int i = 0; i < 100000; i++) {
        if (send(sockfd, message, strlen(message), 0) == -1) {
            perror("send");
            ERR("send error");
            close(sockfd);
            exit(EXIT_FAILURE);
        }
    }

    // 接收服务器的响应
    char buffer[1024] = {0};
    ssize_t num_bytes = recv(sockfd, buffer, sizeof(buffer), 0);
    if (num_bytes == -1) {
        perror("recv");
        close(sockfd);
        ERR("recv error");
        exit(EXIT_FAILURE);
    }

    // 打印服务器的响应
    printf("Received message from server: %s\n", buffer);

    // 关闭套接字
    close(sockfd);

    return 0;
}
