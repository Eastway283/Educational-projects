#include "raylib.h"
#include "button.h"
#include "array.h"
#include "algorithm.h"
#include "structures.h"
#include <stdbool.h>
#include <stdio.h>

/* =========================================================================================================================================
   Данная функция является входной точкой в программу по визуализации алгоритмов и структур
   Написанной в качестве курсовой работы по учебной дисциплине "Основы алгоритмизации и программирования"
   студентом первого курса ИИТ БГУИР. При разработке проекта использовалась графическая библиотке raylib,
   а также информация из открытых источников и проектов с открытым исходным кодом.
   Отдельную благодарность хотелось бы выразить raylib/examples, куда я очень часто обращался за примерами работы с графической библиотекой
   Это мой первый проект такого масштаба, спустя 4 месяца после изучения программирования.
   Да, проект содержит изъяны, а также не имеет достаточного тестирования. Также хотел бы сразу извиниться за "магические" константы.
   На данный момент у меня не хватает опыта и знаний, для решения этого вопроса. Возможно, что в будущем я все же вернусь к этому проекту.
   Дата, когда этот проект впервые был представлен в вебе: 2026-02-24
   ========================================================================================================================================= */

/* Параметры окна */
#define WIDTH 900
#define HEIGHT 600

/* Параметры рабочей области */
#define VIEW_X 240
#define VIEW_Y 30
#define VIEW_WIDTH 630
#define VIEW_HEIGHT 430

/* Символические константы массива */
#define MINSIZE 4 
#define MAXSIZE 50
/* Емкость структуры */
#define CAPACITY 10

#define FNAME "array.txt"

/* Режим программы */
typedef enum {
    MODE_ARRAY = 0,
    MODE_STRUCT
} UImode;

int main(void)
{

    /* Создание окна */
    const int screenWidth = WIDTH;
    const int screenHeight = HEIGHT;
    InitWindow(screenWidth, screenHeight, "VisAlgo - Visualization of algorithms and structures");

    /* Инициализация кнопок */
    initButtons();

    /* Инициализация массива */
    int rectCount = GetRandomValue(MINSIZE, MAXSIZE);
    int stepCount = 0;
    InitArray(rectCount, VIEW_X, VIEW_Y, VIEW_WIDTH, VIEW_HEIGHT);

    /* Переменные для авто-режима */
    static bool autoModArray = false;
    static double lastStepTimeArray = 0.0;
    static double lastStepTimeStruct = 0.0;
    static double stepDelay = 0.1;

    /* Переменная текущего режима */
    static UImode currentMode = MODE_ARRAY;

    /* Переменные структур и инициализация стека и очереди */
    int ElemCount = 0;
    static bool autoModStruct = false;
    InitStack(VIEW_X, VIEW_Y + VIEW_HEIGHT / 2 - VIEW_Y, VIEW_WIDTH, 110, CAPACITY);
    InitQueue(VIEW_X, VIEW_Y + VIEW_HEIGHT / 2 - VIEW_Y, VIEW_WIDTH, 110, CAPACITY);

    /* Флаг нажатия кнопки */
    bool exitWindow = false;

    SetTargetFPS(60);

    while(!exitWindow && !WindowShouldClose())
    {

        /* Получаем текущее состояние мыши */
        Vector2 mousePoint = GetMousePosition();
        bool mousePressed = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

        UpdateButton(mousePoint, mousePressed, currentMode);

        /* Сортировка */
        if (IsButtonPressed(BTN_BUBBLE))
        {
            currentMode = MODE_ARRAY;
            StartAlgorithm(ALGO_BUBBLE);
            stepCount = 0;
            autoModArray = false;
        }
        if (IsButtonPressed(BTN_SHAKE))
        {
            currentMode = MODE_ARRAY;
            StartAlgorithm(ALGO_SHAKE);
            stepCount = 0;
            autoModArray = false;
        }
        if (IsButtonPressed(BTN_INSERT))
        {
            currentMode = MODE_ARRAY;
            StartAlgorithm(ALGO_INSERT);
            stepCount = 0;
            autoModArray = false;
        }
        if (IsButtonPressed(BTN_SHELL))
        {
            currentMode = MODE_ARRAY;
            StartAlgorithm(ALGO_SHELL);
            stepCount = 0;
            autoModArray = false;
        }

        /* Поиск */
        if (IsButtonPressed(BTN_LINEAR))
        {
            currentMode = MODE_ARRAY;
            StartAlgorithm(ALGO_LINEAR);
            stepCount = 0;
            autoModArray = false;
        }
        if (IsButtonPressed(BTN_BINARY))
        {
            currentMode = MODE_ARRAY;
            StartAlgorithm(ALGO_BINARY);
            stepCount = 0;
            autoModArray = false;
        }

        /* Структуры */
        if (IsButtonPressed(BTN_STACK))
        {
            currentMode = MODE_STRUCT;
            StopAlgorithm();
            stepCount = 0;
            ElemCount = 0;
            autoModArray = false;
            autoModStruct = false;
        }
        if (IsButtonPressed(BTN_QUEUE))
        {
            currentMode = MODE_STRUCT;
            StopAlgorithm();
            stepCount = 0;
            ElemCount = 0;
            autoModArray = false;
            autoModStruct = false;
        }

        /* Save/Load/Quit */
        if (IsButtonPressed(BTN_SAVE))
        {
            if (SaveArray(FNAME))
            {
                printf("Saving succefully\n");
                autoModArray = false;
                autoModStruct = false;
            }
        }
        if (IsButtonPressed(BTN_LOAD))
        {
            if (LoadArray(FNAME))
            {
                printf("Load succefully\n");
                rectCount = GetArraySize();
                StopAlgorithm();
                stepCount = 0;
                autoModArray = false;
                autoModStruct = false;
            } 
        }
        if (IsButtonPressed(BTN_EXIT))
        {
            printf("Good bye!\n");
            exitWindow = true;
        }

        /* Панель управления массива */
        if (currentMode == MODE_ARRAY)
        {
            if (IsButtonPressed(BTN_STEP))
            {
                bool finished = StepAlgorithm();
                if (!finished)
                    ++stepCount;
                autoModArray = false;
            }
            if (IsButtonPressed(BTN_STOP))
            {
                autoModArray = false;
            }
            if (IsButtonPressed(BTN_SHUFFLE))
            {
                ShuffleArray();
                StopAlgorithm();
                stepCount = 0;
                autoModArray = false;
            }
            if (IsButtonPressed(BTN_REDUCE))
            {
                --rectCount;
                if (rectCount < MINSIZE)
                    rectCount = MINSIZE;
                InitArray(rectCount, VIEW_X, VIEW_Y, VIEW_WIDTH, VIEW_HEIGHT);
                StopAlgorithm();
                stepCount = 0;
                autoModArray = false;
            }
            if (IsButtonPressed(BTN_ADD))
            {
                ++rectCount;
                if (rectCount > MAXSIZE)
                    rectCount = 50;
                InitArray(rectCount, VIEW_X, VIEW_Y, VIEW_WIDTH, VIEW_HEIGHT);
                StopAlgorithm();
                stepCount = 0;
                autoModArray = false;
            }
            if (IsButtonPressed(BTN_DECREASE))
            {
                stepDelay -= 0.05;
                if (stepDelay < 0.05)
                    stepDelay = 0.05;
            }
            if (IsButtonPressed(BTN_INCREASE))
            {
                stepDelay += 0.05;
                if (stepDelay > 0.5)
                    stepDelay = 0.5;
            }
            if (IsButtonPressed(BTN_RESET))
            {
                StopAlgorithm();
                stepCount = 0;
                autoModArray = false;
            }
            if (IsButtonPressed(BTN_AUTO))
            {
                autoModArray = !autoModArray;
                if (autoModArray)
                    lastStepTimeArray = GetTime();
            }

            if (autoModArray && isAlgorithmRunning())
            {
                double now = GetTime();
                if (now - lastStepTimeArray > stepDelay)
                {
                    bool finished = StepAlgorithm();
                    if (!finished)
                        ++stepCount;
                    else
                        autoModArray = false;

                    lastStepTimeArray = now;
                }
            }
        }

        /* Панель управления структурами */
        if (currentMode == MODE_STRUCT)
        {
            if (IsButtonPressed(BTN_PUSH))
            {
                if (IsButtonActive(BTN_STACK))
                {
                    PushStack();
                    ElemCount = StackSize();
                }
                else if (IsButtonActive(BTN_QUEUE))
                {
                    EnqueueQueue();
                    ElemCount = QueueSize();
                }
                autoModStruct = false;
            }
            if (IsButtonPressed(BTN_POP))
            {
                if (IsButtonActive(BTN_STACK))
                {
                    PopStack();
                    ElemCount = StackSize();
                }
                else if (IsButtonActive(BTN_QUEUE))
                {
                    DequeueQueue();
                    ElemCount = QueueSize();
                }
                autoModStruct = false;
            }
            if (IsButtonPressed(BTN_FILL))
            {
                if (IsButtonActive(BTN_STACK))
                {
                    StartFillStack();
                }
                else if (IsButtonActive(BTN_QUEUE))
                {
                    StartFillQueue();
                }
                lastStepTimeStruct = GetTime();
                autoModStruct = true;
            }
            if (IsButtonPressed(BTN_CLEAR))
            {
                if (IsButtonActive(BTN_STACK))
                {
                    StartClearStack();
                }
                else if (IsButtonActive(BTN_QUEUE))
                {
                    StartClearQueue();
                }
                lastStepTimeStruct = GetTime();
                autoModStruct = true;
            }

            if (autoModStruct)
            {
                double now = GetTime();
                if (now - lastStepTimeStruct > stepDelay)
                {
                    bool finished = false;
                    if (IsButtonActive(BTN_STACK))
                    {
                        if (IsFillingStack())
                            finished = StepFillStack();
                        else if (IsClearingStack())
                            finished = StepClearStack();
                    }
                    else if (IsButtonActive(BTN_QUEUE))
                    {
                        if (IsFillingQueue())
                            finished = StepFillQueue();
                        else if (IsClearingQueue())
                            finished = StepClearQueue();
                    }

                    if (finished)
                        autoModStruct = false;

                    if (IsButtonActive(BTN_STACK))
                        ElemCount = StackSize();
                    else if (IsButtonActive(BTN_QUEUE))
                        ElemCount = QueueSize();
                }
            }
        }

        BeginDrawing();

        ClearBackground(RAYWHITE);
        /* Рисование раскладки меню */
        DrawRectangleLines(10, 30, 200, screenHeight - 50, DARKGRAY);
        DrawText("Menu", 15, 10, 20, GRAY);
        DrawText("Sorting type", 25, 35, 20, GRAY);
        DrawText("Search type", 25, 215, 20, GRAY);
        DrawText("Stuctures", 25, 325, 20, GRAY);
        DrawText("to\\from file", 25, 440, 20, GRAY);
        DrawButtons(mousePoint, currentMode);

        /* Отрисовка границ рабочей области и панели управления */
        DrawRectangleLines(VIEW_X, VIEW_Y, VIEW_WIDTH, VIEW_HEIGHT, DARKGRAY);
        DrawRectangleLines(VIEW_X, VIEW_Y + VIEW_HEIGHT + 10, VIEW_WIDTH, 110, DARKGRAY);

        if (currentMode == MODE_ARRAY)
        {
            DrawText(TextFormat("current step: %d", stepCount), 405, 480, 20, GRAY);

            DrawText("current speed:", 405, 515, 20, GRAY);
            DrawText(TextFormat("%.2fs", stepDelay), 620, 515, 20, GRAY);

            DrawText("number of elements:", 405, 550, 20, GRAY);
            DrawText(TextFormat("%d", rectCount), 650, 550, 20, GRAY);
            DrawArray();

            if (isAlgorithmRunning())
            {
                AlgorithmType type = GetCurrentAlgorithm();
                if (type == ALGO_LINEAR || type == ALGO_BINARY)
                {
                    int target = GetSearchTarget();
                    DrawText(TextFormat("searching for: %d", target), 680, 480, 20, MAROON);
                }
            }
        }
        
        if (currentMode == MODE_STRUCT)
        {
            DrawText(TextFormat("number of elements: %d", ElemCount), 445, 515, 20, GRAY);
            if (IsButtonActive(BTN_STACK))
            {
                DrawStack();
            }
            else if (IsButtonActive(BTN_QUEUE))
            {
                DrawQueue();
            }
        }

        DrawText("Developed by Daniil Kukso, 2026", screenWidth - 175, screenHeight - 15, 10, LIGHTGRAY);

        EndDrawing();
    }

    /* Очистка массива и структур */
    FreeArray();
    FreeStack();
    FreeQueue();
    CloseWindow();

    return 0;
}
