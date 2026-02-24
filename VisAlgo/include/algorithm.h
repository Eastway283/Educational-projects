#ifndef ALGORITHM_H
#define ALGORITHM_H

#include <stdbool.h>

/* Тип алгоритма, для массива */
typedef enum {
    ALGO_NONE,
    ALGO_BUBBLE,
    ALGO_SHAKE,
    ALGO_INSERT,
    ALGO_SHELL,
    ALGO_LINEAR,
    ALGO_BINARY
} AlgorithmType;

/* Запуск работы алгоритма в зависимости от типа */
void StartAlgorithm(AlgorithmType type);

/* Остановка текущего алгоритма */
void StopAlgorithm(void);

/* Получение статуса алгоритма */
bool isAlgorithmRunning(void);

/* Шаг алгоритма, возвращает true, если массив отсортирован */
bool StepAlgorithm(void);

/* Функция для получения целевого элемента поиска */
int GetSearchTarget(void);

/* Получение текущего типа алгоритма */
AlgorithmType GetCurrentAlgorithm(void);

#endif
