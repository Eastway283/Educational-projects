#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXLINE 1000
#define ON 1 
#define OFF 0
#define DEFAULT_CAP 200

int compare(const void *a, const void *b)
{
    return strcmp(*(const char **) a, *(const char **) b);
}

/* sort: вывод строк файла в лексикографическом порядке */ 
int main(int argc, char **argv)
{
    FILE *file;
    char line[MAXLINE];
    char **text = NULL;
    int start, reverse, idx, capacity = DEFAULT_CAP;
    
    if (argc < 2) {
        fprintf(stderr, "Usage: sort [-r] filename");
        return 1;
    }

    if (strcmp(argv[1], "-r") == 0 && argc >= 3) {
        reverse = ON;
        start = 2;
    } else {
        reverse = OFF;
        start = 1;
    }


    file = fopen(argv[start], "r");
    if (!file) {
        fprintf(stderr, "sort: can't open %s\n", argv[start]);
        return 1;
    }

    text = (char **)malloc(capacity * sizeof(char *));
    if (!text) {
        fprintf(stderr, "sort: allocate error\n");
        return 1;
    }
    idx = 0;
    while (fgets(line, MAXLINE, file) != NULL) {
        text[idx++] = strdup(line);
        if (text[idx - 1] == NULL) {
            fprintf(stderr, "sort: strdup error\n");
            exit(1);
        }
        if (idx == capacity) {
            text = realloc(text, (capacity *= 2) * sizeof(char *));
            if (!text) {
                fprintf(stderr, "sort: realloc error\n");
                exit(1);
            }
        }
    }

    qsort(text, idx, sizeof(char *), compare);
    if (reverse) {
        for (int i = idx - 1; i >= 0; i--)
            printf("%s", text[i]);
    } else {
        for (int i = 0; i < idx; i++)
            printf("%s", text[i]);
    }

    while (idx > 0)
        free(text[--idx]);
    free(text);

    return 0;
}
