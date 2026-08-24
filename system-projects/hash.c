#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TABLE_SIZ 101

unsigned long hash(const char *str) {
    unsigned long h = 5381;
    int c;
    while ((c = *str++))
        h = ((h << 5) + h) + c;
    return h;
}

typedef struct entry {
    char *name;
    int val;
    int used; // 0 - empty, 1 - busy
} entry;

entry table[TABLE_SIZ];

int enter(const char *symbol, int val) {
    unsigned int i = hash(symbol) % TABLE_SIZ;
    while (table[i].used && strcmp(table[i].name, symbol) != 0)
        i = (i + 1) % TABLE_SIZ;
    if (!table[i].used) {
        table[i].name = strdup(symbol);
        if (!table[i].name)
            return -1;
        table[i].used = 1;
    }
    table[i].val = val;
    return i;
}

int lookup(const char *symbol) {
    unsigned int i = hash(symbol) % TABLE_SIZ;
    while (table[i].used) {
        if (strcmp(table[i].name, symbol) == 0)
            return table[i].val;
        i = (i + 1) % TABLE_SIZ;
    }
    return -1;
}

void freetable(void) {
    for (int i = 0; i < TABLE_SIZ; i++)
        if (table[i].name)
            free(table[i].name);
}

int main(int argc, char **argv) {

    char line[128];
    memset(table, 0, sizeof(table));
    printf("Enter line and value\n> ");
    while (fgets(line, sizeof(line), stdin)) {
        char *space = strchr(line, ' ');
        if (!space)
            continue;
        *space = '\0';
        int val = atoi(space + 1);
        printf("symbol - %s, value - %d\n", line, val);
        int index = enter(line, val);
        printf("pair was paste in table[%d]\n", index);
    }

    freetable();

    return 0;
}
