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

#define TRUE 1
#define FALSE 0

#define FILE_VIDEO "/dev/video0"

#define IMAGEWIDTH 640
#define IMAGEHEIGHT 480
#define DELAY_TIME 5

static int fd;
const char *yuv = "/home/moomoo/img/img";
static struct v4l2_capability cap;
struct v4l2_fmtdesc fmtdesc;
struct v4l2_format fmt, fmtack;
struct v4l2_streamparm setfps;
struct v4l2_requestbuffers req;
struct v4l2_buffer buf;
enum v4l2_buf_type type;

struct buffer
{
    void *start;
    unsigned int length;
} *buffers;


void yuv_to_rgb(const unsigned char* yuv_image, unsigned char* rgb_image, int width, int height)
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

        r = (r < 0) ? 0 : ((r > 255) ? 255 : r);
        g = (g < 0) ? 0 : ((g > 255) ? 255 : g);
        b = (b < 0) ? 0 : ((b > 255) ? 255 : b);

        rgb_image[c++] = (unsigned char)r;
        rgb_image[c++] = (unsigned char)g;
        rgb_image[c++] = (unsigned char)b;
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

int init_v4l2(void)
{
    int i;
    int ret = 0;

    // opendev
    if ((fd = open(FILE_VIDEO, O_RDWR)) == -1)
    {
        printf("Error opening V4L interface\n");
        return (FALSE);
    }

    // 驱动设备的能力信息
    if (ioctl(fd, VIDIOC_QUERYCAP, &cap) == -1)
    {
        printf("Error opening device %s: unable to query device.\n", FILE_VIDEO);
        return (FALSE);
    }
    else
    {
        printf("driver:\t\t%s\n", cap.driver);
        printf("card:\t\t%s\n", cap.card);
        printf("bus_info:\t%s\n", cap.bus_info);
        printf("version:\t%d\n", cap.version);
        printf("capabilities:\t%x\n", cap.capabilities);

        if ((cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) == V4L2_CAP_VIDEO_CAPTURE)
        {
            printf("Device %s: supports capture.\n", FILE_VIDEO);
        }

        if ((cap.capabilities & V4L2_CAP_STREAMING) == V4L2_CAP_STREAMING)
        {
            printf("Device %s: supports streaming.\n", FILE_VIDEO);
        }
    }

    // 支持的type
    fmtdesc.index = 0;
    fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    printf("Support format:\n");
    while (ioctl(fd, VIDIOC_ENUM_FMT, &fmtdesc) != -1)
    {
        printf("\t%d.%s\n", fmtdesc.index + 1, fmtdesc.description);
        fmtdesc.index++;
    }

    // set fmt
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
    fmt.fmt.pix.height = IMAGEHEIGHT;
    fmt.fmt.pix.width = IMAGEWIDTH;
    fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;

    if (ioctl(fd, VIDIOC_S_FMT, &fmt) == -1)
    {
        printf("Unable to set format\n");
        return FALSE;
    }
    if (ioctl(fd, VIDIOC_G_FMT, &fmt) == -1)
    {
        printf("Unable to get format\n");
        return FALSE;
    }
    // 打印信息
    {
        printf("fmt.type:\t\t%d\n", fmt.type);
        printf("pix.pixelformat:\t%c%c%c%c\n", fmt.fmt.pix.pixelformat & 0xFF, (fmt.fmt.pix.pixelformat >> 8) & 0xFF, (fmt.fmt.pix.pixelformat >> 16) & 0xFF, (fmt.fmt.pix.pixelformat >> 24) & 0xFF);
        printf("pix.height:\t\t%d\n", fmt.fmt.pix.height);
        printf("pix.width:\t\t%d\n", fmt.fmt.pix.width);
        printf("pix.field:\t\t%d\n", fmt.fmt.pix.field);
    }

    // 设置帧率
    setfps.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    setfps.parm.capture.timeperframe.numerator = 10;
    setfps.parm.capture.timeperframe.denominator = 10;

    return TRUE;
}

int v4l2_grab(MagickWand *wand )
{
    unsigned int n_buffers;
    req.count = 10;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
    //用于请求视频设备的缓冲区
    if (ioctl(fd, VIDIOC_REQBUFS, &req) == -1)
    {
        printf("request for buffers error\n");
    }
    // mmap for buffers
    buffers = malloc(req.count * sizeof(*buffers));
    if (!buffers)
    {
        printf("Out of memory\n");
        return (FALSE);
    }

    printf("init %s \t[OK]\n", FILE_VIDEO);

    //遍历缓冲区
    for (n_buffers = 0; n_buffers < req.count; n_buffers++)
    {
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = n_buffers;
        // ll
        // ers
        //先进行了查询缓冲区的操作,然后在确定了缓冲区的长度和偏移量之后，才调用 mmap 函数进行内存映射,拿到buf.length和buf.m.offset
        if (ioctl(fd, VIDIOC_QUERYBUF, &buf) == -1)
        {
            printf("query buffer error\n");
            return (FALSE);
        }

        buffers[n_buffers].length = buf.length;
        // map
        //将视频设备的缓冲区映射到用户空间
        buffers[n_buffers].start = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);
        if (buffers[n_buffers].start == MAP_FAILED)
        {
            printf("buffer map error\n");
            return (FALSE);
        }
    }

    // queue
    //将预先准备好的视频缓冲区（Buffer）放入视频设备的输入队列中
    for (n_buffers = 0; n_buffers < req.count; n_buffers++)
    {
        buf.index = n_buffers;
        ioctl(fd, VIDIOC_QBUF, &buf);
    }

    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    //开始捕获视频数据
    ioctl(fd, VIDIOC_STREAMON, &type);

    //将指定索引的缓冲区从视频设备的输出队列中取出
    for (n_buffers = 0; n_buffers < req.count; n_buffers++)
    {
        buf.index = n_buffers;
        ioctl(fd, VIDIOC_DQBUF, &buf);

        // 将 YUV 格式的图像数据转换为 RGB 格式，以便后续处理
        unsigned char *rgb_image = malloc(IMAGEWIDTH * IMAGEHEIGHT * 3);
        yuv_to_rgb(buffers[buf.index].start, rgb_image, IMAGEWIDTH, IMAGEHEIGHT);

        // 添加 RGB 格式的图像帧到 GIF 动画中
        add_frame_to_gif(wand, rgb_image, IMAGEWIDTH, IMAGEHEIGHT);

        // 释放 RGB 格式的图像数据内存
        free(rgb_image);

        // 将内存映射缓冲区放回输入队列
        if (ioctl(fd, VIDIOC_QBUF, &buf) == -1) {
            perror("无法将缓冲区放回输入队列");
            exit(EXIT_FAILURE);
        }
    }

    printf("grab yuyv OK\n");
    return (TRUE);
}

int close_v4l2(void)
{
    if (fd != -1)
    {
        close(fd);
        return (TRUE);
    }
    return (FALSE);
}



int main()
{
    int i;
    struct timeval tv;               // 超时时间
    fd_set fds;                      // 文件描述符集合

    // 初始化 ImageMagick 库
    MagickWandGenesis();

// 创建一个新的 MagickWand 对象，用于生成 GIF 动画
    MagickWand *wand = NewMagickWand();

    FILE *yuyv_fd;
    init_v4l2();
    v4l2_grab(wand);
    for (i=0; i < req.count; i++)
    {
        char pic_name[80];
        sprintf(pic_name, "%s_%d.yuv", yuv, i);
        yuyv_fd = fopen(pic_name, "wb");
        if (!yuyv_fd)
        {
            printf("open %s_%d.yuv error\n", yuv, i);
            return (FALSE);
        }
        //使用 fwrite 函数将位于 buffers[i].start 地址处的图像数据写入打开的文件中。640 * 480 * 2 表示图像数据的总字节数，其中 640 和 480 分别代表图像的宽度和高度，2 表示每个像素占用两个字节。
        fwrite(buffers[i].start, 640 * 480 * 2, 1, yuyv_fd);
        printf("save %s_%d.yuv OK \n", yuv, i);
        fclose(yuyv_fd);
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