#ifndef BUTTON_H
#define BUTTON_H

#include "raylib.h"
#include <stdbool.h>

/* Перечисление для удобного доступа к кнопкам в массиве */
typedef enum {
    BTN_BUBBLE = 0,
    BTN_SHAKE,
    BTN_INSERT,
    BTN_SHELL,
    BTN_LINEAR,
    BTN_BINARY,
    BTN_STACK,
    BTN_QUEUE,
    BTN_SAVE,
    BTN_LOAD,
    BTN_EXIT,
    BTN_STEP,
    BTN_AUTO,
    BTN_SHUFFLE,
    BTN_RESET,
    BTN_STOP,
    BTN_REDUCE,
    BTN_ADD,
    BTN_DECREASE,
    BTN_INCREASE,
    BTN_POP,
    BTN_PUSH,
    BTN_CLEAR,
    BTN_FILL,
    BUTTON_COUNT
} ButtonID;

/* Перечисление для групп кнопок */
typedef enum {
    GROUP_NONE = 0,
    GROUP_MENU,
    GROUP_ARRAY_CONTROL,
    GROUP_STRUCT_CONTROL,
    GROUP_ACTION
} ButtonGroup;

/* Функция инициализации кнопок */
void initButtons(void);

/* Функция обновления состояния кнопок */
void UpdateButton(Vector2 mousePoint, bool mousePressed, int mode);

/* Функция рисования кнопок, в зависимости от текущего режима */
void DrawButtons(Vector2 mousePoint, int mode);

/* Проверка кнопки на ее активность */
bool IsButtonActive(ButtonID id);

/* Проверка - нажата ли кнопка в текуший кадр */
bool IsButtonPressed(ButtonID id);

#endif
