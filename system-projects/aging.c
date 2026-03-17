#include <stdio.h>
#include <inttypes.h>
#include <string.h>

#define NUM_PAGE 6

uint8_t tables[NUM_PAGE] = {0};
uint8_t reference[NUM_PAGE] = {0};

void print_counters(void)
{
    printf("Counters:\n");
    for (int i = 0; i < NUM_PAGE; i++) {
        printf("%d: %02X (binary: ", i, tables[i]);
        // вывод двоичного представления
        for (int b = 7; b >= 0; b--) {
            putchar((tables[i] >> b) & 1 ? '1' : '0');
        }
        printf(")\n");
    }
}

int find_victim(void)
{
    int min_idx = 0;
    uint8_t min_val = tables[0];
    for (int i = 1; i < NUM_PAGE; i++)
        if (tables[i] < min_val) {
            min_val = tables[i];
            min_idx = i;
        }
    return min_idx;
}

void do_tick(void)
{
    for (int i = 0; i < NUM_PAGE; i++)
        tables[i] >>= 1;
    for (int i = 0; i < NUM_PAGE; i++)
        if (reference[i]) {
            tables[i] |= 0x80;
            reference[i] = 0;
        }
    printf("Tick done. Counters update\n");
}

int main(void)
{
    char input[100];
    printf("Введите номерa страниц(от 0 до 5) для обращения\n'p' - показать счетчик\n'q' - выйти\n't' - сделать такт\n");
    while (1) {
        printf("> ");
        if (fgets(input, sizeof(input), stdin) == NULL)
            break;
        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "p") == 0) {
            print_counters();
            int victim = find_victim();
            printf("Страница для удаления: %d\n", victim);
            continue;
        }
        else if (strcmp(input, "t") == 0) {
            do_tick();
            continue;
        }
        else if (strcmp(input, "q") == 0)
            break;

        char *token = strtok(input, " ");
        while (token != NULL) {
            if (strlen(token) == 1 && token[0] >= '0' && token[0] <= '5') {
                int page = token[0] - '0';
                reference[page] = 1;
            } else {
                printf("Неизвестный символ '%s', ожидалась цифра от 0 до 5-х. Повторите ввод\n", token);
            }
            token = strtok(NULL, " ");
        }
    }

    printf("Программа завершена\n");

    return 0;
}
