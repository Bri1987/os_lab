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

int force = 0; //是否强制覆盖，而不需要用户判读，默认为0，1为强制覆盖
int merge = 0; //是否合并，而不是覆盖，默认为0，1为合并

void print_usage_and_exit(const char *prog_name) {
    fprintf(stderr, "Usage: %s [-f] [-m] <src> <dst>\n", prog_name);
    exit(EXIT_FAILURE);
}

void copy_file(const char *src, const char *dst) {
    int src_fd, dst_fd, read_size, write_size;
    char buffer[BUFFER_SIZE];

    if ((src_fd = open(src, O_RDONLY)) < 0) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    if (access(dst, F_OK) == 0) { // 目标文件存在
        if (!force) { // 如果不是强制覆盖
            printf("%s already exists. Do you want to overwrite it? (y/n) ", dst);
            char answer;
            scanf("%c", &answer);
            if (answer != 'y' && answer != 'Y') {
                if (merge) { // 如果选择合并
                    dst_fd = open(dst, O_WRONLY | O_APPEND); // 追加写入
                    if (dst_fd < 0) {
                        perror("open");
                        exit(EXIT_FAILURE);
                    }
                } else {
                    exit(EXIT_SUCCESS);
                }
            }
        }
    }

    if ((dst_fd = open(dst, O_WRONLY | O_CREAT | O_TRUNC, 0666)) < 0) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    while ((read_size = read(src_fd, buffer, BUFFER_SIZE)) > 0) {
        write_size = write(dst_fd, buffer, read_size);
        if (write_size < 0) {
            perror("write");
            exit(EXIT_FAILURE);
        }
    }

    close(src_fd);
    close(dst_fd);
}

void copy_dir(const char *src, const char *dst) {
    DIR *dir;
    struct dirent *dp;
    char src_file[BUFFER_SIZE], dst_file[BUFFER_SIZE];

    if ((dir = opendir(src)) == NULL) {
        perror("opendir");
        exit(EXIT_FAILURE);
    }

    while ((dp = readdir(dir)) != NULL) {
        if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0) {
            continue;
        }

        snprintf(src_file, BUFFER_SIZE, "%s/%s", src, dp->d_name);
        snprintf(dst_file, BUFFER_SIZE, "%s/%s", dst, dp->d_name);

        if (dp->d_type == DT_DIR) {
            if (mkdir(dst_file, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0) {
                perror("mkdir");
                exit(EXIT_FAILURE);
            }

            copy_dir(src_file, dst_file);
        } else {
            copy_file(src_file, dst_file);
        }
    }

    closedir(dir);
}

int main(int argc, char *argv[]) {
    int opt;

    while ((opt = getopt(argc, argv, "fm")) != -1) {
        switch (opt) {
            case 'f':
                force = 1;
                break;
            case 'm':
                merge = 1;
                break;
            default:
                print_usage_and_exit(argv[0]);
        }
    }

    if (optind + 2 != argc) {
        print_usage_and_exit(argv[0]);
    }

    struct stat src_stat_buf, dst_stat_buf;
    if (stat(argv[optind], &src_stat_buf) != 0) {
        perror("stat");
        exit(EXIT_FAILURE);
    }

    if (stat(argv[optind + 1], &dst_stat_buf) == 0) {
        if (S_ISDIR(src_stat_buf.st_mode) && !S_ISDIR(dst_stat_buf.st_mode)) {
            fprintf(stderr, "Error: destination '%s' is not a directory.\n", argv[optind + 1]);
            exit(EXIT_FAILURE);
        }

        if (!S_ISDIR(src_stat_buf.st_mode) && S_ISDIR(dst_stat_buf.st_mode)) {
            snprintf(dst_stat_buf.st_name, BUFFER_SIZE, "%s/%s", argv[optind + 1], basename(argv[optind]));
        }

        if (S_ISDIR(src_stat_buf.st_mode) && S_ISDIR(dst_stat_buf.st_mode)) {
            snprintf(dst_stat_buf.st_name, BUFFER_SIZE, "%s/%s", argv[optind + 1], basename(argv[optind]));
            if (mkdir(dst_stat_buf.st_name, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0) {
                perror("mkdir");
                exit(EXIT_FAILURE);
            }
            copy_dir(argv[optind], dst_stat_buf.st_name);
            exit(EXIT_SUCCESS);
        }
    }

    if (S_ISDIR(src_stat_buf.st_mode)) {
        if (mkdir(argv[optind + 1], S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0) {
            perror("mkdir");
            exit(EXIT_FAILURE);
        }
        copy_dir(argv[optind], argv[optind + 1]);
    } else {
        copy_file(argv[optind], argv[optind + 1]);
    }

    return EXIT_SUCCESS;
}