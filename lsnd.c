#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"
#include "fs.h"


void itoa2(int n, char *str) {
    int temp, len;
    temp = n;
    len = 1;
    while (temp / 10 != 0) {
        len++;
        temp /= 10;
    }
    for (temp = len; temp > 0; temp--) {
        str[temp - 1] = (n % 10) + 48;
        n /= 10;
    }
    str[len] = '\0';
}

int
main(int argc, char *argv[]) {

    int pid;
    char path[16];
    int fd;
    char buf[50];

    // name
    if (strcmp(argv[1], "name") == 0) {
        pid = getpid();
        strcpy(path, "/proc/");
        itoa2(pid, path + 6);
        memmove(path + strlen(path), "/name", 5);
        memmove(path + strlen(path) + 5, "\0", 1);
        printf(1, "path: %s\n", path);
        fd = open(path, O_RDONLY);
        read(fd, buf, 50);
        printf(1, "%s\n\n", buf);
        exit();
    }
    // status
    if (strcmp(argv[1], "status") == 0) {
        pid = getpid();
        strcpy(path, "/proc/");
        itoa2(pid, path + 6);
        memmove(path + strlen(path), "/status", 9);
//        printf(1, "path: %s\n", path);
        fd = open(path, O_RDONLY);
        read(fd, buf, 50);
        printf(1, "%s\n\n", buf);
        exit();
    }

    // ideinfo
    if (strcmp(argv[1], "ideinfo") == 0) {
        strcpy(path, "/proc/");
        memmove(path + strlen(path), "/ideinfo", 8);
        memmove(path + strlen(path) + 8, "\0", 1);
//        printf(1, "path: %s\n", path);
        fd = open(path, O_RDONLY);
        read(fd, buf, 50);
        printf(1, "%s\n\n", buf);
        exit();
    }
    // filestat
    if (strcmp(argv[1], "filestat") == 0) {
        strcpy(path, "/proc/");
        memmove(path + strlen(path), "/filestat", 9);
        memmove(path + strlen(path) + 9, "\0", 1);
//        printf(1, "path: %s\n", path);
        fd = open(path, O_RDONLY);
        read(fd, buf, 50);
        printf(1, "%s\n\n", buf);
        exit();
    }
    // inodeinfo
    if (strcmp(argv[1], "inodeinfo") == 0) {
        strcpy(path, "/proc/");
        memmove(path + 6, "inodeinfo/", 10);

        int used_cache_idx[50];
        memset(used_cache_idx, 0, 50);
        get_cache_idx_arr(used_cache_idx);
        char tmp[16];
        char cache_idx_str[10];
        for (int i = 0; i < 50; ++i) {
            //used
            if (used_cache_idx[i] > 0) {
                memset(tmp, 0, 16);
                memset(cache_idx_str, 0, 10);
                memset(buf, 0, 50);
                memmove(tmp, path, 16);

//                printf(1, "cache index: %d\n", i);
                itoa2(i, cache_idx_str);
                memmove(tmp + 16, cache_idx_str, strlen(cache_idx_str));
                memmove(tmp + strlen(tmp), "\0", 1);
//                printf(1, "path: %s\n", tmp);

                fd = open(tmp, O_RDONLY);
                read(fd, buf, 50);
                printf(1, "inode %d:\n %s\n", i, buf);
            }
        }
        exit();
    }

    strcpy(path, "/proc/");
    memmove(path + 6, "inodeinfo/", 10);

    int used_cache_idx[50];
    memset(used_cache_idx, 0, 50);
    get_cache_idx_arr(used_cache_idx);
    char tmp[16];
    char cache_idx_str[10];
    for (int i = 0; i < 50; ++i) {
        //used
        if (used_cache_idx[i] > 0) {
            memset(tmp, 0, 16);
            memset(cache_idx_str, 0, 10);
            memset(buf, 0, 50);
            memmove(tmp, path, 16);

//            printf(1, "cache index: %d\n", i);
            itoa2(i, cache_idx_str);
            memmove(tmp + 16, cache_idx_str, strlen(cache_idx_str));
            memmove(tmp + strlen(tmp), "\0", 1);
//            printf(1, "path: %s\n", tmp);

            fd = open(tmp, O_RDONLY);
            read(fd, buf, 50);
            printf(1, "%s\n\n", i, buf);
        }
    }

    exit();
}

