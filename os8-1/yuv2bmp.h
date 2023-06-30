#ifndef YUV_TO_BMP_H
#define YUV_TO_BMP_H

#include <stdio.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define BM 19778

unsigned char * yuv_y;
unsigned char * yuv_u;
unsigned char * yuv_v;
unsigned char * rgb_buf;

int fn_Yuv_To_Bmp(const char * yuv_path);
int fn_Open_Yuv(const char * yuv_path);
int fn_Convert_Yuv();
int fn_Make_Bmp();

//bmp头文件信息结构体
struct bitmap_header//文件头 -->14个字节
{
    unsigned short	type; //文件类型，必须为BM
    unsigned int  size; // 位图文件大小
    unsigned short reserved1; //预留位
    unsigned short reserved2; //预留位
    unsigned int offbits; // bmp图像文件头数据偏移量(填54)
}__attribute__((packed));//忽略该结构体地址对齐

struct bitmap_info//像素头 --》40个字节
{
    unsigned int size; // 本结构大小
    unsigned int width; //像素点宽度
    unsigned int height; //像素点高度
    unsigned short planes;//目标设备的级别，必须为1

    unsigned short bit_count; // 色深每个像素点所占的位数24bit
    unsigned int compression; //是否压缩，0表示不压缩
    unsigned int size_img; // bmp数据大小，必须是4的整数倍
    unsigned int X_pel;//位图水平分辨率
    unsigned int Y_pel;//位图垂直分辨率
    unsigned int clrused;//位图实际使用的颜色表中的颜色数(24位位图 = 0)
    unsigned int clrImportant;//位图显示过程中重要的颜色数(24位位图 = 0)
}__attribute__((packed));

#endif

