#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#define PERMS 0666

/* cp: копирование содержимого файла f1 в f2 */
int main(int argc, char **argv)
{
    int f1, f2, n;
    char buf[BUFSIZ];
    if (argc != 3) {
        fprintf(stderr, "Usage: cp from to\n");
        return 1;
    }
    if ((f1 = open(argv[1], O_RDONLY, 0)) == -1) {
        perror(argv[1]);
        return 1;
    }
    if ((f2 = creat(argv[2], PERMS)) == -1) {
        perror(argv[2]);
        return 1;
    }
    while ((n = read(f1, buf, BUFSIZ)) > 0)
        if (write(f2, buf, n) != n) {
            perror("copy");
            return 1;
        }
    close(f1);
    close(f2);
    return 0;
}
