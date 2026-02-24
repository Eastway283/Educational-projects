#include "array.h"
#include <raylib.h>
#include <raymath.h>
#include <stdlib.h>
#include <stdio.h>

/* ====================================================================================================================
   Данный модуль реализует логику и обработку массива
   Каждый фрагмент массива представлен в виде структуры, которая хранит в себе прямоугольник, цвет и значение
   Параметры массива задаются статическими переменными, которые учавствуют в обработке массива
   В случае необходимости, лимиты можно изменить. Однако, придется модифицировать остальные модули
   Все прямоугольники храняться в динамическом массиве, который автоматически пересоздается при повторной инициализации
   Требует явной очистки в конце основной программы
   ==================================================================================================================== */

/* Параметры области отображения массив */
static float rectWidth = 0.0f;
static float viewX = 0.0f;
static float viewY = 0.0f;
static float viewWidth = 0.0f;
static float viewHeight = 0.0f;

/* Параметры самого массива */
static SeqRect* rectangles = NULL;
static int rectCount = 0;
static const int minCapacity = 4;
static const int maxCapacity = 50;
static const int MaxValue = 100;
static const int MinValue = 1;

/* Индексы для подсветки */
static int highlightIndex1 = -1;
static int highlightIndex2 = -1;

/* Вспомогательная функция, для обновления позиции элемента по индексу */
static void UpdateRectPosition(int index)
{
    float height = Remap((float) rectangles[index].value, MinValue, MaxValue, 0.0f, viewHeight);

    rectangles[index].rect.x = viewX + index * rectWidth;
    rectangles[index].rect.y = viewY + viewHeight - height;
    rectangles[index].rect.height = height;
}

/* Инициализация массива */
void InitArray(int count, float vx, float vy, float vw, float vh)
{
    if (rectangles)
        FreeArray();

    rectCount = count;
    viewX = vx;
    viewY = vy;
    viewHeight = vh;
    viewWidth = vw;

    rectWidth = vw / count;

    rectangles = (SeqRect *)RL_CALLOC(count, sizeof(SeqRect));

    for (int i = 0; i < rectCount; i++)
    {
        rectangles[i].value = GetRandomValue(MinValue, MaxValue);
        float height = Remap((float)rectangles[i].value, MinValue, MaxValue, 0.0f, viewHeight);

        rectangles[i].rect = (Rectangle) {
            viewX + i * rectWidth,
            viewY + viewHeight - height,
            rectWidth,
            height
        };
        rectangles[i].color = LIGHTGRAY;
    }
}

void FreeArray(void)
{
    if (rectangles)
    {
        RL_FREE(rectangles);
        rectangles = NULL;
        rectCount = 0;
    }
}

/* Перетасовка элементов массива в случайном порядке */
void ShuffleArray(void)
{
    int* seq = LoadRandomSequence(rectCount, 0, rectCount - 1);

    for (int i = 0; i < rectCount; i++)
    {
        SeqRect* r1 = &rectangles[i];
        SeqRect* r2 = &rectangles[seq[i]];

        int temp = r1->value;
        r1->value = r2->value;

        r2->value = temp;
    }

    for(int i = 0; i < rectCount; i++)
        UpdateRectPosition(i);

    UnloadRandomSequence(seq);
}

/* Рисуем массив, если индекс подсветки совпадает с номером прямоугольника - заполняем красным */
void DrawArray(void)
{
    for (int i = 0; i < rectCount; i++)
    {
        if(i == highlightIndex1 || i == highlightIndex2)
            DrawRectangleRec(rectangles[i].rect, MAROON);
        else
            DrawRectangleRec(rectangles[i].rect, rectangles[i].color);

        DrawRectangleLinesEx(rectangles[i].rect, 1.0f, BLACK);
    }
}

int GetArraySize(void)
{
    return rectCount;
}

/* Обмен между двумя элементами массива, необходимо для облегчения сортировки */
void SwapElements(int i, int j)
{
    if (i == j || i < 0 || j < 0 || i >= rectCount || j >= rectCount)
        return;

    int temp = rectangles[i].value;
    rectangles[i].value = rectangles[j].value;
    rectangles[j].value = temp;

    UpdateRectPosition(i);
    UpdateRectPosition(j);
}

int GetValue(int index)
{
    if (index < 0 || index >= rectCount)
        return -1;

    return rectangles[index].value;
}

/* Функции для подсветки */
void SetHighlightIndices(int index1, int index2)
{
    highlightIndex1 = index1;
    highlightIndex2 = index2;
}

void ClearHighlights(void)
{
    highlightIndex1 = highlightIndex2 = -1;
}

/* Функции для сохранения и загрузки массива */

bool SaveArray(const char *filename)
{
    if (rectangles == NULL || rectCount == 0)
        return false;

    FILE* f = fopen(filename, "w");
    if (!f)
    {
        printf("Error: unable to open the output file %s\n", filename);
        return false;
    }

    fprintf(f, "%d", rectangles[0].value);
    for (int i = 1; i < rectCount; i++)
    {
        fprintf(f, " %d", rectangles[i].value);
    }

    fclose(f);
    return true;
}

bool LoadArray(const char *filename)
{
    FILE* f = fopen(filename, "r");
    if (!f)
    {
        printf("Error: unable to open the input file %s\n", filename);
        return false;
    }

    int* tempArr = (int *)RL_MALLOC(maxCapacity * sizeof(int));
    int count = 0;
    int val;
    while ((fscanf(f, "%d", &val) == 1) && count < maxCapacity)
    {
        tempArr[count++] = val;
    }

    fclose(f);

    if (count == 0)
    {
        printf("File %s is empty\n", filename);
        RL_FREE(tempArr);
        return false;
    }

    if (count < minCapacity)
    {
        printf("Too few elements, minimum size: %d\n", minCapacity);
        RL_FREE(tempArr);
        return false;
    }

    for (int i = 0; i < count; i++)
    {
        if (tempArr[i] < MinValue || MaxValue < tempArr[i])
        {
            printf("Error: the sequence contains invalid elements\n");
            RL_FREE(tempArr);
            return false;
        }
    }

    if (rectangles)
        FreeArray();


    rectCount = count;

    rectWidth = viewWidth / count;

    rectangles = (SeqRect *)RL_CALLOC(count, sizeof(SeqRect));

    for (int i = 0; i < rectCount; i++)
    {
        rectangles[i].value = tempArr[i];
        float height = Remap((float)rectangles[i].value, MinValue, MaxValue, 0.0f, viewHeight);

        rectangles[i].rect = (Rectangle) {
            viewX + i * rectWidth,
            viewY + viewHeight - height,
            rectWidth,
            height
        };
        rectangles[i].color = LIGHTGRAY;
    }

    RL_FREE(tempArr);
    return true;
}
