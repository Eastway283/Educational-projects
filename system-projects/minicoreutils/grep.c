#include <stdio.h>
#include <string.h>

#define MAXLINE 1000

/* grep: поиск строк по шаблону */ 
int main(int argc, char **argv)
{
    FILE *file;
    char line[MAXLINE], *pattern;
    unsigned lineno;
    int start, status = 0, number = 0; /* флаг вывода номера строки */

    if (argc < 3) {
        fprintf(stderr, "Usage: grep [-n] pattern filename\n");
        return 1;
    }

    if (argc == 3) {
        pattern = argv[1];
        start = 2;
    } else {
        if (strcmp(argv[1], "-n") == 0) {
            pattern = argv[2];
            start = 3;
            number = 1;
        } else {
            pattern = argv[1];
            start = 2;
        }
    }

    for (int i = start; i < argc; i++) {
        lineno = 0;
        file = fopen(argv[i], "r");
        if (!file) {
            fprintf(stderr, "grep: can't open %s\n", argv[i]);
            status = 1;
            continue;
        }
        while (fgets(line, MAXLINE, file) != NULL) {
            ++lineno;
            if (strstr(line, pattern) != NULL) {
                if (number)
                    printf("%u: %s", lineno, line);
                else
                    printf("%s", line);
            }
        }
        fclose(file);
    }

    return status;
}
