#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

#define BUFFER_SIZE 1024

int main(int argc, char const *argv[]) {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};
    const char* exit_message = "exit";

    if (argc != 3) {
        printf("usage：%s <ip> <port>\n", argv[0]);
        return -1;
    }

    // 创建套接字
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("套接字创建失败\n");
        return -1;
    }

    // 设置服务器地址和端口
    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[2]));

    if (inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) <= 0) {
        printf("无效的服务器地址\n");
        return -1;
    }

    // 连接服务器
    int ret = connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if (ret < 0) {
        printf("连接服务器失败: %s\n", strerror(errno));
        return -1;
    }

    printf("已连接到服务器\n");

    // 通信循环
    while (1) {
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(STDIN_FILENO, &read_fds);
        FD_SET(sock, &read_fds);

        int max_fd = (STDIN_FILENO > sock ? STDIN_FILENO : sock) + 1;

        if (select(max_fd, &read_fds, NULL, NULL, NULL) == -1) {
            printf("select 失败\n");
            break;
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
            send(sock, buffer, strlen(buffer), 0);
        }

        // 读取服务器消息并显示
        if (FD_ISSET(sock, &read_fds)) {
            memset(buffer, 0, sizeof(buffer));
            if (recv(sock, buffer, sizeof(buffer), 0) <= 0) {
                printf("与服务器断开连接\n");
                break;
            }
            printf("收到消息：%s\n", buffer);
            if (strcmp(buffer, exit_message) == 0) {
                printf("服务器已断开连接\n");
                break;
            }
        }
    }

    close(sock);
    return 0;
}