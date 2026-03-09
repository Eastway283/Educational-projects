#include <stdio.h>
#include <unistd.h>

/* rm: удаление файла */
int main(int argc, char **argv)
{
    int status = 0;
    if (argc == 1) {
        fprintf(stderr, "Usage: rm filename\n");
        return 1;
    }
    for (int i = 1; i < argc; i++)
        if (unlink(argv[i]) != 0) {
            fprintf(stderr, "rm: can't remove %s\n", argv[i]);
            status = 1;
        }

    return status;
}
