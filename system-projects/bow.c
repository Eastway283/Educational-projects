#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

/*
 * Задача о двух поклонах
 * Два человека (потока) кланяются друг другу
 * Протокол: первый поклонившийся ждет, пока поклонится второйъ
 * Если кланяются одновременно, то оба остаются в поклоне всегда (livelock)
 *
 * Решение: использовать мьютекс и две условные переменные
 * Каждый поток захватывает мьютекс, устанавливает свой флаг поклона,
 * затем проверяет флаг другого. Если другой еще не поклонился, поток
 * ждет своей условной переменной. Второй поток, увидев флаг первого,
 * выпрямляется и будит первого. Таким образом одновременного поклона
 * не происходи. Блокировка исключена
 */

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;   // первый человек
pthread_cond_t cond2 = PTHREAD_COND_INITIALIZER;   // второй человек

int bowed1 = 0; // флаг: первый поклонился
int bowed2 = 0; // второй поклонился

void* person1(void *arg)
{
    pthread_mutex_lock(&mutex);
    bowed1 = 1;
    printf("Person 1 bows\n");

    if (!bowed2)
        pthread_cond_wait(&cond1, &mutex);

    printf("Person 1 straightens up\n");
    pthread_cond_signal(&cond2);
    pthread_mutex_unlock(&mutex);
    return NULL;
}

void* person2(void *arg)
{
    pthread_mutex_lock(&mutex);
    bowed2 = 1;
    printf("Person 2 bows\n");

    if (!bowed1)
        pthread_cond_wait(&cond2, &mutex);

    printf("Person 2 straightens up\n");
    pthread_cond_signal(&cond1);
    pthread_mutex_unlock(&mutex);
    return NULL;
}

int main(void)
{
    pthread_t p1, p2;

    pthread_create(&p1, NULL, person1, NULL);
    pthread_create(&p2, NULL, person2, NULL);
    pthread_join(p1, NULL);
    pthread_join(p2, NULL);
    return 0;
}

