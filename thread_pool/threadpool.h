#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <pthread.h>

// структура задачи. Содержит функция для выполнения,
// аргумент функции и указатель на следующую задачу
typedef struct task {
    void (*fcn) (void *arg);
    void *arg;
    struct task *next;
} task_t;

// структура пула потоков для выполнения различных задач
typedef struct thread_pool {
    pthread_t *threads;   // массив потоков
    task_t *list_head;    // начало очереди задач
    task_t *list_tail;    // конец очереди задач
    pthread_mutex_t lock; // мьютекс для доступа к списку
    pthread_cond_t cond;  // cond для уведомлений
    size_t num_threads;   // кол-во потоков
    int stop;             // флаг остановки
} thread_pool_t;

void *worker(void *arg);

int thread_pool_init(thread_pool_t *pool, size_t num_threads);

int thread_pool_destroy(thread_pool_t *pool); 

int thread_pool_add_task(thread_pool_t *pool, void (*fcn) (void *), void *arg);

#endif // THREADPOOL_H
