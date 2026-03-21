#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <string.h>

#define DEFAULT_BUF_SIZE (1024 * 1024)  /* 1 МБ */

typedef struct {
    int fd;
    off_t size;
    off_t start;      /* начальный индекс левой половины */
    off_t end;        /* конечный индекс левой половины (не включая) */
    size_t buf_size;  /* размер буфера для каждого потока */
} thread_arg_t;

void *reverse_part(void *arg) {
    thread_arg_t *data = (thread_arg_t *)arg;
    int fd = data->fd;
    off_t size = data->size;
    off_t left = data->start;
    off_t right = size - 1 - left;   /* правый индекс, соответствующий началу */
    size_t buf_size = data->buf_size;
    unsigned char *buf_left = malloc(buf_size);
    unsigned char *buf_right = malloc(buf_size);
    if (!buf_left || !buf_right) {
        perror("malloc");
        free(buf_left);
        free(buf_right);
        return NULL;
    }

    while (left < data->end && left < right) {
        /* Определяем, сколько байт можно обработать за раз */
        size_t to_process = buf_size;
        if (left + to_process > (off_t)data->end)
            to_process = (size_t)(data->end - left);
        if (left + to_process > right + 1)   /* чтобы не пересечь середину */
            to_process = (size_t)(right + 1 - left);

        if (to_process == 0) break;

        /* Читаем блок слева и блок справа */
        ssize_t n_read_left = pread(fd, buf_left, to_process, left);
        ssize_t n_read_right = pread(fd, buf_right, to_process, right - to_process + 1);
        if (n_read_left != (ssize_t)to_process || n_read_right != (ssize_t)to_process) {
            perror("pread");
            break;
        }

        /* Меняем местами блоки с обратным порядком байтов */
        for (size_t i = 0; i < to_process; i++) {
            unsigned char tmp = buf_left[i];
            buf_left[i] = buf_right[to_process - 1 - i];
            buf_right[to_process - 1 - i] = tmp;
        }

        /* Записываем обратно */
        if (pwrite(fd, buf_left, to_process, left) != (ssize_t)to_process ||
            pwrite(fd, buf_right, to_process, right - to_process + 1) != (ssize_t)to_process) {
            perror("pwrite");
            break;
        }

        left += to_process;
        right -= to_process;
    }

    free(buf_left);
    free(buf_right);
    return NULL;
}

int main(int argc, char *argv[]) {
    int fd;
    off_t size;
    int num_threads = 0;
    char *filename = NULL;
    pthread_t *threads;
    thread_arg_t *args;
    int opt;
    int i;
    off_t half;
    off_t chunk;
    size_t buf_size = DEFAULT_BUF_SIZE;

    /* Разбор параметров */
    while ((opt = getopt(argc, argv, "t:f:b:")) != -1) {
        switch (opt) {
            case 't':
                num_threads = atoi(optarg);
                if (num_threads <= 0) {
                    fprintf(stderr, "Число потоков должно быть положительным\n");
                    exit(EXIT_FAILURE);
                }
                break;
            case 'f':
                filename = optarg;
                break;
            case 'b':
                buf_size = (size_t)atol(optarg);
                if (buf_size == 0) buf_size = DEFAULT_BUF_SIZE;
                break;
            default:
                fprintf(stderr, "Использование: %s -f файл [-t число_потоков] [-b размер_буфера_байт]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (!filename) {
        fprintf(stderr, "Не указан файл. Используйте -f имя_файла\n");
        exit(EXIT_FAILURE);
    }

    fd = open(filename, O_RDWR);
    if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    size = lseek(fd, 0, SEEK_END);
    if (size == -1) {
        perror("lseek");
        close(fd);
        exit(EXIT_FAILURE);
    }

    if (size == 0) {
        printf("Файл пуст, ничего не делаем.\n");
        close(fd);
        return 0;
    }

    half = size / 2;
    if (half == 0) {
        printf("Файл состоит из одного байта, реверсирование не требуется.\n");
        close(fd);
        return 0;
    }

    if (num_threads <= 0) num_threads = 1;
    if ((off_t)num_threads > half) num_threads = (int)half;

    threads = malloc(num_threads * sizeof(pthread_t));
    args = malloc(num_threads * sizeof(thread_arg_t));
    if (!threads || !args) {
        fprintf(stderr, "Ошибка выделения памяти\n");
        close(fd);
        free(threads);
        free(args);
        exit(EXIT_FAILURE);
    }

    chunk = half / num_threads;
    for (i = 0; i < num_threads; i++) {
        args[i].fd = fd;
        args[i].size = size;
        args[i].start = i * chunk;
        args[i].end = (i == num_threads - 1) ? half : (i + 1) * chunk;
        args[i].buf_size = buf_size;
        if (pthread_create(&threads[i], NULL, reverse_part, &args[i]) != 0) {
            perror("pthread_create");
            for (int j = 0; j < i; j++) {
                pthread_cancel(threads[j]);
                pthread_join(threads[j], NULL);
            }
            close(fd);
            free(threads);
            free(args);
            exit(EXIT_FAILURE);
        }
    }

    for (i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Реверсирование файла '%s' (размер %ld байт) выполнено с буферами %zu байт.\n",
           filename, (long)size, buf_size);
    close(fd);
    free(threads);
    free(args);
    return 0;
}
