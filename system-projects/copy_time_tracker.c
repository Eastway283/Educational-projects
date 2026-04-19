#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Измерение времени копирования данных различной длины. Программа сделана в качестве ответа на вопрос
// после главы 8 книги Э.Таненбаума "Современные операционные системы 4-е издание"

#define ITERATIONS 1000
#define MAX_SIZE (1024 * 1024 * 64) // 256 мб

int main(void) {

    int *src, *dst;
    clock_t start, end;
    double copy_time, overhead_time, total_time;

    //Выделим максимальный объем заранее
    src = (int *)malloc(MAX_SIZE * sizeof(int));
    dst = (int *)malloc(MAX_SIZE * sizeof(int));
    if (!src || !dst) {
        perror("malloc");
        exit(1);
    }

    // Заполним исходный массив
    for (size_t i = 0; i < MAX_SIZE; i++)
        src[i] = i;

    printf("Размер (элементов)\tВремя копирования (в мс)\n");

    for (size_t n = 1024; n <= MAX_SIZE; n *= 2) {
        // Измерение накладных расходов
        start = clock();
        for (int iter = 0; iter < ITERATIONS; iter++) {
            volatile int dummy; // Убираем оптимизацию от компилятора
            for (size_t i = 0; i < n; i++)
                dummy = i;
        }
    end = clock();
    overhead_time = (double) (end - start) / CLOCKS_PER_SEC * 1000;

    // Измерение копирования
    start = clock();
    for (int iter = 0; iter < ITERATIONS; iter++) {
        for (size_t i = 0; i < n; i++)
            dst[i] = src[i];
    }
    end = clock();
    total_time = (double) (end - start) / CLOCKS_PER_SEC * 1000;
    copy_time = total_time - overhead_time;

    if (copy_time < 0) copy_time = 0;

    printf("%10zu\t\t%10.2f\n", n, copy_time);
    }

    free(src);
    free(dst);

    return 0;
}
