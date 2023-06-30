#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>
#include <sys/wait.h>

#define MAXBUF 1024
#define PORT 8083

void error_handling(char *message);
void send_file(int client_sock, char *filename, char *content_type,int photo_flag);

void removeFirstChar(char *str) {
    if (strlen(str) > 0) {
        memmove(str, str+1, strlen(str));
    }
}

int main(int argc, char *argv[]) {
    int serv_sock, client_sock;
    struct sockaddr_in serv_addr, client_addr;
    socklen_t client_addr_size;
    char buf[MAXBUF];
    int str_len, flags;

    serv_sock = socket(AF_INET, SOCK_STREAM, 0);
    if(serv_sock == -1) {
        error_handling("socket() error");
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(PORT);

    if(bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
        error_handling("bind() error");
    }

    if(listen(serv_sock, 5) == -1) {
        error_handling("listen() error");
    }

    printf("Web server is running on port %d...\n", PORT);

    while(1) {
        int photo_flag=0;
        client_addr_size = sizeof(client_addr);
        client_sock = accept(serv_sock, (struct sockaddr*)&client_addr, &client_addr_size);
        if(client_sock == -1) {
            error_handling("accept() error");
        }

        flags = fcntl(client_sock, F_GETFL, 0);
        fcntl(client_sock, F_SETFL, flags | O_NONBLOCK);

        while(1) {
            str_len = read(client_sock, buf, MAXBUF);
            if(str_len == -1 && errno != EAGAIN) {
                error_handling("read() error");
            } else if(str_len == 0) {
                break;
            } else if(str_len > 0) {
                buf[str_len] = '\0';
                printf("Received message from client: %s\n", buf);
                if(strstr(buf, "GET") != NULL) {
                    char *filename = strtok(buf + 4, " ");
                    char *extension = strrchr(filename, '.');
                    if(extension != NULL && (strcmp(extension, ".html") == 0 || strcmp(extension, ".jpg") == 0 || strcmp(extension, ".jpeg") == 0 || strcmp(extension, ".png") == 0 || strcmp(extension, ".bmp") == 0)) {
                        char *content_type = "text/html";
                        if(strstr(filename, ".jpg") != NULL || strstr(filename, ".jpeg") != NULL) {
                            printf("%s\n",filename);
                            content_type = "image/jpeg";
                            if(strcmp("/photo.jpg",filename)==0)
                                photo_flag=1;

                        } else if(strstr(filename, ".png") != NULL) {
                            content_type = "image/png";
                        }else if(strstr(filename, ".bmp") != NULL) {
                            content_type = "image/bmp";
                        }
                        send_file(client_sock, filename, content_type,photo_flag);
                    }
                    break;
                }
            }
        }

        close(client_sock);
    }

    close(serv_sock);
    return 0;
}

void error_handling(char *message) {
    perror(message);
    exit(1);
}

void send_file(int client_sock, char *filename, char *content_type,int photo_flag) {
    char buf[MAXBUF];
    struct stat file_stat;
    int fd, str_len;

    removeFirstChar(filename);
    //调用摄像头程序
    if(photo_flag)
    {
        pid_t pid=fork();
        if(pid==0)
        {
            char *args[] = {"./capture2", NULL}; // 填写可执行文件的路径及名称

            execvp(args[0], args); // 调用可执行文件
        }
        else if(pid>0)
        {
            wait(NULL);
        }
    }

    printf("%s\n",filename);
    if(stat(filename, &file_stat) == -1) {
        error_handling("stat() error");
    }

    printf("======file_name:%s\n",filename);
    fd = open(filename, O_RDONLY);
    if(fd == -1) {
        error_handling("open() error");
    }

    sprintf(buf, "HTTP/1.1 200 OK\r\nContent-Type: %s\r\nContent-Length: %ld\r\n\r\n", content_type, file_stat.st_size);
    str_len = write(client_sock, buf, strlen(buf));
    if(str_len == -1 && errno != EAGAIN) {
        error_handling("write() error");
    }

    while((str_len = read(fd, buf, MAXBUF)) > 0) {
        int send_len = 0;
        while(send_len < str_len) {
            int len = write(client_sock, buf + send_len, str_len - send_len);
            if(len == -1 && errno != EAGAIN) {
                error_handling("write() error");
            } else if(len > 0) {
                send_len += len;
            }
        }
    }

    close(fd);
}