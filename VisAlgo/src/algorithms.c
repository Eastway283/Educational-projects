#include "../include/algorithm.h"
#include "../include/array.h"
#include "raylib.h"
#include <stdbool.h>
#include <stdio.h>


/* ========================================================================
   Модуль пошаговых алгоритмов сортировки и поиска
   Каждый алгоритм реализован в виде конечного автомата
   Выполняет шаг и обновляет переменные состояния
   Возвращаемое значение true означает, что алгоритм завершен
   Переменные состояний храняться как static и сохраняются между вызовами
   Это позволяет останавливать и продолжать алгоритм с того же места
   ======================================================================== */


/* Стартовое состояние алгоритма */
static AlgorithmType currentAlgo = ALGO_NONE;
static bool running = false;

/* Переменные для проверки и работы пошаговых алгоритмов */

/* Сортировка пузырьком */
static int bubble_i, bubble_j;
static bool bubble_swapped;

/* Шейкерная сортировка */
static int shaker_left, shaker_right, shaker_index;
static bool shaker_forward; /* true - сортировка вперед, false - назад */
static bool shaker_swapped;

/* Сортировка вставками */
static int ins_i, ins_j;

/* Сортировка Шелла */
static int shell_gap;
static int shell_i, shell_j;
static bool shell_swapped;

/* Линейный поиск */
static int linear_i, linear_target;
static bool linear_found;

/* Бинарный поиск */
static int binary_left, binary_right, binary_mid;
static int binary_target;
static bool binary_found;

/* Функция для бинарного поиска, т.к. он требует отсортированного массива */
static bool isArraySorted(void)
{
    int size = GetArraySize();
    for (int i = 0; i < size - 1; i++)
    {
        if (GetValue(i) > GetValue(i + 1))
            return false;
    }
    return true;
}

void StartAlgorithm(AlgorithmType type)
{
    StopAlgorithm(); /* Останавливаем алгоритм */

    currentAlgo = type;
    running = true;

    /* В зависимости от типа, устанавливаем значения и подсветку */
    switch(type)
    {
        case ALGO_BUBBLE:
            bubble_i = 0;
            bubble_j = 0;
            bubble_swapped = false;
            SetHighlightIndices(0, 1);
            break;

        case ALGO_SHAKE:
            shaker_left = 0;
            shaker_right = GetArraySize() - 1;
            shaker_index = shaker_left;
            shaker_forward = true;
            shaker_swapped = false;
            SetHighlightIndices(shaker_left, shaker_left + 1);
            break;

        case ALGO_INSERT:
            ins_i = 1;
            ins_j = 0;
            SetHighlightIndices(1, 0);
            break;

        case ALGO_SHELL:
            shell_gap = GetArraySize() / 2;
            shell_i = shell_gap;
            shell_j = shell_i - shell_gap;
            shell_swapped = false;
            SetHighlightIndices(shell_i, shell_j);
            break;

        case ALGO_LINEAR:
            linear_i = 0;
            linear_target = GetValue(GetRandomValue(0, GetArraySize() - 1));
            linear_found = false;
            SetHighlightIndices(linear_i, -1);
            break;
        
        case ALGO_BINARY:
            if (!isArraySorted())
            {
                printf("Array is not sorted. Cannot perform binary search\n");
                running = false;
                return;
            }
            binary_left = 0;
            binary_right = GetArraySize() - 1;
            binary_target = GetValue(GetRandomValue(0, GetArraySize() - 1));
            binary_found = false;
            binary_mid = -1;
            SetHighlightIndices(-1, -1);
            break;

        default:
            running = false;
            break;
    }
}

/* Останавливаем алгоритм и снимаем подсветку */
void StopAlgorithm(void)
{
    running = false;
    currentAlgo = ALGO_NONE;
    ClearHighlights();
}

bool isAlgorithmRunning(void)
{
    return running;
}

int GetSearchTarget(void)
{
    if (currentAlgo == ALGO_LINEAR)
        return linear_target;

    if (currentAlgo == ALGO_BINARY)
        return binary_target;

    return -1;
}

AlgorithmType GetCurrentAlgorithm(void)
{
    return currentAlgo;
}

/* Реализация пошаговых алгоритмов */

static bool stepBubble(void)
{
    int size = GetArraySize();
    if (size <= 1)
        return true;

    /* Если прошли все проходы - завершаем */
    if (bubble_i >= size - 1)
        return true;

    /* Если в текущем проходе дошли до конца массива */
    if (bubble_j >= size - bubble_i - 1)
    {
        if (!bubble_swapped)
            return true;

        ++bubble_i;
        bubble_j = 0;
        bubble_swapped = false;
        if (bubble_i >= size - 1)
            return true;
    }

    int a = GetValue(bubble_j);
    int b = GetValue(bubble_j + 1);

    if (a > b)
    {
        SwapElements(bubble_j, bubble_j + 1);
        bubble_swapped = true;
    }

    SetHighlightIndices(bubble_j, bubble_j + 1);
    ++bubble_j;
    return false;
}

static bool stepShaker(void)
{
    int size = GetArraySize();
    if (size <= 1)
        return true;

    if (shaker_left >= shaker_right)
        return true;

    if (shaker_forward)
    {
        /* Движение слева направо */
        if (shaker_index < shaker_right)
        {
            if(GetValue(shaker_index) > GetValue(shaker_index + 1))
            {
                SwapElements(shaker_index, shaker_index + 1);
                shaker_swapped = true;
            }
            SetHighlightIndices(shaker_index, shaker_index + 1);
            ++shaker_index;
        }
        else
        {
            /* Достигли правого края */
            if (!shaker_swapped)
                return true;
            --shaker_right;
            shaker_forward = false;
            shaker_index = shaker_right;
            shaker_swapped = false;
        }
    }
    else 
    {
        /* Движение справа налево */
        if (shaker_index > shaker_left)
        {
            if (GetValue(shaker_index - 1) > GetValue(shaker_index))
            {
                SwapElements(shaker_index - 1, shaker_index);
                shaker_swapped = true;
            }
            SetHighlightIndices(shaker_index - 1, shaker_index);
            --shaker_index;
        }
        else 
        {
            /* Достигли левого края */
            if (!shaker_swapped)
                return true;
            ++shaker_left;
            shaker_forward = true;
            shaker_index = shaker_left;
            shaker_swapped = false;
        }
    }
    return false;
}

static bool stepInsertion(void)
{
    int size = GetArraySize();
    if (size <= 1) 
        return true;

    if(ins_i >= size)
        return true;

    if (ins_j >= 0 && GetValue(ins_j) > GetValue(ins_j + 1))
    {
        SwapElements(ins_j, ins_j + 1);
        --ins_j;
        SetHighlightIndices(ins_j + 1, ins_j);
    }
    else
    {
        ++ins_i;
        if (ins_i < size)
        {
            ins_j = ins_i - 1;
            SetHighlightIndices(ins_i, ins_j);
        }
        else 
        {
            return true;
        }
    }

    return false;
}

static bool stepShell(void)
{
    int size = GetArraySize();
    if (size <= 1)
        return true;

    if (shell_gap <= 0)
        return true;

    if (shell_i >= size)
    {
        shell_gap /= 2;

        if (shell_gap <= 0)
            return true;

        shell_i = shell_gap;
        shell_j = shell_i - shell_gap;
        SetHighlightIndices(shell_i, shell_j);
        return false;
    }

    if (shell_j >= 0 && GetValue(shell_j) > GetValue(shell_j + shell_gap))
    {
        SwapElements(shell_j, shell_j + shell_gap);
        shell_j -= shell_gap;
        SetHighlightIndices(shell_j, shell_j + shell_gap);
    }
    else 
    {
        ++shell_i;
        shell_j = shell_i - shell_gap;
        SetHighlightIndices(shell_i, shell_j);
    }

    return false;
}

static bool stepLinear(void)
{
    int size = GetArraySize();
    if (linear_i >= size)
    {
        running = false;
        return true;
    }

    SetHighlightIndices(linear_i, -1);
    if (GetValue(linear_i) == linear_target)
    {
        linear_found = true;
        running = false;
        return true;
    }
    ++linear_i;
    return false;
}

static bool stepBinary(void)
{
    if (binary_left > binary_right)
    {
        running = false;
        return true;
    }

    binary_mid = (binary_left + binary_right) / 2;
    SetHighlightIndices(binary_mid, -1);

    int val = GetValue(binary_mid);
    if (val == binary_target)
    {
        binary_found = true;
        running = false;
        return true;
    }
    else if (val < binary_target)
    {
        binary_left = binary_mid + 1;
    }
    else 
    {
        binary_right = binary_mid - 1;
    }
    
    return false;
}

/* В зависимости от текущего типа алгоритма, делаем шаг, возвращает true, если алгоритм завершен */
bool StepAlgorithm(void)
{
    if (!running)
        return true;

    bool finished = false;
    switch (currentAlgo)
    {
        case ALGO_BUBBLE:
            finished = stepBubble();
            break;

        case ALGO_SHAKE:
            finished = stepShaker();
            break;

        case ALGO_INSERT:
            finished = stepInsertion();
            break;

        case ALGO_SHELL:
            finished = stepShell();
            break;

        case ALGO_LINEAR:
            finished = stepLinear();
            break;

        case ALGO_BINARY:
            finished = stepBinary();
            break;

        default:
            finished = true;
            running = false;
            break;
    }

    return finished;
}
