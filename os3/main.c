#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <string.h>

int main(int argc, char *argv[]) {
    char dir_path[1024];
    DIR *dir;
    struct dirent *entry;
    struct stat file_stat;
    struct passwd *pwd;
    struct group *grp;
    char time_buf[80];

    //1.获取当前工作目录
    if (argc == 1) {
        getcwd(dir_path, sizeof(dir_path));
    } else if (argc > 1) {
        strcpy(dir_path,argv[1]);
    } else {
        printf("Usage: %s [directory_path]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    //2.打开目录
    dir = opendir(dir_path);
    if (dir == NULL) {
        perror("opendir");
        exit(EXIT_FAILURE);
    }

    //3.读取目录文件,获取目录项结构指针
    while ((entry = readdir(dir)) != NULL) {
        char file_path[1024];
        //拼接获取整个文件的path
        sprintf(file_path, "%s/%s", dir_path, entry->d_name);

        //获取文件属性，并将其填充到stat结构中
        if (lstat(file_path, &file_stat) == -1) {
            perror("lstat");
            exit(EXIT_FAILURE);
        }

        //判定文件类型宏
        if (S_ISREG(file_stat.st_mode) || S_ISLNK(file_stat.st_mode)) {         //普通文件或符号链接
            printf("-");
        } else if (S_ISDIR(file_stat.st_mode)) {           //目录文件
            printf("d");
        } else if (S_ISCHR(file_stat.st_mode)) {          //字符设备
            printf("c");
        } else if (S_ISBLK(file_stat.st_mode)) {      //块设备
            printf("b");
        } else if (S_ISFIFO(file_stat.st_mode)) {     //管道
            printf("p");
        } else if (S_ISSOCK(file_stat.st_mode)) {      //socket
            printf("s");
        } else {
            printf("?");
        }

        printf((file_stat.st_mode & S_IRUSR) ? "r" : "-");     //文件所有者拥有的可读取权限
        printf((file_stat.st_mode & S_IWUSR) ? "w" : "-");
        printf((file_stat.st_mode & S_IXUSR) ? "x" : "-");
        printf((file_stat.st_mode & S_IRGRP) ? "r" : "-");
        printf((file_stat.st_mode & S_IWGRP) ? "w" : "-");
        printf((file_stat.st_mode & S_IXGRP) ? "x" : "-");
        printf((file_stat.st_mode & S_IROTH) ? "r" : "-");
        printf((file_stat.st_mode & S_IWOTH) ? "w" : "-");
        printf((file_stat.st_mode & S_IXOTH) ? "x" : "-");

        //打印出文件的硬链接数,表示有多少个文件名指向该文件
        printf(" %ld", file_stat.st_nlink);

        //输入用户id，返回用户属性信息
        pwd = getpwuid(file_stat.st_uid);
        if (pwd != NULL) {
            printf(" %s", pwd->pw_name);  //用户名
        } else {
            printf(" %d", file_stat.st_uid);
        }

        grp = getgrgid(file_stat.st_gid);
        if (grp != NULL) {
            printf(" %s", grp->gr_name);   //组名称
        } else {
            printf(" %d", file_stat.st_gid);
        }

        printf(" %7ld", file_stat.st_size);  //文件大小

        strftime(time_buf, 80, "%Y-%m-%d %H:%M:%S", localtime(&file_stat.st_mtime));   //表示最后一次修改该文件的时间
        printf(" %s", time_buf);

        printf(" %s\n", entry->d_name);   //打印文件名
    }

    //关闭目录
    closedir(dir);

    return 0;
}