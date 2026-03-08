#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>     /* флаги чтения и записи        */
#include <sys/types.h> /* определения типов            */
#include <sys/stat.h>  /* структура, возвращаемая stat */
#include <dirent.h>
#include <time.h>

#define MAX_PATH 1024

void finfo(char *);
void dirwalk(char *, void (*) (char *));

/* ls: вывод информации о каталоге и его содержании */ 
int main(int argc, char **argv)
{
    if (argc == 1)
        finfo(".");
    else
        while (--argc > 0)
            finfo(++*argv);

    return 0;
}

/* finfo: вывод информации о файле name */
void finfo(char *name)
{
    struct stat stbuf;

    if (stat(name, &stbuf) == -1) {
        fprintf(stderr, "finfo: can't access %s\n", name);
        return;
    }
    if ((stbuf.st_mode & S_IFMT) == S_IFDIR)
        dirwalk(name, finfo);
    printf("file name: %s\n", name);
    printf("size: %8ld bytes\n", stbuf.st_size);
    printf("last access: %s", ctime(&stbuf.st_atime));
    printf("last modification: %s", ctime(&stbuf.st_mtime));
    printf("last status change: %s", ctime(&stbuf.st_ctime));
}

/* dirwalk: применение fcn ко всем файлам каталога dir */
void dirwalk(char *dir, void (*fcn) (char *))
{
    char name[MAX_PATH];
    struct dirent *dp;
    DIR *dfd;

    if ((dfd = opendir(dir)) == NULL) {
        fprintf(stderr, "dirwalk: can't open %s\n", dir);
        return;
    }
    while ((dp = readdir(dfd)) != NULL) {
        if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0)
            continue;
        if (strlen(dir) + strlen(dp->d_name) + 2 > sizeof(name))
            fprintf(stderr, "dirwalk: name %s %s too long\n", dir, dp->d_name);
        else {
            sprintf(name, "%s/%s", dir, dp->d_name);
            (*fcn) (name);
        }
    }
    closedir(dfd);
}
