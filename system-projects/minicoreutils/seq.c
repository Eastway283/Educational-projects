#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#define DEFAULT 1

void error(char *fmt, ...);

/* seq: вывод числовой последовательности */ 
int main(int argc, char **argv)
{
    long min, max, increase;
    char *endptr;
    if (argc == 1) {
        fprintf(stderr, "Usage: seq [min] [increase] max\n");
        return 1;
    }

    if (argc == 2) {
        min = 0;
        max = strtol(argv[1], &endptr, 10);
        increase = DEFAULT;
        if (*endptr != '\0')
            error("seq: invalid argument %s", argv[1]);
    } else if (argc == 3) {
        min = strtol(argv[1], &endptr, 10);
        if (*endptr != '\0')
            error("seq: invalid argument %s", argv[1]);
        max = strtol(argv[2], &endptr, 10);
        if (*endptr != '\0')
            error("seq: invalid argument %s", argv[2]);
        increase = DEFAULT;
    } else if (argc == 4) {
        min = strtol(argv[1], &endptr, 10);
        if (*endptr != '\0')
            error("seq: invalid argument %s", argv[1]);
        increase = strtol(argv[2], &endptr, 10);
        if (*endptr != '\0' || increase == 0)
            error("seq: invalid argument %s", argv[2]);
        max = strtol(argv[3], &endptr, 10);
        if (*endptr != '\0')
            error("seq: invalid argument %s", argv[3]);
    } else 
        error("seq: too many arguments");

    if (increase < 0)
        for (long i = min; i >= max; i += increase)
            printf("%ld\n", i);
    else
        for (long i = min; i <= max; i += increase)
            printf("%ld\n", i);

    return 0;
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
