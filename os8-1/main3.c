#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <sys/mman.h>
#include <unistd.h>
#include <wand/magick_wand.h>
#include <wand/pixel-wand.h>

#define DEVICE "/dev/video0" // 摄像头设备文件
#define WIDTH 640            // 图像宽度
#define HEIGHT 480           // 图像高度
#define FRAME_NUM 10          // 收集的帧数
#define BUFFER_NUM 10         // 缓冲区数量
#define DELAY_TIME 5        // 每帧之间的延迟时间，单位为毫秒

struct buffer {
    void *start;
    size_t length;
};

void yuv_to_rgb(unsigned const char *yuv_image, unsigned char *rgb_image, int width, int height)
{
    int n = width * height;
    int c = 0;
    int i = 0;
    for (; i < n; ++i) {
        int y = yuv_image[i];
        int u = yuv_image[n + (i >> 1)];
        int v = yuv_image[n + (i >> 1) + (n >> 2)];
        int r = y + ((360 * (v - 128)) >> 8);
        int g = y - ((88 * (u - 128) + 184 * (v - 128)) >> 8);
        int b = y + ((454 * (u - 128)) >> 8);
        rgb_image[c++] = r < 0 ? 0 : (r > 255 ? 255 : r);
        rgb_image[c++] = g < 0 ? 0 : (g > 255 ? 255 : g);
        rgb_image[c++] = b < 0 ? 0 : (b > 255 ? 255 : b);
    }
}

void add_frame_to_gif(MagickWand *wand, unsigned char *rgb_image, int width, int height)
{
    // 创建一个新 的像素颜色对象，用于设置背景色
    PixelWand *pixel_wand = NewPixelWand();
    PixelSetColor(pixel_wand, "white");

    // 创建一个新的 MagickWand 对象，并将 RGB 像素数据导入到其中
    MagickWand *frame_wand = NewMagickWand();
    MagickNewImage(frame_wand, width, height, pixel_wand);
    MagickSetImageBackgroundColor(frame_wand, pixel_wand);
    MagickImportImagePixels(frame_wand, 0, 0, width, height, "RGB", CharPixel, rgb_image);

    // 添加帧到 GIF 图像中，并设置延迟时间
    MagickAddImage(wand, frame_wand);
    MagickSetImageDelay(wand, DELAY_TIME);

    // 销毁临时对象
    DestroyMagickWand(frame_wand);
    DestroyPixelWand(pixel_wand);
}

int main()
{
    int fd, i;
    struct v4l2_capability cap;      // 视频设备功能信息
    struct v4l2_format fmt;          // 视频格式信息
    struct v4l2_requestbuffers req;  // 请求分配缓冲区
    struct v4l2_buffer buf;          // 缓冲区信息
    struct timeval tv;               // 超时时间
    fd_set fds;                      // 文件描述符集合

    // 初始化 ImageMagick 库
    MagickWandGenesis();

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
    printf("支持的 IO 操作：%x\n", cap.capabilities);

// set fmt
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
    fmt.fmt.pix.height = HEIGHT;
    fmt.fmt.pix.width = WIDTH;
    fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;

    if (ioctl(fd, VIDIOC_S_FMT, &fmt) == -1)
    {
        printf("Unable to set format\n");
        exit(EXIT_FAILURE);
    }
    if (ioctl(fd, VIDIOC_G_FMT, &fmt) == -1)
    {
        printf("Unable to get format\n");
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

// 分配内存映射缓冲区
    struct buffer *buffers = calloc(req.count, sizeof(*buffers));
    for (i = 0; i < req.count; ++i) {
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;
        if (ioctl(fd, VIDIOC_QUERYBUF, &buf) == -1) {
            perror("无法查询缓冲区信息");
            exit(EXIT_FAILURE);
        }
        buffers[i].length = buf.length;
        buffers[i].start = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);
        if (buffers[i].start == MAP_FAILED) {
            perror("无法映射缓冲区");
            exit(EXIT_FAILURE);
        }
    }

// 将内存映射缓冲区放入输入队列
    for (i = 0; i < req.count; ++i) {
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;
        if (ioctl(fd, VIDIOC_QBUF, &buf) == -1) {
            perror("无法将缓冲区放入输入队列");
            exit(EXIT_FAILURE);
        }
    }

// 启动视频采集
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(fd, VIDIOC_STREAMON, &type) == -1) {
        perror("无法启动视频采集");
        exit(EXIT_FAILURE);
    }

// 创建一个新的 MagickWand 对象，用于生成 GIF 动画
    MagickWand *wand = NewMagickWand();

// 收集多帧图像并转换为 RGB 格式，添加到 GIF 动画中
    for (i = 0; i < FRAME_NUM; ++i) {
        // 等待下一帧图像
        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        tv.tv_sec = 2;
        tv.tv_usec = 0;
        int r = select(fd + 1, &fds, NULL, NULL, &tv);
        if (r == -1) {
            perror("select()");
            exit(EXIT_FAILURE);
        } else if (r == 0) {
            fprintf(stderr, "超时等待摄像头响应\n");
            exit(EXIT_FAILURE);
        }

        // 从输出队列中取出缓冲区
        memset(&buf, 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        if (ioctl(fd, VIDIOC_DQBUF, &buf) == -1) {
            perror("无法取出缓冲区");
            exit(EXIT_FAILURE);
        }

        //------------------yuv
        const char *yuv = "/home/moomoo/img/img";
        for (; i < req.count; i++)
        {
            char pic_name[80];
            sprintf(pic_name, "%s_%d.yuv", yuv, i);
            FILE *yuyv_fd = fopen(pic_name, "wb");
            //使用 fwrite 函数将位于 buffers[i].start 地址处的图像数据写入打开的文件中。640 * 480 * 2 表示图像数据的总字节数，其中 640 和 480 分别代表图像的宽度和高度，2 表示每个像素占用两个字节。
            fwrite(buffers[i].start, 640 * 480 * 2, 1, yuyv_fd);
            printf("save %s_%d.yuv OK \n", yuv, i);
            fclose(yuyv_fd);
        }
        //------------------yuv

        // 将 YUV 格式的图像数据转换为 RGB 格式，以便后续处理
        unsigned char *rgb_image = malloc(WIDTH * HEIGHT * 3);
        yuv_to_rgb(buffers[buf.index].start, rgb_image, WIDTH, HEIGHT);

        // 添加 RGB 格式的图像帧到 GIF 动画中
        add_frame_to_gif(wand, rgb_image, WIDTH, HEIGHT);

        // 释放 RGB 格式的图像数据内存
        free(rgb_image);

        // 将内存映射缓冲区放回输入队列
        if (ioctl(fd, VIDIOC_QBUF, &buf) == -1) {
            perror("无法将缓冲区放回输入队列");
            exit(EXIT_FAILURE);
        }
    }

// 停止视频采集
    if (ioctl(fd, VIDIOC_STREAMOFF, &type) == -1) {
        perror("无法停止视频采集");
        exit(EXIT_FAILURE);
    }

// 保存 GIF 动画到文件中
    if (MagickWriteImages(wand, "output.gif", MagickTrue) == MagickFalse) {
        fprintf(stderr, "无法保存 GIF 动画\n");
        exit(EXIT_FAILURE);
    }

// 释放缓冲区内存映射和 MagickWand 对象
    for (i = 0; i < req.count; ++i) {
        munmap(buffers[i].start, buffers[i].length);
    }
    free(buffers);
    DestroyMagickWand(wand);

// 终止 ImageMagick 库
    MagickWandTerminus();

// 关闭摄像头设备
    close(fd);

    return 0;
}