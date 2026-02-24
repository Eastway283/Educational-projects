#ifndef STRUCTURES_H
#define STRUCTURES_H

#include "raylib.h"
#include <stdbool.h>

/* Инициализация стека и очереди параметрами рабочей области и емкостью */
void InitStack(float x_pos, float y_pos, float width, float height, int capacity);

void InitQueue(float x_pos, float y_pos, float width, float height, int capacity);

/* Методы для освобождения памяти */
void FreeStack(void);

void FreeQueue(void);

/* Методы для добавления / удаления элементов */
bool PushStack(void);

bool PopStack(void);

bool EnqueueQueue(void);

bool DequeueQueue(void);

/* Методы для пошаговой очистки */
void StartClearStack(void);

bool StepClearStack(void);

void StartClearQueue(void);

bool StepClearQueue(void);

/* Методы для пошагового заполнения */
void StartFillStack(void);

bool StepFillStack(void);

void StartFillQueue(void);

bool StepFillQueue(void);

/* Текущие размеры структур */
int StackSize(void);

int QueueSize(void);

/* Проверка, идет ли очистка */
bool IsClearingStack(void);

bool IsClearingQueue(void);

/* Проверка, идет ли заполение */
bool IsFillingStack(void);
bool IsFillingQueue(void);

/* Методы для отрисовки */
void DrawStack(void);

void DrawQueue(void);


#endif
