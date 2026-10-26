#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUF_SIZE 4096
#define MAX_FILES 1024

int insertsort(int num);
int diag(const char* mes, int err_code);
void copy_file(const char* src, const char* dst);

static char* list[MAX_FILES];

int insertsort(int num) {
    int i = 0;
    int j;
    char* p;
    while (++i < num) {
        j = i - 1;
        p = list[i];
        while (j >= 0 && strcmp(p, list[j]) < 0) {
            list[j + 1] = list[j];
            j--;
        }
        list[j + 1] = p;
    }
    return 0;
}

int diag(const char* mes, int err_code) {
    write(2, mes, strlen(mes));
    write(2, ": ", 2);
    const char* err_str = strerror(err_code);
    write(2, err_str, strlen(err_str));
    write(2, "\n", 1);
    exit(err_code);
}

void copy_file(const char* src, const char* dst) {
    int in_fd, out_fd;
    char buf[BUF_SIZE];
    ssize_t bytes_read;

    in_fd = open(src, O_RDONLY);
    if (in_fd < 0) return;

    out_fd = open(dst, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (out_fd < 0) {
        close(in_fd);
        return;
    }

    while ((bytes_read = read(in_fd, buf, sizeof(buf))) > 0) {
        if (write(out_fd, buf, bytes_read) != bytes_read) {
            break;
        }
    }

    close(in_fd);
    close(out_fd);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        diag("Usage: ./directory <target_dir>", EINVAL);
    }

    char* target_dir = argv[1];
    struct stat st_src, st_dst;
    DIR* fdir;
    struct dirent* entry;

    if (stat(".", &st_src) < 0) {
        diag("stat current dir error", errno);
    }
    if (stat(target_dir, &st_dst) == 0) {
        if (st_src.st_ino == st_dst.st_ino && st_src.st_dev == st_dst.st_dev) {
            diag("Source and target directories are the same", EINVAL);
        }
    } else if (errno == ENOENT) {
        if (mkdir(target_dir, 0777) < 0) {
            diag("mkdir error", errno);
        }
        printf("Directory '%s' created.\n", target_dir);
    } else {
        diag("stat target dir error", errno);
    }

    if ((fdir = opendir(".")) == NULL) {
        diag("opendir error", errno);
    }

    int count_files = 0;
    int count_dirs = 0;
    char* start = sbrk(0);

    while ((entry = readdir(fdir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        struct stat sbuf;
        if (stat(entry->d_name, &sbuf) < 0) {
            continue;
        }

        if (S_ISDIR(sbuf.st_mode)) {
            count_dirs++;
        } else if (S_ISREG(sbuf.st_mode)) {
            int len = strlen(entry->d_name);
            char* s = sbrk(len + 1);
            memcpy(s, entry->d_name, len + 1);
            list[count_files] = s;
            count_files++;
        }
    }
    closedir(fdir);

    insertsort(count_files);

    char dest_path[4096];
    for (int i = 0; i < count_files; i++) {
        snprintf(dest_path, sizeof(dest_path), "%s/%s", target_dir, list[i]);
        copy_file(list[i], dest_path);
    }

    brk(start);

    char diag_msg[256];
    int msg_len = snprintf(diag_msg, sizeof(diag_msg),
        "\n--- Diagnostics ---\n"
        "Copied files: %d\n"
        "Subdirectories found: %d\n",
        count_files, count_dirs);
    write(2, diag_msg, msg_len);

    exit(0);
}