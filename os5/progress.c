#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <string.h>
#include <sys/wait.h>
#include <libgen.h>
#include <sys/shm.h>
#include <sys/sem.h>

#define SEM_KEY 0x12345678
int shmid;
int semid;

int calculate_directory_size(char* dir_path) {
    DIR *dir;
    struct dirent *entry;
    struct stat file_stat;
    int total_size = 0;

    // 打开目录
    dir = opendir(dir_path);
    if (dir == NULL) {
        perror("opendir");
        exit(EXIT_FAILURE);
    }

    // 读取目录文件
    while ((entry = readdir(dir)) != NULL) {
        // 是否需要显示当前目录和上级目录,不显示就88
        if ((strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)) {
            continue;
        }

        // 是否需要显示隐藏文件，不显示就88
        if (strcmp(entry->d_name,".")!=0 && strcmp(entry->d_name,"..")!=0 && entry->d_name[0] == '.') {
            continue;
        }

        char file_path[1024];
        //拼接获取整个文件的path
        sprintf(file_path, "%s/%s", dir_path, entry->d_name);

        if (lstat(file_path, &file_stat) == -1) {
            perror("lstat");
            exit(EXIT_FAILURE);
        }

        if (S_ISDIR(file_stat.st_mode)) { // 如果是目录，递归计算子目录大小
            total_size += calculate_directory_size(file_path);
        } else {
            total_size += file_stat.st_size; // 如果是文件，累加文件大小
        }
    }

    closedir(dir);
    return total_size;
}

int main(int argc,char *argv[])
{
    //创建共享内存,记录现在copy的字节数
    int *shared_var;

    shmid = shmget(0x123, sizeof(int), 0666 | IPC_CREAT);
    if (shmid == -1) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    // 将共享内存附加到当前进程的地址空间
    shared_var = (int *)shmat(shmid, NULL, 0);
    if (shared_var == (int *)-1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    *shared_var=0;

    // 创建信号量
    semid = semget(SEM_KEY, 1, IPC_CREAT | 0666);
    if (semid == -1) {
        perror("semget");
        exit(EXIT_FAILURE);
    }

    // 初始化信号量
    semctl(semid, 0, SETVAL, 1);

    // 获取目录路径
    //1.如果所有命令行已经处理完毕，默认是当前工作目录
    char dir_path[1024];
    if (optind >= argc) {
        getcwd(dir_path, sizeof(dir_path));
    } else {
        strcpy(dir_path, argv[optind]);
    }
    //1.计算目录总大小
    int total_size= calculate_directory_size(dir_path);
    printf("total size is %d\n",total_size);

    //2.调用process_main了
    pid_t pid=fork();
    if(pid==0)
    {
        char* arg[4];
        arg[0] = malloc(100); arg[1] = malloc(100);
        arg[2]= NULL;
        strcpy(arg[1],dir_path);
        execvp("./process_copy",arg);
    }
    else
    {
        //主进程
        //3.每隔一段时间算一次进度
        while(*shared_var<total_size)
        {
            printf("===============process : %.2f%%\n",(float)(*shared_var)/total_size);
            usleep(500);
        }
    }


    // 分离共享内存
    shmdt(shared_var);

    // 删除共享内存
    shmctl(shmid, IPC_RMID, NULL);

    // 删除信号量
    semctl(semid, 0, IPC_RMID);
    return 1;
}