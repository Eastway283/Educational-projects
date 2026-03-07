#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXLINE 1000

#define NOT_EMPTY 1 
#define ALL 0

/* nl: вывод строк со сквозной нумерацией               */
/* флаг -t - только непустые(по умолчанию), -a - все    */
int main(int argc, char **argv)
{
    char line[MAXLINE];
    FILE *file;
    unsigned lineno;
    int start, status, mode;
    status = 0;
    mode = NOT_EMPTY;

    if (argc == 1) {
        fprintf(stderr, "Usage: nl [-t, -a] filename\n");
        exit(1);
    }
    if (argc == 2) {
        file = fopen(argv[1], "r");
        if (!file) {
            fprintf(stderr, "nl: can't open %s\n", argv[1]);
            exit(1);
        }
        lineno = 0;
        while (fgets(line, MAXLINE, file) != NULL)
            if (strlen(line) > 1)
            printf("\t%u  %s", ++lineno, line);
        fclose(file);
    } else {
        if (argv[1][0] == '-') {
            if (argv[1][1] == 'a')
                mode = ALL;
            else if (argv[1][1] == 't')
                mode = NOT_EMPTY;
            else {
                fprintf(stderr, "nl: illegal option %c\n", argv[1][1]);
                exit(1);
            }
            start = 2;
        } else 
            start = 1;
        lineno = 0;
        for (int i = start; i < argc; i++) {
            file = fopen(argv[i], "r");
            if (!file) {
                fprintf(stderr, "nl: can't open file %s\n", argv[i]);
                status = 1;
                continue;
            }
            while (fgets(line, MAXLINE, file) != NULL) {
                if (mode == ALL)
                    printf("\t%u  %s", ++lineno, line);
                else {
                    if (strlen(line) > 1)
                        printf("\t%u  %s", ++lineno, line);
                }
            }
            fclose(file);
        }
    }
 
    return status;
}
