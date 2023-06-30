#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define DEVICE_NAME "/dev/mychardev2"
#define BUF_LEN 1024

int main()
{
    int fd;
    char buf[BUF_LEN];
    if ((fd = open(DEVICE_NAME, O_RDWR)) < 0) {
        perror("open");
        exit(1);
    }
    printf("Device %s opened\n", DEVICE_NAME);
    if (ioctl(fd, 0) < 0) {
        perror("ioctl");
        exit(1);
    }
    printf("Device %s ioctl called\n", DEVICE_NAME);
    if (write(fd, "Hello, world!\n", 14) < 0) {
        perror("write");
        exit(1);
    }
    printf("Device %s write called\n", DEVICE_NAME);
    if (read(fd, buf, BUF_LEN) < 0) {
        perror("read");
        exit(1);
    }
    printf("Device %s read %s\n", DEVICE_NAME, buf);
    if (close(fd) < 0) {
        perror("close");
        exit(1);
    }
    printf("Device %s closed\n", DEVICE_NAME);
    return 0;
}