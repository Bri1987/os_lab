#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <sys/mman.h>
#include <unistd.h>
#include "yuv2bmp.h"

#define DEVICE "/dev/video0" // 摄像头设备文件
#define WIDTH 640            // 图像宽度
#define HEIGHT 480           // 图像高度
#define BUFFER_NUM 4         // 缓冲区数量

struct buffer {
    void *start;
    size_t length;
};

//----------------------

int fn_Yuv_To_Bmp(const char * yuv_path)
{
    //1.打开yuv文件，并获取Y，U，V分别放在不同的数组中
    int ret = fn_Open_Yuv(yuv_path);
    if(ret < 0)
    {
        printf("获取yuv像素点失败\n");
        return -1;
    }

    //2.转yuv为rgb
    ret = fn_Convert_Yuv();
    if(ret < 0)
    {
        printf("申请rgb空间失败\n");
        return -1;
    }
    else
    {
        printf("rgb像素点转换成功\n");
    }

    //3.创建bmp文件，处理文件头，并把rgb放进bmp里面
    ret = fn_Make_Bmp();
    if(ret < 0)
    {
        printf("创建bmp文件失败\n");
        return -1;
    }
    else
        printf("YUV转BMP成功\n");

    return 0;
}

int fn_Make_Bmp()
{
    int bmp_fd = open("./test.bmp" , O_RDWR | O_TRUNC | O_CREAT , 0777);
    if(bmp_fd <= 0)
    {
        perror("");
        return -1;
    }
    //处理bmp数据头
    struct bitmap_header head_info;
    memset(&head_info , 0 , 14);
    head_info.type = BM;
    head_info.size = HEIGHT * WIDTH * 3 + 54;
    head_info.offbits = 54;

    struct bitmap_info bmp_info;
    memset(&bmp_info , 0 , 40);
    bmp_info.size = 40;
    bmp_info.width = WIDTH;
    bmp_info.height = HEIGHT;
    bmp_info.planes = 1;
    bmp_info.bit_count = 24;
    bmp_info.size_img = HEIGHT * WIDTH * 3;
    bmp_info.X_pel = WIDTH;
    bmp_info.Y_pel = HEIGHT;

    //把数据头，像素点写入bmp文件中
    write(bmp_fd , &head_info , 14);
    write(bmp_fd , &bmp_info , 40);
    int ret = write(bmp_fd , rgb_buf , HEIGHT * WIDTH * 3);
    printf("ret = %d\n" , ret);

    return 0;
}

int fn_Convert_Yuv()
{
    rgb_buf = malloc(WIDTH * HEIGHT * 3);
    if(rgb_buf == NULL)
        return -1;

    int i = 0;
    int j = 0;
    for(i = 0 ; i < HEIGHT ; i++)
    {
        for(j = 0 ; j < WIDTH ; j++)
        {
            //r
            rgb_buf[i * WIDTH * 3 + j * 3] = (unsigned char)
                    ((float)yuv_y[(HEIGHT - i - 1) * WIDTH + j] +
                     1.403 * ((float)yuv_u[(((HEIGHT - i - 1) / 2) * (WIDTH / 2)  + (j / 2))] - 128));

            if(rgb_buf[i * WIDTH * 3 + j * 3] < 0)
            {
                rgb_buf[i * WIDTH * 3 + j * 3] = 0;
            }
            else if(rgb_buf[i * WIDTH * 3 + j * 3] > 255)
            {
                rgb_buf[i * WIDTH * 3 + j * 3] = 255;
            }

            //g
            rgb_buf[i * WIDTH * 3 + j * 3 + 1] = (unsigned char)
                    ((float)yuv_y[(HEIGHT - i - 1) * WIDTH + j] -
                     0.3455 * ((float)yuv_v[(((HEIGHT - i - 1) / 2) * (WIDTH / 2)  + (j / 2))] - 128) -
                     0.7169 * ((float)yuv_u[(((HEIGHT - i - 1) / 2) * (WIDTH / 2)  + (j / 2))] - 128));

            if(rgb_buf[i * WIDTH * 3 + j * 3 + 1] < 0)
            {
                rgb_buf[i * WIDTH * 3 + j * 3 + 1] = 0;
            }
            else if(rgb_buf[i * WIDTH * 3 + j * 3 + 1] > 255)
            {
                rgb_buf[i * WIDTH * 3 + j * 3 + 1] = 255;
            }

            //b
            rgb_buf[i * WIDTH * 3 + j * 3 + 2] = (unsigned char)
                    ((float)yuv_y[(HEIGHT - i - 1) * WIDTH + j] +
                     1.779 * ((float)yuv_v[(((HEIGHT - i - 1) / 2) * (WIDTH / 2)  + (j / 2))] - 128));

            if(rgb_buf[i * WIDTH * 3 + j * 3 + 2] < 0)
            {
                rgb_buf[i * WIDTH * 3 + j * 3 + 2] = 0;
            }
            else if(rgb_buf[i * WIDTH * 3 + j * 3 + 2] > 255)
            {
                rgb_buf[i * WIDTH * 3 + j * 3 + 2] = 255;
            }

        }
    }

    return 0;
}

int fn_Open_Yuv(const char * yuv_path)
{
    int yuv_fd = open(yuv_path , O_RDWR);
    yuv_y = malloc(WIDTH * HEIGHT);
    yuv_u = malloc(WIDTH / 2 * HEIGHT / 2);
    yuv_v = malloc(WIDTH / 2 * HEIGHT / 2);

    //读取yuv数据
    int ret = 0;
    ret = read(yuv_fd , yuv_y , WIDTH * HEIGHT);
    if(ret != WIDTH * HEIGHT)
    {
        printf("读取y数据失败\n");
        return -1;
    }
    ret = read(yuv_fd , yuv_u , WIDTH / 2 * HEIGHT / 2);
    if(ret != WIDTH / 2 * HEIGHT / 2)
    {
        printf("读取u数据失败\n");
        return -1;
    }
    ret = read(yuv_fd , yuv_v , WIDTH / 2 * HEIGHT / 2);
    if(ret != WIDTH / 2 * HEIGHT / 2)
    {
        printf("读取v数据失败\n");
        return -1;
    }

    close(yuv_fd);

    return 0;
}

//----------------------

int main(int argc, char **argv)
{
    int fd, i;
    struct v4l2_capability cap;      // 视频设备功能信息
    struct v4l2_format fmt;          // 视频格式信息
    struct v4l2_fmtdesc fmtdesc;
    struct v4l2_requestbuffers req;  // 请求分配缓冲区
    struct v4l2_buffer buf;          // 缓冲区信息
    struct timeval tv;               // 超时时间
    fd_set fds;                      // 文件描述符集合

    // 打开摄像头设备
    fd = open(DEVICE, O_RDWR);
    if (fd == -1) {
        perror("无法打开设备");
        exit(EXIT_FAILURE);
    }

    // 查询设备功能
    if (ioctl(fd, VIDIOC_QUERYCAP, &cap) == -1) {
        perror("无法查询设备功能");
        exit(EXIT_FAILURE);
    }
    printf("驱动名称：%s\n", cap.driver);
    printf("设备名称：%s\n", cap.card);
    printf("设备版本：%u.%u.%u\n", (cap.version >> 16) & 0xFF, (cap.version >> 8) & 0xFF, cap.version & 0xFF);

    memset(&fmtdesc, 0, sizeof(fmtdesc));
    fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    while (ioctl(fd, VIDIOC_ENUM_FMT, &fmtdesc) != -1) {
        printf("pixelformat: '%c%c%c%c', description: '%s'\n",
               fmtdesc.pixelformat & 0xFF, (fmtdesc.pixelformat >> 8) & 0xFF,
               (fmtdesc.pixelformat >> 16) & 0xFF, (fmtdesc.pixelformat >> 24) & 0xFF,
               fmtdesc.description);
        fmtdesc.index++;
    }
    if (errno != EINVAL) {
        perror("VIDIOC_ENUM_FMT");
        exit(EXIT_FAILURE);
    }

    // 查询并设置当前视频格式
    memset(&fmt, 0, sizeof(fmt));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(fd, VIDIOC_G_FMT, &fmt) == -1) {
        perror("无法查询当前视频格式");
        exit(EXIT_FAILURE);
    }
    printf("当前视频格式：宽度=%d，高度=%d，图像格式=%d\n", fmt.fmt.pix.width, fmt.fmt.pix.height, fmt.fmt.pix.pixelformat);
    fmt.fmt.pix.width = WIDTH;
    fmt.fmt.pix.height = HEIGHT;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV; // 指定采集的图像格式为YUV420P
    fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;
    if (ioctl(fd, VIDIOC_S_FMT, &fmt) == -1) {
        perror("无法设置视频格式");
        exit(EXIT_FAILURE);
    }

    // 请求分配缓冲区
    memset(&req, 0, sizeof(req));
    req.count = BUFFER_NUM;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
    if (ioctl(fd, VIDIOC_REQBUFS, &req) == -1) {
        perror("无法请求分配缓冲区");
        exit(EXIT_FAILURE);
    }

    // 映射缓冲区到用户空间
    struct buffer *buffers = malloc(sizeof(struct buffer) * BUFFER_NUM);
    for (i = 0; i < BUFFER_NUM; ++i) {
        memset(&buf, 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;
        if (ioctl(fd, VIDIOC_QUERYBUF, &buf) == -1) {
            perror("无法查询缓冲区信息");
            exit(EXIT_FAILURE);
        }
        buffers[i].length = buf.length;
        //printf("缓冲区长度is %d\n",buf.length);
        buffers[i].start = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);
        if (buffers[i].start == MAP_FAILED) {
            perror("无法映射缓冲区到用户空间");
            exit(EXIT_FAILURE);
        }
    }

    // 申请到的帧缓冲放入视频采集输出队列
    for (i = 0; i < BUFFER_NUM; ++i) {
        memset(&buf, 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;
        if (ioctl(fd, VIDIOC_QBUF, &buf) == -1) {
            perror("无法将缓冲区加入视频输出队列");
            exit(EXIT_FAILURE);
        }
    }

    // 开始录制
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(fd, VIDIOC_STREAMON, &type) == -1) {
        perror("无法开始录制");
        exit(EXIT_FAILURE);
    }

    // 等待摄像头数据准备好
    FD_ZERO(&fds);
    FD_SET(fd, &fds);
    tv.tv_sec = 2;
    tv.tv_usec = 0;

    int r = select(fd + 1, &fds, NULL, NULL, &tv);
    if (r == -1) {
        perror("select 失败");
        exit(EXIT_FAILURE);
    } else if (r == 0) {
        fprintf(stderr, "select 超时");
        exit(EXIT_FAILURE);
    }

    //从视频输出队列中取出一帧
    if (ioctl(fd, VIDIOC_DQBUF, &buf) == -1) {
        perror("无法将缓冲区出队");
        exit(EXIT_FAILURE);
    }

    //处理图像数据
    printf("已捕获一帧图像，长度为 %d 字节\n", buf.bytesused);

    //将捕获到的图像写入文件
    char filename[50];
    sprintf(filename, "frame%d.yuv", 0); // 以序号命名文件
    FILE *fp = fopen(filename, "wb");
    if (fp == NULL) {
        perror("无法创建输出文件");
        exit(EXIT_FAILURE);
    }
    fwrite(buffers[buf.index].start, HEIGHT*WIDTH*2, 1, fp);
    fclose(fp);

    //转换格式成bmp
    fn_Yuv_To_Bmp(filename);

    //停止录制
    if (ioctl(fd, VIDIOC_STREAMOFF, &type) == -1) {
        perror("无法停止录制");
        exit(EXIT_FAILURE);
    }

    //释放映射的内存
    for (i = 0; i < BUFFER_NUM; ++i) {
        if (munmap(buffers[i].start, buffers[i].length) == -1) {
            perror("无法释放映射的内存");
            exit(EXIT_FAILURE);
        }
    }

    //关闭设备
    if (close(fd) == -1) {
        perror("无法关闭设备");
        exit(EXIT_FAILURE);
    }

    return 0;
}