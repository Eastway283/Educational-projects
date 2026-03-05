#include <stdio.h>
#include <string.h>
#include <fcntl.h>       /* флаги чтения и записи     */
#include <unistd.h>     /* системные вызовы           */
#include <sys/types.h> /* определения типа            */
#include <sys/stat.h> /* структура, возвращаемая stat */
#include <dirent.h>  /* структура dirent              */
#include <time.h>   /* вывод временных параметров     */

#define MAX_PATH 1024

void fsize(char *);

void finfo(char *);

void dirwalk(char *, void (*fcn) (char *));

/* вывод размера файла */
int main(int argc, char *argv[])
{
    if (argc == 1)
        finfo(".");
    else
        while (--argc > 0)
            finfo(*++argv);
    return 0;
}

/* fsize: вывод размера файла name */
void fsize(char *name)
{
    struct stat stbuf;

    if (stat(name, &stbuf) == -1) {
        fprintf(stderr, "fsize: can't access %s\n", name);
        return;
    }
    if ((stbuf.st_mode & S_IFMT) == S_IFDIR)
        dirwalk(name, fsize);
    printf("%8ld %s\n", stbuf.st_size, name);
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
    printf("name: %s\n", name);
    printf("size: %ld bytes\n", stbuf.st_size);
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
            continue; /* пропустить себя и родительский */
        if (strlen(dir) + strlen(dp->d_name) + 2 > sizeof(name))
            fprintf(stderr, "dirwalk: name %s %s too long\n", dir, dp->d_name);
        else {
            sprintf(name, "%s/%s", dir, dp->d_name);
            (*fcn) (name);
        }
    }
    closedir(dfd);
}
