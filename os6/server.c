#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 10002
#define BUFFER_SIZE 1024

int main(void) {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};
    const char* exit_message = "exit";

    // 创建服务器套接字
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // 设置端口重用
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("set sockopt failed");
        exit(EXIT_FAILURE);
    }

    // 绑定端口
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // 监听连接
    if (listen(server_fd, 1) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    printf("等待客户端连接...\n");

    // 接受连接请求
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        perror("accept failed");
        exit(EXIT_FAILURE);
    }

    printf("客户端已连接：%s:%d\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));

    // 通信循环
    while (1) {
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(new_socket, &read_fds);
        FD_SET(STDIN_FILENO, &read_fds);

        int max_fd = (new_socket > STDIN_FILENO ? new_socket : STDIN_FILENO) + 1;

        if (select(max_fd, &read_fds, NULL, NULL, NULL) == -1) {
            printf("select 失败\n");
            break;
        }

        // 读取客户端消息并显示
        if (FD_ISSET(new_socket, &read_fds)) {
            memset(buffer, 0, sizeof(buffer));
            if (recv(new_socket, buffer, sizeof(buffer), 0) <= 0) {
                printf("与客户端断开连接\n");
                break;
            }
            printf("收到消息：%s\n", buffer);
            if (strcmp(buffer, exit_message) == 0) {
                printf("客户端已断开连接\n");
                break;
            }
        }

        // 读取标准输入并发送消息
        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            memset(buffer, 0, sizeof(buffer));
            if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
                break;
            }
            if (strcmp(buffer, exit_message) == 0) {
                printf("已退出程序\n");
                break;
            }
            send(new_socket, buffer, strlen(buffer), 0);
        }
    }

    close(new_socket);
    close(server_fd);
    return 0;
}