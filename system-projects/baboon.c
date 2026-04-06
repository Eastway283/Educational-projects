// Этот код содержит решение задания №40 главы 6 книги "Современные операционные системы" Эндрю Таненбаума
// В нем используется системно-зависимая библиоткеа pthread, а также unistd, переносимость на windows не гарантируется

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

#define ITERATIONS 5

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t west_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t east_cond = PTHREAD_COND_INITIALIZER;

int west_count = 0; //Счетчик бабуинов на запад
int east_count = 0; //Счетчик бабуинов на восток

void cross_west(int id)
{
    pthread_mutex_lock(&mutex);
    while (east_count > 0)
        pthread_cond_wait(&west_cond, &mutex);
    west_count++;
    pthread_mutex_unlock(&mutex);

    //Пересечение каньона (имитация)
    printf("West baboon %d is crossind\n", id);
    usleep(rand() % 100000);
    
    pthread_mutex_lock(&mutex);
    west_count--;
    if (west_count == 0)
        pthread_cond_broadcast(&east_cond);

    pthread_mutex_unlock(&mutex);
}

void cross_east(int id)
{
    pthread_mutex_lock(&mutex);
    while (west_count > 0)
        pthread_cond_wait(&east_cond, &mutex);
    east_count++;
    pthread_mutex_unlock(&mutex);

    //Пересечение каньона (имитация)
    printf("East baboon %d is crossing\n", id);
    usleep(rand() % 100000);

    pthread_mutex_lock(&mutex);
    east_count--;
    if (east_count == 0)
        pthread_cond_broadcast(&west_cond);

    pthread_mutex_unlock(&mutex);
}

void *baboon_w(void *arg)
{
    int id = *(int *)arg;
    for (int i = 0; i < ITERATIONS; i++) {
        cross_west(id);
        usleep(rand() % 200000);
    }
    return NULL;
}

void *baboon_e(void *arg)
{
    int id = *(int *)arg;
    for (int i = 0; i < ITERATIONS; i++) {
        cross_east(id);
        usleep(rand() % 200000);
    }

    return NULL;
}

int main(void)
{
    srand(time(NULL));
    pthread_t w1, w2, e1, e2;
    int id1 = 1, id2 = 2, id3 = 3, id4 = 4;

    pthread_create(&w1, NULL, baboon_w, &id1);
    pthread_create(&w2, NULL, baboon_w, &id2);
    pthread_create(&e1, NULL, baboon_e, &id3);
    pthread_create(&e2, NULL, baboon_e, &id4);

    pthread_join(w1, NULL);
    pthread_join(w2, NULL);
    pthread_join(e1, NULL); 
    pthread_join(e2, NULL);

    return 0;
}
