#ifndef ARRAY_H
#define ARRAY_H

#include "raylib.h"

/* Структура прямоугольника массива */
typedef struct SeqRect {
    Rectangle rect;
    Color color;
    int value;
} SeqRect;

/* Функции для управления массивом */

void InitArray(int rectCount, float viewX, float viewY, float viewWidth, float viewHeight);

void FreeArray(void);

void ShuffleArray(void);

void DrawArray(void);

/* Функции для подсветки текущих элементов */

void SetHighlightIndices(int index1, int index2);

void ClearHighlights(void);

/* Получение размера массива */
int GetArraySize(void);

/* Сохранение или загрузка из файла */
bool SaveArray(const char* filename);

bool LoadArray(const char* filename);

/* Вспомогательные методы для алгоритмов */
void SwapElements(int i, int j);

int GetValue(int index);

#endif
