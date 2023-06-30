#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>
#include <libgen.h>

#define BUFFER_SIZE 1024
int force=0;      //是否强制覆盖，而不需要用户判读，默认为0，1为强制覆盖
int merge=0;      //如果文件存在，是否选择合并，默认是0为否

void copy_file(char *src_path,char *dest_path)
{
    int src_fd, dest_fd;
    char buffer[BUFFER_SIZE];
    int bytes_read, bytes_written;
    //打开文件，准备读写
    //读原文件
    src_fd = open(src_path, O_RDONLY);
    if (src_fd == -1) {
        printf("open source file : %s\n",src_path);     //如果源文件不存在，给出错误信息
        exit(EXIT_FAILURE);
    }

    // 目标文件存在
    if (access(dest_path, F_OK) == 0) {
        if (!force) {
            // 如果不是强制覆盖
            printf("%s already exists. Do you want to overwrite it? (y/n) ", dest_path);
            char answer;
            scanf("%c", &answer);
            //不强制覆盖
            if (answer != 'y' && answer != 'Y') {
                if (merge) {
                    // 如果选择合并就合并
                    dest_fd = open(dest_path, O_WRONLY | O_APPEND); // 追加写入
                    if (dest_fd < 0) {
                        perror("open");
                        exit(EXIT_FAILURE);
                    }
                }
                else {
                    //又不合并又不强制覆盖，就88
                    exit(EXIT_SUCCESS);
                }
            }
                //强制覆盖
            else
            {
                // Open the destination file for writing
                mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;
                dest_fd = open(dest_path, O_WRONLY | O_CREAT | O_TRUNC, mode);
                if (dest_fd == -1) {
                    perror("open");
                    exit(EXIT_FAILURE);
                }
            }
        }
    }
    else
    {
        // Open the destination file for writing
        mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;
        dest_fd = open(dest_path, O_WRONLY | O_CREAT | O_TRUNC, mode);
        if (dest_fd == -1) {
            perror("open");
            exit(EXIT_FAILURE);
        }
    }

    // Copy the contents of the source file to the destination file using read() and write() functions
    while ((bytes_read = read(src_fd, buffer, BUFFER_SIZE)) > 0) {

        bytes_written = write(dest_fd, buffer, bytes_read);
        if (bytes_written != bytes_read) {
            perror("write");
            exit(EXIT_FAILURE);
        }
    }

    // Close the source and destination files
    if (close(src_fd) == -1) {
        perror("close");
        exit(EXIT_FAILURE);
    }
    if (close(dest_fd) == -1) {
        perror("close");
        exit(EXIT_FAILURE);
    }

    printf("File copied successfully: %s\n",basename(src_path));
}

void copy_dir(char *src_path,char *dest_path)
{
    DIR *src_dir;
    struct dirent *entry;
    struct stat stat_buf;
    char src_file_path[PATH_MAX];
    char dest_file_path[PATH_MAX];

    // 打开源目录开读
    src_dir = opendir(src_path);
    if (src_dir == NULL) {
        perror("opendir source dir");
        exit(EXIT_FAILURE);
    }

    // 目标路径，没有的话得mkdir
    struct stat dest_stat;
    if (stat(dest_path, &dest_stat) == -1) {
        // 如果目标路径不存在，则创建它
        if (mkdir(dest_path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1) {
            perror("mkdir");
            exit(EXIT_FAILURE);
        }
    } else if (!S_ISDIR(dest_stat.st_mode)) {
        // 如果目标路径已经存在但不是目录，则打印错误信息并退出程序
        printf("dest_path %s is not a dir\n",dest_path);
        exit(EXIT_FAILURE);
    }


    // 开始遍历复制
    while ((entry = readdir(src_dir)) != NULL) {
        // 跳过对.或者..的copy
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // 构建一个个的文件路径
        snprintf(src_file_path, PATH_MAX, "%s/%s", src_path, entry->d_name);
        snprintf(dest_file_path, PATH_MAX, "%s/%s", dest_path, entry->d_name);

        if (lstat(src_file_path, &stat_buf) == -1) {
            perror("lstat");
            exit(EXIT_FAILURE);
        }

        //判断遍历到的是文件还是目录，开始递归调用
        if (S_ISDIR(stat_buf.st_mode)) {
            // If the source file is a directory, recursively copy it to the destination directory
            copy_dir(src_file_path, dest_file_path);
        } else if (S_ISREG(stat_buf.st_mode)) {
            // If the source file is a regular file, copy it to the destination directory
            copy_file(src_file_path, dest_file_path);
        }
    }

    // Close the source directory
    if (closedir(src_dir) == -1) {
        perror("closedir");
        exit(EXIT_FAILURE);
    }
}

int main(int argc,char *argv[]) {
    char*src_path;
    char* dest_path;

    int cp_dir=0;     //复制的是否为目录，如果是则为1，默认是0

    //-r参数
    int opt;
    while ((opt = getopt(argc, argv, "rmf")) != -1) {
        switch (opt) {
            case 'r':       //显示所有文件和目录,包括隐藏文件
                cp_dir=1;
                break;
            case 'f':
                force=1;
                break;
            case 'm':
                merge=1;
                break;
            default:
                printf("Usage: %s [-r|-f|-m] [directory_path]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    // 剩下的2个参数是src_path and dest_path
    if (argc - optind != 2) {
        fprintf(stderr, "Usage: %s [-r] [-f] <src_path> <dest_path>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    src_path= malloc(100);
    strcpy(src_path,argv[optind]);
    dest_path= malloc(100);
    strcpy(dest_path,argv[optind+1]);
    strcat(dest_path,"/");
    strcat(dest_path,basename(src_path));

    if(cp_dir==0)
    {
        struct stat sb;
        //目前没有这个文件
        if (stat(dest_path, &sb) == -1) {
            //open新文件,暂时不管
            copy_file(src_path,dest_path);
        }
        else if (S_ISDIR(sb.st_mode)) {
            printf("argv[optind+1]:%s\n",argv[optind+1]);
            printf("basename(src_path):%s\n",basename(src_path));
            //dest_path= realloc(dest_path,sizeof(argv[optind+1])+1+sizeof (basename(src_path)));
            strcpy(dest_path,argv[optind+1]);

            copy_file(src_path,dest_path);
        }
        else
            copy_file(src_path,dest_path);
    }
    else
        copy_dir(src_path,dest_path);

    return 0;
}
