#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "threadpool.h"

/*
 * Различные задачи для имитации работы
 * print_task - выводит сообщение в терминал
 * delay_task - функция с имитацией задержки
 * factorial_task - функция с нагрузкой на CPU
 * hang_task - задача, которая зависает
 */
void print_task(void *arg) {
    int id = *(int*)arg;
    printf("[Thread %lu] Task %d executed\n", pthread_self(), id);
    free(arg); // если аргумент выделен динамически
}

void delay_task(void *arg) {
    int seconds = *(int*)arg;
    printf("[Thread %lu] Sleeping for %d seconds...\n", pthread_self(), seconds);
    sleep(seconds);
    printf("[Thread %lu] Woke up after %d seconds\n", pthread_self(), seconds);
    free(arg);
}

void factorial_task(void *arg) {
    int n = *(int*)arg;
    long long result = 1;
    for (int i = 2; i <= n; i++) {
        result *= i;
    }
    printf("[Thread %lu] Factorial of %d = %lld\n", pthread_self(), n, result);
    free(arg);
}

void hang_task(void *arg) {
    printf("[Thread %lu] Starting infinite loop...\n", pthread_self());
    while (1) {
        sleep(1); // имитация бесконечной работы
    }
}

/* add_tasks - вспомогательная функция для добавления задачи */
void add_tasks(thread_pool_t *pool, int count, void (*fcn)(void*), int base_arg) {
    for (int i = 0; i < count; i++) {
        int *arg = malloc(sizeof(int));
        *arg = base_arg + i;
        thread_pool_add_task(pool, fcn, arg);
    }
}

int main(void) {

    thread_pool_t pool;
    if (thread_pool_init(&pool, 4) != 0) {
        fprintf(stderr, "Failed to init thread pool\n");
        return 1;
    }

    printf("=== Testing print_task ===\n");
    add_tasks(&pool, 5, print_task, 1);

    printf("=== Testing factorial_task ===\n");
    add_tasks(&pool, 5, factorial_task, 5);

    printf("=== Testing delay_task (3s) ===\n");
    add_tasks(&pool, 2, delay_task, 3);

    // Даём время на выполнение
    sleep(4);

    printf("=== Shutting down ===\n");
    thread_pool_destroy(&pool);
    printf("All tasks completed, pool destroyed.\n");
    return 0;
}
