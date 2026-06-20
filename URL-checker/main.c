#include <stdio.h>
#include <pthread.h>
#include "url.h"

/* 
 * Многопоточная проверка доступности URL из текстового файла
 * Код может обрабатывать как одиночные URL, так и файлы
 * Программа реализована при помощи потоков. Один поток-писатель
 * читает и парсит URL, несколько потоков-читателей проверяют TCP-подключение
 * Создано в рамках изучения системного и сетевого программирования для UNIX
 * Исходные файлы содержат очень подробные комментарии для меня же и других 
 * людей, изучающих программирование. Переносимость на windows отсувствует
*/

/* escape-коды для улучшения читаемости вывода */
#define BOLD "\033[1m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define RESET "\033[0m"

/* статический кольцевой буфер структур url_info */
static buffer_t buffer;

/* инициализация мьютекса и условных переменных */
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t not_full = PTHREAD_COND_INITIALIZER;
pthread_cond_t full = PTHREAD_COND_INITIALIZER;

/* Глобальный флаг сообщающий о конце файла */
int writer_done = 0;

/* максимальный размер строки */
#define MAXLINE 1000

/* флаг для обозначения файла */
#define FLAG "-f"

/* функция для потока писателя */
void *writer(void *arg) {
    FILE *file = (FILE *)arg;
    char line[MAXLINE];
    while (fgets(line, MAXLINE, file)) {
        line[strcspn(line, "\n")] = '\0';
        url_info data;
        if (parse_url(&data, line) != 0)
            continue;
        /* вход в критическую область */
        pthread_mutex_lock(&mutex);
        /* ожидаем и уступаем мьютекс, если буфер заполнен */
        while(buffer.count == BUFFSIZE)
            pthread_cond_wait(&not_full, &mutex);
        putBuffer(&buffer, &data);
        /* отправляем сигнал о наличии данных в буфере */
        pthread_cond_signal(&full);
        /* выход из критической области */
        pthread_mutex_unlock(&mutex);
    }

    /* дошли до конца файла */
    pthread_mutex_lock(&mutex);
    writer_done = 1;
    /* будим всех читателей */
    pthread_cond_broadcast(&full);
    pthread_mutex_unlock(&mutex);

    return NULL;
}

/* функция для потока читателя */
void *reader(void *arg) {
    while (1) {
    url_info data;
    /* вход в критическую область, и ожидание данных в буфере */
    pthread_mutex_lock(&mutex);
    while(!buffer.count) {
        if (writer_done) { // если писатель закончил, то ждать нечего
            pthread_mutex_unlock(&mutex);
            return NULL;
        }
        pthread_cond_wait(&full, &mutex);
    }
    /* забираем данные и будим писателя */
    getBuffer(&buffer, &data);
    pthread_cond_signal(&not_full);
    pthread_mutex_unlock(&mutex);
    int res = connect_to_host(data.host, data.port);
    const char *state = (res == 0) ? GREEN : RED;
    printf("%s://%s%s%s:%d - %s%s%s\n",
            data.scheme, BOLD,
            data.host, RESET,
            data.port,
            state,(res == 0) ? "valid" : "invalid", RESET);
    }
    return NULL;
}

int main(int argc, char **argv) {

    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Usage: %s [-f file] [URL]\n", argv[0]);
        return 1;
    }

    if (argc == 2) { /* только URL */
        url_info data;
        if (parse_url(&data, argv[1]) != 0) {
            fprintf(stderr, "%s: invalid URL format %s\n", argv[0], argv[1]);
            return 1;
        }
        int res = connect_to_host(data.host, data.port);
        const char *state = (res == 0) ? GREEN : RED;
        printf("%s://%s%s%s:%d - %s%s%s\n",
            data.scheme, BOLD,
            data.host, RESET,
            data.port,
            state,(res == 0) ? "valid" : "invalid", RESET);
        return 0;
    }

    /* в параметрах командной строки есть файл */
    if (strcmp(argv[1], FLAG) != 0) {
        fprintf(stderr, "%s: unknown flag %s\n", argv[0], argv[1]);
        return 1;
    }

    FILE *file = fopen(argv[2], "r");
    if (!file) {
        perror("fopen");
        return 1;
    }

    pthread_t w1, r1, r2;

    initBuffer(&buffer);

    /* создаем потоки */
    pthread_create(&w1, NULL, writer, file);
    pthread_create(&r1, NULL, reader, NULL);
    pthread_create(&r2, NULL, reader, NULL);

    /* ожидаем их завершения */
    pthread_join(w1, NULL);
    pthread_join(r1, NULL);
    pthread_join(r2, NULL);

    fclose(file);
    return 0;
}
