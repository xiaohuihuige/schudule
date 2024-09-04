#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"  // 服务器IP地址
#define SERVER_PORT 1935      // 服务器端口号

int main() {
    // 创建套接字
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // 设置服务器地址结构体
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    // 连接服务器
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // 发送数据给服务器
    char *message = "Hello, server!";
    if (send(sockfd, message, strlen(message), 0) == -1) {
        perror("send");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // 接收服务器的响应
    char buffer[1024] = {0};
    ssize_t num_bytes = recv(sockfd, buffer, sizeof(buffer), 0);
    if (num_bytes == -1) {
        perror("recv");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // 打印服务器的响应
    printf("Received message from server: %s\n", buffer);

    // 关闭套接字
    close(sockfd);

    return 0;
}
