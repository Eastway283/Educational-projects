#include <stdlib.h>
#include <pthread.h>
#include "threadpool.h"

// worker - базовая функция-исполнитель потока
// принимаем в thread_pool_t, извлекает и выполняет task
// доступ к списку задач атомарен
void *worker(void *arg) {
    thread_pool_t *pool = (thread_pool_t *)arg;
    while (1) {
        pthread_mutex_lock(&pool->lock);
        while (pool->list_head == NULL && !pool->stop)
            pthread_cond_wait(&pool->cond, &pool->lock);
        if (pool->stop && pool->list_head == NULL) {
            pthread_mutex_unlock(&pool->lock);
            break;
        }
        // извлечение задачи
        task_t *task = pool->list_head;
        pool->list_head = task->next;
        if (pool->list_head == NULL)
            pool->list_tail = NULL;
        pthread_mutex_unlock(&pool->lock);
        // выполнение задачи
        task->fcn(task->arg);
        free(task);
    }
    return NULL;
}

// thread_pool_init - инициализация pool с количеством потоков num_threads
// -1 в случае ошибки, иначе 0
int thread_pool_init(thread_pool_t *pool, size_t num_threads) {
    pool->threads = (pthread_t *)malloc(sizeof(pthread_t) * num_threads);
    if (!pool->threads)
        return -1;
    pool->list_head = pool->list_tail = NULL;
    pool->num_threads = num_threads;
    pool->stop = 0;
    if (pthread_mutex_init(&pool->lock, NULL) != 0) {
        free(pool->threads);
        return -1;
    }
    if (pthread_cond_init(&pool->cond, NULL) != 0) {
        free(pool->threads);
        return -1;
    }
    // инициализация каждого потока
    for (int i = 0; i < num_threads; i++) {
        if (pthread_create(&pool->threads[i], NULL, worker, pool) != 0) {
            pool->stop = 1;
            pthread_cond_broadcast(&pool->cond);
            for (int j = 0; j < num_threads; j++)
                pthread_join(pool->threads[j], NULL);
            pthread_mutex_destroy(&pool->lock);
            pthread_cond_destroy(&pool->cond);
            free(pool->threads);
            return -1;
        }

    }

    return 0;
}

// thread_pool_destroy - уничтожение и обнуление всех членов pool
int thread_pool_destroy(thread_pool_t *pool) {
    // Сигнал остановки
    pthread_mutex_lock(&pool->lock);
    pool->stop = 1;
    pthread_cond_broadcast(&pool->cond);
    pthread_mutex_unlock(&pool->lock);
    // Ожидание завершения каждого потока
    for (int i = 0; i < pool->num_threads; i++)
        pthread_join(pool->threads[i], NULL);
    pthread_mutex_destroy(&pool->lock);
    pthread_cond_destroy(&pool->cond);
    free(pool->threads);
    // очистка списка задач
    task_t *temp = pool->list_head;
    task_t *next;
    while (temp) {
        next = temp->next;
        free(temp);
        temp = next;
    }
    pool->list_head = pool->list_tail = NULL;
    pool->num_threads = 0;
    return 1;
}

// thread_pool_add_task - добавление задачи в список пула
// отрицательное число в случае ошибки, иначе 0
// доступ к списку атомарен
int thread_pool_add_task(thread_pool_t *pool, void (*fcn) (void *), void *arg) {
    task_t *temp = (task_t *)malloc(sizeof(task_t));
    if (!temp)
        return -1;
    temp->fcn = fcn;
    temp->arg = arg;
    temp->next = NULL;
    // добавляем задачу в конец списка
    pthread_mutex_lock(&pool->lock);
    // проверка на работу пула
    if (pool->stop) {
        pthread_mutex_unlock(&pool->lock);
        free(temp);
        return -2;
    }
    if (!pool->list_head)
        pool->list_head = pool->list_tail = temp;
    else {
    pool->list_tail->next = temp;
    pool->list_tail = temp;
    }
    pthread_cond_signal(&pool->cond);
    pthread_mutex_unlock(&pool->lock);
    return 0;
}
