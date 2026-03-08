#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* sleep: приостановка выполнения */ 
int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "Usage: sleep seconds\n");
        return 1;
    }

    char *endptr;
    long sec = strtol(argv[1], &endptr, 10);
    if (*endptr != '\0' || sec <= 0) {
        fprintf(stderr, "sleep: invalid argument %s", argv[1]);
        return 1;
    }
    sleep((unsigned int) sec);
    return 0;
}
