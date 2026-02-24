#include "structures.h"
#include "raylib.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

/* ==============================================================================================================
   Этот модуль содержит в себе реализацию логики и обработки структур, таких как стек и очередь
   Обе структуры представлены в виде блоков, хранящих в себе значение. Верхушка стека заливается красным
   В очереди: голова - красный, хвост - синий. Аналогично другим модулям все переменные имеют модификатор static,
   что в свою очередь позволяет хранить их значения между вызовами и инкапсулировать логику работы и обработки
   Геометрия элементов расчитывается в зависимости от размеров рабочей области. 
   ============================================================================================================== */

#define MINVALUE 1
#define MAXVALUE 100

#define OFFSET 10

/* Элемент структуры */
typedef struct {
    Rectangle rect;
    Color color;
    int value;
} StructElem;

/* ============================ Стек ==============================*/
static StructElem* stack = NULL;
static int stackCapacity = 0;
static int stackTop = -1;
static float stackX, stackY, stackW, stackH;
static bool clearingStack = false;
static bool fillingStack = false;



/* Вспомогательная функция для пересчета позиций стека */
static void UpdateStackPosition(void)
{
    float startX = stackX + OFFSET;
    float elemWidth = (stackW - 2 * OFFSET) / stackCapacity;
    for (int i = 0; i <= stackTop; i++)
    {
        stack[i].rect = (Rectangle) {
            .x = startX + i * elemWidth,
            .y = stackY + OFFSET,
            .width = elemWidth,
            .height = stackH - 2 * OFFSET
        };
    }
}

/* Создание стека и установка позиций отрисовки        */
/* Дополнительно сбрасываем флаги очистки и заполнения */
void InitStack(float x_pos, float y_pos, float width, float height, int capacity)
{
    FreeStack();

    stackX = x_pos;
    stackY = y_pos;
    stackW = width;
    stackH = height;
    stackCapacity = capacity;
    stackTop = -1;
    
    stack = (StructElem *)RL_CALLOC(capacity, sizeof(StructElem));
    for (int i = 0; i < capacity; i++)
    {
        stack[i].color = LIGHTGRAY;
        stack[i].value = 0;
    }

    clearingStack = false;
    fillingStack = false;
}

void FreeStack(void)
{
    if (stack)
    {
        RL_FREE(stack);
        stack = NULL;
        stackCapacity = 0;
        stackTop = -1;
    }
}

/* Если размер позволяет, то заполняем случайным числом и обновляем состояние стека */
bool PushStack(void)
{
    if (stackTop >= stackCapacity - 1)
    {
        printf("Stack overflow\n");
        return false;
    }

    ++stackTop;
    stack[stackTop].value = GetRandomValue(MINVALUE, MAXVALUE);
    stack[stackTop].color = LIGHTGRAY;
    UpdateStackPosition();
    return true;
}

/* Аналогично, но с удалением */
bool PopStack(void)
{
    if (stackTop < 0)
    {
        printf("Stack underflow\n");
        return false;
    }

    --stackTop;
    UpdateStackPosition();
    return true;
}

/* Установка флага для автоматической очистки */
void StartClearStack(void)
{
    if (stackTop >= 0)
        clearingStack = true;
}

/* Делаем шаг при автоматической очистке, пока возвращает true при полной очистке */
bool StepClearStack(void)
{
    if (!clearingStack)
        return true;

    if (stackTop >= 0)
    {
        --stackTop;
        UpdateStackPosition();
    }
    
    if (stackTop < 0)
    {
        clearingStack = false;
        return true;
    }

    return false;
}

bool IsClearingStack(void)
{
    return clearingStack;
}

void StartFillStack(void)
{
    if (stackTop < stackCapacity - 1)
        fillingStack = true;
}

/* Аналогично автоматическому удалению */
bool StepFillStack(void)
{
    if (!fillingStack)
        return true;

    if (stackTop < stackCapacity - 1)
    {
        PushStack();
        return false;
    }
    else 
    {
        fillingStack = false;
        return true;
    }
}

bool IsFillingStack(void)
{
    return fillingStack;
}

int StackSize(void)
{
    return stackTop + 1;
}

/* Рисуем стек, если верхушка стека - рисуем красным */
void DrawStack(void)
{
    if (!stack)
        return;

    for (int i = 0; i <= stackTop; i++)
    {
        Color fillColor = (i == stackTop) ? MAROON : stack[i].color;
        DrawRectangleRec(stack[i].rect, fillColor);
        DrawRectangleLinesEx(stack[i].rect, 1.0f, DARKGRAY);

        char text[16];
        sprintf(text, "%d", stack[i].value);

        int fontSize = 20;
        Vector2 size = MeasureTextEx(GetFontDefault(), text, fontSize, 1);
        float tx = stack[i].rect.x + (stack[i].rect.width - size.x) / 2;
        float ty = stack[i].rect.y + (stack[i].rect.height - size.y) / 2;
        DrawText(text, tx, ty, fontSize, BLACK);
    }
}

/* ============================== Очередь =========================== */

static StructElem* queue = NULL;
static int queueCapacity = 0;
static int queueHead = 0;
static int queueTail = 0;
static int queueCount = 0;
static float queueX, queueY, queueW, queueH;
static bool clearingQueue = false;
static bool fillingQueue = false;

static void UpdateQueuePosition(void)
{
    float startX = queueX + OFFSET;
    float elemWidth = (queueW - 2 * OFFSET) / queueCapacity;

    for (int i = 0; i < queueCount; i++)
    {
        int idx = (queueHead + i) % queueCapacity;
        queue[idx].rect = (Rectangle) {
            .x = startX + i * elemWidth,
            .y = queueY + OFFSET,
            .width = elemWidth,
            .height = queueH - 2 * OFFSET
        };
    }
}

void InitQueue(float x, float y, float w, float h, int capacity)
{
    FreeQueue();

    queueX = x;
    queueY = y;
    queueW = w;
    queueH = h;
    queueCapacity = capacity;
    queueHead = queueTail = queueCount = 0;

    queue = (StructElem *)RL_CALLOC(capacity, sizeof(StructElem));
    for (int i = 0; i < capacity; i++)
    {
        queue[i].color = LIGHTGRAY;
        queue[i].value = 0;
    }

    clearingQueue = false;
    fillingQueue = false;
}

void FreeQueue(void)
{
    if (queue)
    {
        RL_FREE(queue);
        queue = NULL;
        queueCapacity = 0;
        queueHead = queueTail = queueCount = 0;
    }
}

bool EnqueueQueue (void)
{
    if (queueCount >= queueCapacity)
    {
        printf("Queue overflow\n");
        return false;
    }

    queue[queueTail].value = GetRandomValue(MINVALUE, MAXVALUE);
    queue[queueTail].color = LIGHTGRAY;
    queueTail = (queueTail + 1) % queueCapacity;
    ++queueCount;
    UpdateQueuePosition();

    return true;
}

bool DequeueQueue(void)
{
    if (queueCount == 0)
    {
        printf("Queue underflow\n");
        return false;
    }

    queueHead = (queueHead + 1) % queueCapacity;
    --queueCount;
    UpdateQueuePosition();

    return true;
}

void StartClearQueue(void)
{
    if (queueCount > 0)
        clearingQueue = true;
}

bool StepClearQueue(void)
{
    if (!clearingQueue)
        return true;

    if (queueCount > 0)
    {
        /* Удаляем элемент из головы */
        queueHead = (queueHead + 1) % queueCapacity;
        --queueCount;
        UpdateQueuePosition();
    }
    if (queueCount == 0)
    {
        clearingQueue = false;
        return true;
    }

    return false;
}

bool IsClearingQueue(void)
{
    return clearingQueue;
}

void StartFillQueue(void)
{
    if (queueCount < queueCapacity)
        fillingQueue = true;
}

bool StepFillQueue(void)
{
    if (!fillingQueue)
        return true;

    if (queueCount < queueCapacity)
    {
        EnqueueQueue();
        return false;
    }
    else 
    {
        fillingQueue = false;
        return true;
    }
}

bool IsFillingQueue(void)
{
    return fillingQueue;
}

int QueueSize(void)
{
    return queueCount;
}

void DrawQueue(void)
{
    if (!queue)
        return;

    for (int i = 0; i < queueCount; i++)
    {
        int idx = (queueHead + i) % queueCapacity;
        Color fillColor;
        if (i == 0)
            fillColor = MAROON;
        else if (i == queueCount - 1)
            fillColor = BLUE;
        else
            fillColor = queue[i].color;
        DrawRectangleRec(queue[idx].rect, fillColor);
        DrawRectangleLinesEx(queue[idx].rect, 1.0f, DARKGRAY);

        char text[16];
        sprintf(text, "%d", queue[idx].value);

        int fontSize = 20;
        Vector2 sz = MeasureTextEx(GetFontDefault(), text, fontSize, 1);
        float tx = queue[idx].rect.x + (queue[idx].rect.width - sz.x) / 2;
        float ty = queue[idx].rect.y + (queue[idx].rect.height - sz.y) / 2;
        DrawText(text, tx, ty, fontSize, BLACK);
    }
}
