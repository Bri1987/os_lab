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


int main(int argc, char *argv[]) {
    char dir_path[1024];
    DIR *dir;
    struct dirent *entry;
    struct stat file_stat;

    // 获取目录路径
    //1.如果所有命令行已经处理完毕，默认是当前工作目录
    if (optind >= argc) {
        getcwd(dir_path, sizeof(dir_path));
    } else {
        strcpy(dir_path, argv[optind]);
    }

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

        char* arg[4];
        arg[0] = malloc(100); arg[1] = malloc(100);
        arg[2]= malloc(100);
        arg[3] = NULL;
        //判定文件类型宏
        if (!S_ISDIR(file_stat.st_mode)) {           //不是目录文件
            //开搞子进程
            pid_t pid=fork();
            if(pid<0)
                perror("pid");
            else if(pid==0)
            {
                strcpy(arg[1],file_path);
                strcpy(arg[2],"/usr/local/os5");
                execvp("./process_mycp",arg);
            }
            else
            {
                int status=0;
                waitpid(pid,&status,0);
            }
        }
        else
        {
            //开搞子进程
            pid_t pid=fork();
            if(pid<0)
                perror("pid");
            else if(pid==0)
            {
                strcpy(arg[1],file_path);
                printf("file_path is %s\n",arg[1]);
                arg[2]=NULL;
                execvp("./process_copy",arg);
            }
            else
            {
                int status=0;
                waitpid(pid,&status,0);
            }
        }
    }

    closedir(dir);

    return 0;
}