#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define NCOUNT 10
#define MAXLINE 1000

/* head: вывод n первых строк файла */
int main(int argc, char **argv) {
    FILE *file;
    int n = NCOUNT;
    int start = 1;
    int status = 0;
  
    if (argc == 1) {
        char line[MAXLINE];
        int lines = 0;
        while (lines < n && fgets(line, MAXLINE, stdin)) {
            fputs(line, stdout);
            lines++;
        }
        return 0;
    }

    if (argv[1][0] == '-' && isdigit((unsigned char)argv[1][1])) {
        n = atoi(argv[1] + 1);
        if (n <= 0) n = NCOUNT;
        start = 2;
    }

    for (int i = start; i < argc; i++) {
        file = fopen(argv[i], "r");
        if (!file) {
            fprintf(stderr, "head: cannot open '%s'\n", argv[i]);
            status = 1;
            continue;
        }
        char line[MAXLINE];
        int lines = 0;
        while (lines < n && fgets(line, MAXLINE, file)) {
            fputs(line, stdout);
            lines++;
        }
        fclose(file);
    }
    return status;
}
