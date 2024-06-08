#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

struct d_name_st_mtime
{
        char name[32];
        time_t mtime;
};

int compare_d_name_st_mtime(const void* a, const void* b)
{
    time_t arg1 = ((struct d_name_st_mtime*)a)->mtime;
    time_t arg2 = ((struct d_name_st_mtime*)b)->mtime;

    if (arg1 < arg2) return -1;
    if (arg1 > arg2) return 1;
    return 0;
}

int main(int argc, char *argv[])
{
        DIR *current_dir;
        if (argc > 2) {
                fprintf(stderr,"Too many arguments in instruction\n");
                return 1;
        } else if (argc == 2) {
                current_dir = opendir(argv[1]);
                if (current_dir == NULL) {
                        perror("Can`t open specified directory");
                        return 2;
                }
                chdir(argv[1]);
        }
        else {
                current_dir = opendir("./");
        }

        size_t files_cnt = 0;
        size_t subdirs_cnt = 0;
        struct dirent *file;
        struct stat stbuf;
        while (file = readdir(current_dir)) {
                if (lstat(file->d_name, &stbuf)) {
                        printf("Can`t get stats for %s\n", file->d_name);
                        continue;
                }
                if (strcmp(file->d_name, ".") == 0 || strcmp(file->d_name, "..") == 0) {
                        continue;
                }
                if ((stbuf.st_mode & S_IFMT) != S_IFDIR) files_cnt++;
                else subdirs_cnt++;
        }
        rewinddir(current_dir);
        printf("Total files: %d\n", files_cnt + subdirs_cnt);

        size_t files_idx = 0;
        size_t subdirs_idx = 0;
        struct d_name_st_mtime *files = (struct d_name_st_mtime*)malloc(files_cnt * sizeof(struct d_name_st_mtime));
        struct d_name_st_mtime *subdirs = (struct d_name_st_mtime*)malloc(subdirs_cnt * sizeof(struct d_name_st_mtime));
        while (file = readdir(current_dir)) {
                if (lstat(file->d_name, &stbuf) || strcmp(file->d_name, ".") == 0 || strcmp(file->d_name, "..") == 0) {
                        continue;
                }
                if (((stbuf.st_mode & S_IFMT) != S_IFDIR)) {
                        strcpy(files[files_idx].name, file->d_name);
                        files[files_idx].mtime = stbuf.st_mtime;
                        files_idx++;
                }
                else {
                        strcpy(subdirs[subdirs_idx].name, file->d_name);
                        subdirs[subdirs_idx].mtime = stbuf.st_mtime;
                        subdirs_idx++;
                }
        }

        qsort(files, files_cnt, sizeof(struct d_name_st_mtime), compare_d_name_st_mtime);
        printf("\nFiles: %d\n", files_cnt);
        for (size_t i = 0; i < files_cnt; ++i) {
                printf("%-31.31s\t%s", files[i].name, ctime(&files[i].mtime));
        }

        qsort(subdirs, subdirs_cnt, sizeof(struct d_name_st_mtime), compare_d_name_st_mtime);
        printf("\nDirectories: %d\n", subdirs_cnt);
        for (size_t i = 0; i < subdirs_cnt; ++i) {
                printf("%-31.31s\t%s", subdirs[i].name, ctime(&subdirs[i].mtime));
        }

        return 0;
}