#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <linux/types.h>
#include <linux/videodev2.h>

#include "header.h"

#define TRUE 1
#define FALSE 0

#define FILE_VIDEO "/dev/video0"

#define IMAGEWIDTH 640
#define IMAGEHEIGHT 480

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

int v4l2_grab(void)
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

int main(void)
{

    FILE *yuyv_fd;
    init_v4l2();
    v4l2_grab();
    int i = 0;
    for (; i < req.count; i++)
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

    close_v4l2();
    system("ffmpeg -s 640x480 -pix_fmt yuyv422 -i \"/home/moomoo/img/img_4.yuv\" -qscale:v 2 \"/home/moomoo/img/photo.jpg\"");

    system("cp /home/moomoo/img/photo.jpg /tmp/tmp.6fZhsAHmFr/");

    return (TRUE);
}
