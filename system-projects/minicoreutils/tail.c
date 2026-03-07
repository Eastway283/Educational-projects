#include <stdio.h>
#include <stdlib.h>

#define NCOUNT 10
#define MAXLINE 1000

/* tail: вывод n последних строк файла */
int main(int argc, char **argv)
{
    char line[MAXLINE];
    FILE *file;
    int n = NCOUNT, lineno = 0, start, status = 0;

    if (argc == 1) {
        fprintf(stderr, "Usage: tail [-n] filename\n");
        exit(1);
    } else if (argc == 2) {
        file = fopen(argv[1], "r");
        if (!file) {
            fprintf(stderr, "tail: can't open %s\n", argv[1]);
            exit(1);
        }
        while (fgets(line, MAXLINE, file) != NULL)
            lineno++;
        rewind(file);
        for (int i = 0; i < lineno && fgets(line, MAXLINE, file) != NULL; i++)
            if (i >= lineno - n)
                fputs(line, stdout);
        fclose(file);
    } else {
        if (argv[1][0] == '-') {
            start = 2;
            n = atoi(argv[1] + 1);
            if (n <= 0)
                n = NCOUNT;
        } else {
            start = 1;
        }
        for (int i = start; i < argc; i++) {
            lineno = 0;
            file = fopen(argv[i], "r");
            if (!file) {
                fprintf(stderr, "tail: can't open %s\n", argv[i]);
                status = 1;
                continue;
            }
            printf("===> %s <===\n", argv[i]);
            while (fgets(line, MAXLINE, file) != NULL)
                lineno++;
            rewind(file);
            for (int j = 0; j < lineno && fgets(line, MAXLINE, file) != NULL; j++)
                if (j >= lineno - n)
                    fputs(line, stdout);
            fclose(file);
        }
    }

    return status;
}
