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
#include <sys/shm.h>
#include <sys/sem.h>

#define BUFFER_SIZE 128
#define SEM_KEY 0x12345678
int force=0;      //是否强制覆盖，而不需要用户判读，默认为0，1为强制覆盖
int merge=0;      //如果文件存在，是否选择合并，默认是0为否


void copy_file(char *src_path,char *dest_path,int* shared_var,int semid)
{
    //mycp
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
        // 获取互斥锁
        struct sembuf sem_op;
        sem_op.sem_num = 0;
        sem_op.sem_op = -1;
        sem_op.sem_flg = 0;
        semop(semid, &sem_op, 1);
        *shared_var+=BUFFER_SIZE;
        // 释放互斥锁
        sem_op.sem_num = 0;
        sem_op.sem_op = 1;
        sem_op.sem_flg = 0;
        semop(semid, &sem_op, 1);

        //printf("now is %d\n",*shared_var);
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

int main(int argc,char *argv[]) {
    char* src_path;
    char* dest_path;

    int shmid = shmget(0x123, sizeof(int), 0666 | IPC_CREAT);
    if (shmid == -1) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }
    // 将共享内存附加到当前进程的地址空间
    int* shared_var = (int *)shmat(shmid, NULL, 0);
    if (shared_var == (int *)-1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }


    // 创建信号量
    int semid = semget(SEM_KEY, 1, IPC_CREAT | 0666);
    if (semid == -1) {
        perror("semget");
        exit(EXIT_FAILURE);
    }

    //-r参数
    int opt;
    while ((opt = getopt(argc, argv, "mf")) != -1) {
        switch (opt) {
            case 'f':
                force=1;
                break;
            case 'm':
                merge=1;
                break;
            default:
                printf("Usage: %s [-f|-m] [directory_path]\n", argv[0]);
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

    copy_file(src_path,dest_path,shared_var,semid);

    // 分离共享内存
    shmdt(shared_var);
    return 0;
}
