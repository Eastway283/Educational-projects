#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <time.h>

#define DISK1_FILE "disk1.img"
#define DISK2_FILE "disk2.img"
#define BLOCK_SIZE 4096
#define COUNTER_SIZE sizeof(uint32_t)
#define DATA_SIZE (BLOCK_SIZE - COUNTER_SIZE)   // 4092 байта для данных

// Структура для хранения данных и счётчика
typedef struct {
    uint32_t counter;
    char data[DATA_SIZE];
} disk_block_t;

// Инициализация дисков (создание файлов, если их нет)
void init_disks() {
    int fd1 = open(DISK1_FILE, O_RDWR | O_CREAT, 0666);
    int fd2 = open(DISK2_FILE, O_RDWR | O_CREAT, 0666);
    if (fd1 == -1 || fd2 == -1) {
        perror("Не удалось создать файлы дисков");
        exit(1);
    }
    // Устанавливаем размер файлов до BLOCK_SIZE
    if (ftruncate(fd1, BLOCK_SIZE) == -1 || ftruncate(fd2, BLOCK_SIZE) == -1) {
        perror("ftruncate");
        exit(1);
    }
    // Проверяем, инициализированы ли диски (читаем счётчик)
    disk_block_t block1, block2;
    ssize_t r1 = read(fd1, &block1, BLOCK_SIZE);
    ssize_t r2 = read(fd2, &block2, BLOCK_SIZE);
    if (r1 != BLOCK_SIZE || r2 != BLOCK_SIZE) {
        // Если файлы пусты или повреждены, инициализируем нулями
        memset(&block1, 0, sizeof(block1));
        memset(&block2, 0, sizeof(block2));
        lseek(fd1, 0, SEEK_SET);
        lseek(fd2, 0, SEEK_SET);
        write(fd1, &block1, BLOCK_SIZE);
        write(fd2, &block2, BLOCK_SIZE);
        printf("Диски инициализированы нулями.\n");
    }
    close(fd1);
    close(fd2);
}

// Чтение блока с диска (файла) по номеру (0 или 1)
int read_disk(int disk_num, disk_block_t *block) {
    const char *filename = (disk_num == 0) ? DISK1_FILE : DISK2_FILE;
    int fd = open(filename, O_RDONLY);
    if (fd == -1) return -1;
    ssize_t n = read(fd, block, BLOCK_SIZE);
    close(fd);
    return (n == BLOCK_SIZE) ? 0 : -1;
}

// Запись блока на диск
int write_disk(int disk_num, const disk_block_t *block) {
    const char *filename = (disk_num == 0) ? DISK1_FILE : DISK2_FILE;
    int fd = open(filename, O_WRONLY);
    if (fd == -1) return -1;
    ssize_t n = write(fd, block, BLOCK_SIZE);
    close(fd);
    return (n == BLOCK_SIZE) ? 0 : -1;
}

// Восстановление: копируем данные с диска с большим счётчиком на диск с меньшим
void recover() {
    disk_block_t block1, block2;
    if (read_disk(0, &block1) != 0 || read_disk(1, &block2) != 0) {
        fprintf(stderr, "Ошибка чтения дисков при восстановлении\n");
        return;
    }
    if (block1.counter > block2.counter) {
        printf("Восстановление: копируем диск1 -> диск2\n");
        write_disk(1, &block1);
    } else if (block2.counter > block1.counter) {
        printf("Восстановление: копируем диск2 -> диск1\n");
        write_disk(0, &block2);
    } else {
        printf("Счётчики равны, восстановление не требуется.\n");
    }
}

// Атомарная запись (имитация стабильного хранилища)
// Данные копируются в буфер, счётчик увеличивается, запись на диск1, затем на диск2
// Если происходит сбой между этими операциями, при восстановлении будет выбрана более новая версия
int write_stable(const char *data, size_t len) {
    if (len > DATA_SIZE) {
        fprintf(stderr, "Данные слишком длинные (макс %ld байт)\n", DATA_SIZE);
        return -1;
    }
    // Читаем текущие блоки
    disk_block_t block1, block2;
    if (read_disk(0, &block1) != 0 || read_disk(1, &block2) != 0) {
        fprintf(stderr, "Ошибка чтения дисков перед записью\n");
        return -1;
    }
    // Используем максимальный счётчик + 1
    uint32_t new_counter = (block1.counter > block2.counter) ? block1.counter : block2.counter;
    new_counter++;
    // Формируем новый блок
    disk_block_t new_block;
    new_block.counter = new_counter;
    memset(new_block.data, 0, DATA_SIZE);
    memcpy(new_block.data, data, len);
    // Сначала пишем на первый диск
    if (write_disk(0, &new_block) != 0) {
        fprintf(stderr, "Ошибка записи на диск1\n");
        return -1;
    }
    // Здесь может произойти сбой! (имитируем)
    // Затем пишем на второй диск
    if (write_disk(1, &new_block) != 0) {
        fprintf(stderr, "Ошибка записи на диск2\n");
        return -1;
    }
    printf("Запись успешно выполнена, новый счётчик: %u\n", new_counter);
    return 0;
}

// Чтение стабильных данных: выбираем диск с большим счётчиком
int read_stable(char *buffer, size_t bufsize) {
    disk_block_t block1, block2;
    if (read_disk(0, &block1) != 0 || read_disk(1, &block2) != 0) {
        fprintf(stderr, "Ошибка чтения дисков\n");
        return -1;
    }
    disk_block_t *selected = NULL;
    if (block1.counter > block2.counter) {
        selected = &block1;
    } else if (block2.counter > block1.counter) {
        selected = &block2;
    } else {
        // равны, берём первый
        selected = &block1;
    }
    size_t copy_len = DATA_SIZE;
    if (bufsize < DATA_SIZE) copy_len = bufsize;
    memcpy(buffer, selected->data, copy_len);
    // Гарантируем нуль-терминатор
    if (copy_len < bufsize) buffer[copy_len] = '\0';
    else buffer[bufsize-1] = '\0';
    printf("Прочитаны данные (счётчик %u): %s\n", selected->counter, buffer);
    return 0;
}

// Имитация сбоя: принудительно завершаем программу с ошибкой
void crash() {
    fprintf(stderr, "Имитация сбоя системы!\n");
    fflush(stderr);
    abort(); // немедленное завершение
}

// Простая командная строка
void print_help() {
    printf("Команды:\n");
    printf("  w <текст>   - записать данные (атомарно)\n");
    printf("  r           - прочитать данные\n");
    printf("  recover     - принудительное восстановление\n");
    printf("  crash       - имитировать сбой (прервать программу)\n");
    printf("  exit        - выход\n");
}

int main() {
    init_disks();
    // При запуске автоматически восстанавливаемся (на случай предыдущего сбоя)
    recover();
    char line[256];
    char cmd[32];
    print_help();
    while (1) {
        printf("> ");
        if (!fgets(line, sizeof(line), stdin)) break;
        line[strcspn(line, "\n")] = '\0';
        if (strlen(line) == 0) continue;
        // Разбор команды
        char *space = strchr(line, ' ');
        if (space) {
            *space = '\0';
            strcpy(cmd, line);
            char *arg = space+1;
            if (strcmp(cmd, "w") == 0) {
                write_stable(arg, strlen(arg));
            } else if (strcmp(cmd, "recover") == 0) {
                recover();
            } else {
                printf("Неизвестная команда. Введите 'help' для справки.\n");
            }
        } else {
            if (strcmp(line, "r") == 0) {
                char buf[DATA_SIZE+1];
                read_stable(buf, sizeof(buf));
            } else if (strcmp(line, "recover") == 0) {
                recover();
            } else if (strcmp(line, "crash") == 0) {
                crash();
            } else if (strcmp(line, "exit") == 0) {
                break;
            } else if (strcmp(line, "help") == 0) {
                print_help();
            } else {
                printf("Неизвестная команда. Введите 'help' для справки.\n");
            }
        }
    }
    return 0;
}
