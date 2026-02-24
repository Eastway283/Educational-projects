#include "button.h"
#include <raylib.h>
#include <stdbool.h>

/* ===================================================================================================================
   Модуль содержит реализацию логики и обработки кнопок GUI
   Все кнопки программы содержаться в статическом массиве, и отрисовываются и обновляются
   в зависимости от текущего режима. В заголовочном файле определены перечисления для более ясного и простого доступа
   к элементам в массиве, а также для более удобной группировки кнопок по их категориям
   Кнопка представляет собой структуру с прямоугольником, цветом и флагами состояния
   Кнопки меню отображаются всегда, и обрабатываются вне зависимости от режима
   В зависимости от режима контрольная панель отображает определенные функциональные элементы
   =================================================================================================================== */

typedef struct Button {
    Rectangle rect; /* Тело кнопки */
    const char* text; 
    bool isActive;     /* Проверка состояния */
    bool isPressed;    /* Проверка нажатия */
    Color normalColor;
    Color activeColor;
    ButtonGroup group;
    double flash;
} Button;

/* Массив кнопок, а также вспомогательная функция создания кнопок */
static Button buttons[BUTTON_COUNT];

static Button createButton(Rectangle rec, const char* txt, Color normalcol, Color activecol, ButtonGroup grp) 
{
    Button btn = {
        .rect = rec,
        .text = txt,
        .isActive = false,
        .isPressed = false,
        .normalColor = normalcol,
        .activeColor = activecol,
        .group = grp,
        .flash = 0.0
    };
    return btn;
}

void initButtons(void)
{
    /* ======================== Раскладка основного меню ======================== */
    /* Секция сортировки */
    buttons[BTN_BUBBLE] = createButton((Rectangle) {25, 60, 170, 25}, "bubble sort", RAYWHITE, LIGHTGRAY, GROUP_MENU);
    buttons[BTN_SHAKE] = createButton((Rectangle) {25, 95, 170, 25}, "shake sort", RAYWHITE, LIGHTGRAY, GROUP_MENU);
    buttons[BTN_INSERT] = createButton((Rectangle) {25, 130, 170, 25}, "insert sort", RAYWHITE, LIGHTGRAY,GROUP_MENU);
    buttons[BTN_SHELL] = createButton((Rectangle) {25, 165, 170, 25}, "shell sort", RAYWHITE, LIGHTGRAY, GROUP_MENU);
    /* Секция поиска */
    buttons[BTN_LINEAR] = createButton((Rectangle) {25, 240, 170, 25}, "linear search", RAYWHITE, LIGHTGRAY, GROUP_MENU);
    buttons[BTN_BINARY] = createButton((Rectangle) {25, 275, 170, 25}, "binary search", RAYWHITE, LIGHTGRAY, GROUP_MENU);
    /* Секция структур */
    buttons[BTN_STACK] = createButton((Rectangle) {25, 350, 170, 25}, "stack", RAYWHITE, LIGHTGRAY, GROUP_MENU);
    buttons[BTN_QUEUE] = createButton((Rectangle) {25, 385, 170, 25}, "queue", RAYWHITE, LIGHTGRAY, GROUP_MENU);
    /* Кнопки сохранения\загрузки */
    buttons[BTN_SAVE] = createButton((Rectangle) {25, 460, 80, 25}, "save", RAYWHITE, LIGHTGRAY, GROUP_ACTION);
    buttons[BTN_LOAD] = createButton((Rectangle) {115, 460, 80, 25}, "load", RAYWHITE, LIGHTGRAY, GROUP_ACTION);
    /* Выход */
    buttons[BTN_EXIT] = createButton((Rectangle) {25, 540, 80, 25}, "quit", RAYWHITE, LIGHTGRAY, GROUP_ACTION);

    /* ===================== Панель управления сортировкой и поиском =================== */
    buttons[BTN_STEP] = createButton((Rectangle) {250, 475, 145, 25}, "step", RAYWHITE, LIGHTGRAY, GROUP_ARRAY_CONTROL);
    buttons[BTN_AUTO] = createButton((Rectangle) {250, 510, 145, 25}, "auto", RAYWHITE, LIGHTGRAY, GROUP_ARRAY_CONTROL);
    buttons[BTN_STOP] = createButton((Rectangle) {250, 545, 145, 25}, "stop", RAYWHITE, LIGHTGRAY, GROUP_ARRAY_CONTROL);
    buttons[BTN_SHUFFLE] = createButton((Rectangle) {715, 510, 145, 25}, "shuffle", RAYWHITE, LIGHTGRAY, GROUP_ARRAY_CONTROL);
    buttons[BTN_RESET] = createButton((Rectangle) {715, 545, 145, 25}, "reset", RAYWHITE, LIGHTGRAY, GROUP_ARRAY_CONTROL);
    buttons[BTN_REDUCE] = createButton((Rectangle) {615, 545, 25, 25}, "<", RAYWHITE, LIGHTGRAY, GROUP_ARRAY_CONTROL);
    buttons[BTN_ADD] = createButton((Rectangle) {680, 545, 25, 25}, ">", RAYWHITE, LIGHTGRAY, GROUP_ARRAY_CONTROL);
    buttons[BTN_DECREASE] = createButton((Rectangle) {585, 510, 25, 25}, "<", RAYWHITE, LIGHTGRAY, GROUP_ARRAY_CONTROL);
    buttons[BTN_INCREASE] = createButton((Rectangle) {680, 510, 25, 25}, ">", RAYWHITE, LIGHTGRAY, GROUP_ARRAY_CONTROL);

    /* ===================== Панель управления структурами ======================== */
    buttons[BTN_PUSH] = createButton((Rectangle) {250, 475, 145, 40}, "add", RAYWHITE, LIGHTGRAY, GROUP_STRUCT_CONTROL);
    buttons[BTN_POP] = createButton((Rectangle) {715, 475, 145, 40}, "delete", RAYWHITE, LIGHTGRAY, GROUP_STRUCT_CONTROL);
    buttons[BTN_FILL] = createButton((Rectangle) {250, 525, 145, 40}, "fill", RAYWHITE, LIGHTGRAY, GROUP_STRUCT_CONTROL);
    buttons[BTN_CLEAR] = createButton((Rectangle) {715, 525, 145, 40}, "reset", RAYWHITE, LIGHTGRAY, GROUP_STRUCT_CONTROL);
}

/* Функция для обновления состояния кнопки в зависимости от состояния курсора и режима */
void UpdateButton(Vector2 mousePoint, bool mousePressed, int mode)
{
    /* Сбрасываем состояния всех кнопок */
    for (int i = 0; i < BUTTON_COUNT; i++)
        buttons[i].isPressed = false;

    if (!mousePressed)
        return;

    int clickedIndex = -1;
    /* Обрабатываем меню и текущую панель управения */
    for (int i = 0; i < BUTTON_COUNT; i++)
    {
        bool visible = false;
        ButtonGroup grp = buttons[i].group;
        if (grp == GROUP_MENU || grp == GROUP_ACTION)
            visible = true;
        else if (grp == GROUP_ARRAY_CONTROL && mode == 0)
            visible = true;
        else if (grp == GROUP_STRUCT_CONTROL && mode == 1)
            visible = true;

        if (!visible)
            continue;

        /* Если кнопка в текущий момент видима, то обрабатываем ее состояние и выходим из цикла */
        if (CheckCollisionPointRec(mousePoint, buttons[i].rect))
        {
            clickedIndex = i;
            break;
        }
    }

    if (clickedIndex == -1)
        return;

    Button* btn = &buttons[clickedIndex];

    /* Устанавливаем подсветку при нажатии */
    btn->flash = GetTime() + 0.2;
    btn->isPressed = true;

    switch (btn->group)
    {
        case GROUP_MENU:
            for (int i = 0; i < BUTTON_COUNT; i++)
            {
                if (buttons[i].group == btn->group)
                    buttons[i].isActive = false;
            }

            btn->isActive = true;
            break;

        case GROUP_ARRAY_CONTROL:
        case GROUP_STRUCT_CONTROL:
        case GROUP_ACTION:
            break;

        default:
            break;
    }
}

/* Функция для затемнения кнопки при нажатии, понижает яркость на amount */
static Color Darken(Color col, int amount)
{
    return (Color) {
        .r = (col.r > amount) ? col.r - amount : 0,
        .g = (col.g > amount) ? col.g - amount : 0,
        .b = (col.b > amount) ? col.b - amount : 0,
        .a = col.a
    };
}

void DrawButtons(Vector2 mousePoint, int mode)
{
    for (int i = 0; i < BUTTON_COUNT; i++)
    {
        Button* btn = &buttons[i];

        bool visible = false;
        if (btn->group == GROUP_MENU || btn->group == GROUP_ACTION)
            visible = true;
        else if (btn->group == GROUP_ARRAY_CONTROL && mode == 0)
            visible = true;
        else if (btn->group == GROUP_STRUCT_CONTROL && mode == 1)
            visible = true;

        if (!visible)
            continue;

        bool hover = CheckCollisionPointRec(mousePoint, btn->rect);

        /* Выбираем цвет кнопки */
        Color color;
        if (GetTime() < btn->flash)
            color = btn->activeColor;
        else if (btn->isActive)
            color = btn->activeColor;
        else if (hover)
            color = Darken(btn->normalColor, 30);
        else
            color = btn->normalColor;

        /* Рисуем тело кнопки */
        DrawRectangleRec(btn->rect, color);

        /* Обводка */
        DrawRectangleLinesEx(btn->rect, 1.0f, DARKGRAY);

        /* Позиция текста вычисляется в зависимости от размера кнопки */
        int fontSize = 20;
        Vector2 textSize = MeasureTextEx(GetFontDefault(), btn->text, fontSize, 1);
        int textX = btn->rect.x + (btn->rect.width - textSize.x) / 2;
        int textY = btn->rect.y + (btn->rect.height - textSize.y) / 2;
        DrawText(btn->text, textX, textY, fontSize, BLACK);
    }
}

bool IsButtonActive(ButtonID id)
{
    return buttons[id].isActive;
}

bool IsButtonPressed(ButtonID id)
{
    return buttons[id].isPressed;
}
