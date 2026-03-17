#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>

#define READERS_COUNT 3
#define WRITERS_COUNT 2
#define ITERATIONS 5

int shared_data = 0;
int rc = 0;

sem_t mutex;
sem_t wsem;

void* reader(void *arg)
{
    int id = *(int *)arg;
    for (int i = 0; i < ITERATIONS; i++) {
        sem_wait(&mutex);
        rc++;
        if (rc == 1)
            sem_wait(&wsem);
        sem_post(&mutex);

        printf("Reader %d read %d\n", id, shared_data);
        usleep(rand() % 100000);

        sem_wait(&mutex);
        rc--;
        if (rc == 0)
            sem_post(&wsem);
        sem_post(&mutex);

        usleep(rand() % 200000);
    }
    return NULL;
}

void* writer(void *arg)
{
    int id = *(int *)arg;
    for (int i = 0; i < ITERATIONS; i++) {
        int new_value = rand() % 100;
        usleep(rand() % 100000);

        sem_wait(&wsem);

        shared_data = new_value;
        printf("Writer %d write new value %d\n", id, shared_data);
        usleep(rand() % 100000);

        sem_post(&wsem);
        usleep(rand() % 200000);
    }
    return NULL;
}

int main() {
    srand(time(NULL));
    
    // Инициализация семафоров
    sem_init(&mutex, 0, 1);  // двоичный семафор (как мьютекс)
    sem_init(&wsem, 0, 1);   // двоичный семафор для доступа к БД
    
    pthread_t readers[READERS_COUNT], writers[WRITERS_COUNT];
    int reader_ids[READERS_COUNT], writer_ids[WRITERS_COUNT];
    
    // Создание потоков-читателей
    for (int i = 0; i < READERS_COUNT; i++) {
        reader_ids[i] = i + 1;
        pthread_create(&readers[i], NULL, reader, &reader_ids[i]);
    }
    
    // Создание потоков-писателей
    for (int i = 0; i < WRITERS_COUNT; i++) {
        writer_ids[i] = i + 1;
        pthread_create(&writers[i], NULL, writer, &writer_ids[i]);
    }
    
    // Ожидание завершения всех потоков
    for (int i = 0; i < READERS_COUNT; i++) {
        pthread_join(readers[i], NULL);
    }
    for (int i = 0; i < WRITERS_COUNT; i++) {
        pthread_join(writers[i], NULL);
    }
    
    // Уничтожение семафоров
    sem_destroy(&mutex);
    sem_destroy(&wsem);
    
    return 0;
}
