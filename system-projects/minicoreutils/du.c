#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>     /* флаги чтения и записи        */
#include <sys/types.h> /* определения типов            */
#include <sys/stat.h>  /* структура, возвращаемая stat */
#include <dirent.h>

#define MAX_PATH 1024

void fsize(char *);
void dirwalk(char *, void (*) (char *));

static unsigned long total = 0;

/* du: вывод общего размера файлов */ 
int main(int argc, char **argv)
{
    if (argc == 1) {
        fsize(".");
        printf("%8ld bytes\n", total);
    }
    else
        for (int i = 0; i < argc; i++) {
            total = 0;
            fsize(argv[i]);
            printf("%s  %8ld bytes\n", argv[i], total);
        }
    return 0;
}

/* fsize: вывод информации о размере файла */
void fsize(char *name)
{
    struct stat stbuf;

    if (stat(name, &stbuf) == -1) {
        fprintf(stderr, "finfo: can't access %s\n", name);
        return;
    }
    if ((stbuf.st_mode & S_IFMT) == S_IFDIR)
        dirwalk(name, fsize);
    total += stbuf.st_size;
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
