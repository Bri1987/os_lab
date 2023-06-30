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
    int show_hidden = 0; // 是否显示隐藏文件,1为显示，默认为0不显示
    int show_self = 0; // 是否显示当前目录和上级目录，默认为0不显示
    int show_link=0;   //默认0显示符号链接文件本身的属性

    // 解析命令行参数
    //k是自定义的，表示只多显示.和..，不显示隐藏文件
    //j是自定义的，表示符号链接
    int opt;
    while ((opt = getopt(argc, argv, "aAkj")) != -1) {
        switch (opt) {
            case 'a':       //显示所有文件和目录,包括隐藏文件
                show_hidden = 1;
                show_self=1;
                break;
            case 'A':       //显示除了.和..的所有文件和目录，包括隐藏文件
                show_hidden =1;
                break;
            case 'k':
                show_self=1;
                break;
            case 'j':
                show_link=1;     //显示符号链接指向文件的文件属性
                break;
            default:
                printf("Usage: %s [-a|-A|-k|-j] [directory_path]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

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
    int i=0;    //记录文件总数
    while ((entry = readdir(dir)) != NULL) {
        // 是否需要显示当前目录和上级目录,不显示就88
        if (show_self==0 && (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)) {
            continue;
        }

        // 是否需要显示隐藏文件，不显示就88
        if (show_hidden ==0 && strcmp(entry->d_name,".")!=0 && strcmp(entry->d_name,"..")!=0 && entry->d_name[0] == '.') {
            continue;
        }

        char file_path[1024];
        //拼接获取整个文件的path
        sprintf(file_path, "%s/%s", dir_path, entry->d_name);

        //获取文件属性，并将其填充到file_stat结构中
        if (show_link) {
            if (stat(file_path, &file_stat) == -1) {   ///stat返回的已经不是S_ISLNK了，就是普通文件类型了
                perror("stat");
                exit(EXIT_FAILURE);
            }
        } else {
            if (lstat(file_path, &file_stat) == -1) {
                perror("lstat");
                exit(EXIT_FAILURE);
            }
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


        if (!show_link && S_ISLNK(file_stat.st_mode)) {   //如果是符号链接文件，输出链接指向文件的路径
            printf(" %s", entry->d_name);   //打印文件名
            char link_path[1024];
            ssize_t len = readlink(file_path, link_path, sizeof(link_path) - 1);
            if (len != -1) {
                link_path[len] = '\0';
                printf(" -> %s", link_path);
            }
        }
        else
            printf(" %s", entry->d_name);   //打印文件名,
        printf("\n");
        i++;
    }

    closedir(dir);

    printf("total: %d\n",i);

    return 0;
}
