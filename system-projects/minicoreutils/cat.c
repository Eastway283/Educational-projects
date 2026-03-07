#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdlib.h>

/* error: вывод сообщения об ошибке и останов программы */
void error(char *fmt, ...);

/* cat: вывод содержимого файлов */
int main(int argc, char **argv)
{
    int fd, n, i, status = 0;
    char buf[BUFSIZ];

    if (argc == 1) {
        while ((n = read(STDIN_FILENO, buf, BUFSIZ)) > 0)
            if (write(STDOUT_FILENO, buf, n) != n)
                error("cat: write error");
    } else {
        for (i = 1; i < argc; i++) {
            if ((fd = open(argv[i], O_RDONLY, 0)) == -1)
            {
                fprintf(stderr, "cat: can't open file %s\n", argv[i]);
                status = 1;
                continue;
            }
            while ((n = read(fd, buf, BUFSIZ)) > 0)
                if (write(STDOUT_FILENO, buf, n) != n)
                    error("cat: write error on file %s", argv[i]);
            close(fd);
        }
    }

    return status;
}

void error(char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    fprintf(stderr, "error: ");
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
    exit(1);
}
