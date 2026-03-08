#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXLINE 1000
#define ON 1
#define OFF 0
#define DEFAULT_CAP 200

/* tac: вывод содержимого файла наоборот */ 
int main(int argc, char **argv)
{
    FILE *file;
    char line[MAXLINE];
    char **text = NULL;
    int start, idx, number, capacity = DEFAULT_CAP;
    
    if (argc < 2) {
        fprintf(stderr, "Usage: tac [-n] filename");
        return 1;
    }

    if (strcmp(argv[1], "-n") == 0 && argc >= 3) {
        number = ON;
        start = 2;
    } else {
        number = OFF;
        start = 1;
    }


    file = fopen(argv[start], "r");
    if (!file) {
        fprintf(stderr, "tac: can't open %s\n", argv[start]);
        return 1;
    }

    text = (char **)malloc(capacity * sizeof(char *));
    if (!text) {
        fprintf(stderr, "tac: allocate error\n");
        fclose(file);
        return 1;
    }
    idx = 0;
    while (fgets(line, MAXLINE, file) != NULL) {
        text[idx++] = strdup(line);
        if (text[idx - 1] == NULL) {
            fprintf(stderr, "tac: strdup error\n");
            fclose(file);
            exit(1);
        }
        if (idx == capacity) {
            text = realloc(text, (capacity *= 2) * sizeof(char *));
            if (!text) {
                fprintf(stderr, "tac: realloc error\n");
                fclose(file);
                exit(1);
            }
        }
    }
    fclose(file);

    if (number) {
        for (int i = idx - 1; i >= 0; i--)
            printf("%d  %s", idx - i, text[i]);
    } else {
        for (int i = idx - 1; i >= 0; i--)
            printf("%s", text[i]);
    }

    while (idx > 0)
        free(text[--idx]);
    free(text);

    return 0;
}
