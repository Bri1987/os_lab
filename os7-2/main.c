#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define MYCHARDEV3_IOCTL_CMD 0

int main(int argc, char *argv[])
{
    int fd;
    char *buf;
    size_t buf_len = 1000000; // 缓冲区大小
    ssize_t ret;

    buf = malloc(buf_len); // 动态分配缓冲区
    if (!buf) {
        perror("failed to allocate buffer");
        exit(EXIT_FAILURE);
    }

    fd = open("/dev/mychardev3", O_RDWR);
    if (fd < 0) {
        perror("failed to open device file");
        free(buf);
        exit(EXIT_FAILURE);
    }

    ret = ioctl(fd, MYCHARDEV3_IOCTL_CMD);
    if (ret == -1) {
        perror("failed to call ioctl command");
        free(buf);
        close(fd);
        exit(EXIT_FAILURE);
    }

    printf("finish ioctl\n");

    ret = read(fd, buf, buf_len);
    if (ret == -1) {
        perror("failed to read data from device file");
        free(buf);
        close(fd);
        exit(EXIT_FAILURE);
    }

    printf("%.*s\n", (int)ret, buf);

    free(buf);
    close(fd);
    return 0;
}