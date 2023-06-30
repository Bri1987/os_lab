#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/videodev2.h>
#include <jpeglib.h>
#include <unistd.h>

#define VIDEO_DEVICE "/dev/video0"
#define IMAGE_FILE "image.yuv"
#define JPEG_FILE "image.jpg"
#define FRAME_WIDTH 640
#define FRAME_HEIGHT 480

int xioctl(int fd, int request, void *arg) {
    int r;
    do {
        r = ioctl(fd, request, arg);
    } while (-1 == r && EINTR == errno);
    return r;
}

void save_yuv_to_file(unsigned char *data, int length) {
    FILE *file = fopen(IMAGE_FILE, "wb");
    if (!file) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
    fwrite(data, 1, length, file);
    fclose(file);
}

void yuv_to_jpeg(unsigned char *yuv_data, int width, int height) {
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    FILE *outfile;
    JSAMPROW row_pointer[1];
    int row_stride;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);

    if ((outfile = fopen(JPEG_FILE, "wb")) == NULL) {
        fprintf(stderr, "Error opening jpeg file %s\n", JPEG_FILE);
        exit(EXIT_FAILURE);
    }
    jpeg_stdio_dest(&cinfo, outfile);

    cinfo.image_width = width;
    cinfo.image_height = height;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_YCbCr;

    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, 75, TRUE);
    jpeg_start_compress(&cinfo, TRUE);

    row_stride = width * 3;
    unsigned char *rgb_data = malloc(row_stride * height);

    while (cinfo.next_scanline < cinfo.image_height) {
        unsigned char *yuv_pointer = &yuv_data[cinfo.next_scanline * row_stride];
        int i;
        for (i = 0; i < row_stride; i += 6) {
            unsigned char y = yuv_pointer[0];
            unsigned char u = yuv_pointer[1];
            unsigned char v = yuv_pointer[2];

            int c = y - 16;
            int d = u - 128;
            int e = v - 128;

            rgb_data[i] = (298 * c + 409 * e + 128) >> 8;     // R
            rgb_data[i + 1] = (298 * c - 100 * d - 208 * e + 128) >> 8; // G
            rgb_data[i + 2] = (298 * c + 516 * d + 128) >> 8;     // B

            y = yuv_pointer[3];
            c = y - 16;

            rgb_data[i + 3] = (298 * c + 409 * e + 128) >> 8;     // R
            rgb_data[i + 4] = (298 * c - 100 * d - 208 * e + 128) >> 8; // G
            rgb_data[i + 5] = (298 * c + 516 * d + 128) >> 8;     // B

            yuv_pointer += 4;
        }
        row_pointer[0] = &rgb_data[cinfo.next_scanline * row_stride];
        jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }

    jpeg_finish_compress(&cinfo);
    fclose(outfile);
    jpeg_destroy_compress(&cinfo);
    free(rgb_data);
}

int main() {
    struct v4l2_format fmt;
    struct v4l2_buffer buf;
    struct v4l2_requestbuffers req;
    enum v4l2_buf_type type;
    int fd, i;

    fd = open(VIDEO_DEVICE, O_RDWR);
    if (fd == -1) {
        perror("Error opening video device");
        exit(EXIT_FAILURE);
    }

    memset(&fmt, 0, sizeof(fmt));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = FRAME_WIDTH;
    fmt.fmt.pix.height = FRAME_HEIGHT;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUV422P;
    fmt.fmt.pix.field = V4L2_FIELD_NONE;

    if (-1 == xioctl(fd, VIDIOC_S_FMT, &fmt)) {
        perror("Error setting pixel format");
        exit(EXIT_FAILURE);
    }

    memset(&req, 0, sizeof(req));
    req.count = 1;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if (-1 == xioctl(fd, VIDIOC_REQBUFS, &req)) {
        perror("Error requesting buffer");
        exit(EXIT_FAILURE);
    }

    memset(&buf, 0, sizeof(buf));
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = 0;

    if (-1 == xioctl(fd, VIDIOC_QUERYBUF, &buf)) {
        perror("Error querying buffer");
        exit(EXIT_FAILURE);
    }

    void *buffer_start = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);

    if (MAP_FAILED == buffer_start) {
        perror("Error mapping memory");
        exit(EXIT_FAILURE);
    }

    memset(buffer_start, 0, buf.length);

    if (-1 == xioctl(fd, VIDIOC_QBUF, &buf)) {
        perror("Error queueing buffer");
        exit(EXIT_FAILURE);
    }

    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == xioctl(fd, VIDIOC_STREAMON, &type)) {
        perror("Error starting capture");
        exit(EXIT_FAILURE);
    }

    if (-1 == xioctl(fd, VIDIOC_DQBUF, &buf)) {
        perror("Error dequeuing buffer");
        exit(EXIT_FAILURE);
    }

    save_yuv_to_file(buffer_start, buf.length);
    yuv_to_jpeg(buffer_start, FRAME_WIDTH, FRAME_HEIGHT);

    printf("YUV image saved to %s\n", IMAGE_FILE);
    printf("JPEG image saved to %s\n", JPEG_FILE);

    if (-1 == xioctl(fd, VIDIOC_QBUF, &buf)) {
        perror("Error queueing buffer");
        exit(EXIT_FAILURE);
    }

    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == xioctl(fd, VIDIOC_STREAMOFF, &type)) {
        perror("Error stopping capture");
        exit(EXIT_FAILURE);
    }

    munmap(buffer_start, buf.length);
    close(fd);

    return 0;
}
