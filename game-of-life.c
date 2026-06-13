#include "raylib.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

/* 
 * Game of life
 * Mouse: left button - add cell 
 *        right button - remove cell 
 * Space - pause game 
 * Left  - decrease time
 * Right - increase time 
 */

#define TITLE "Game of life"

/* Размер клетки */
#define GRID_W 50
#define GRID_H 50

/* Размер окна   */
#define WIN_W 900
#define WIN_H 600

#define UTIME 0.5f
#define INCREASE 0.1f
#define UMAX 5.0f 
#define UMIN 0.1f

#define GRIDCOUNT ((WIN_W / GRID_W) * (WIN_H / GRID_H))

static uint8_t currgen[GRIDCOUNT];
static uint8_t nextgen[GRIDCOUNT];
static Vector2 cellpos[GRIDCOUNT];

void _DrawGrid(Color color) {
    for (int i = GRID_W; i < WIN_W; i += GRID_W)
        DrawLine(i, 0, i, WIN_H, color);
    for (int i = GRID_H; i < WIN_H; i += GRID_H)
        DrawLine(0, i, WIN_W, i, color);
}

void CalculatePos(void) {

    int cols, col, row, cx, cy;
    cols = WIN_W / GRID_W;
    for (int i = 0; i < GRIDCOUNT; i++) {
        row = i / cols;
        col = i % cols;
        cx = col * GRID_W + GRID_W / 2;
        cy = row * GRID_H + GRID_H / 2;
        cellpos[i].x = cx;
        cellpos[i].y = cy;
    }
}



void DrawCell(void) {

    float rad = (float)GRID_W / 2 - 10;
    for (int i = 0; i < GRIDCOUNT; i++)
        if (currgen[i])
            DrawCircleV(cellpos[i], rad, RED);
}

int CountNeighbors(int idx)
{
    int cols = WIN_W / GRID_W;
    int rows = WIN_H / GRID_H;
    int r = idx / cols;
    int c = idx % cols;
    int count = 0;
    for (int dr = -1; dr <= 1; dr++)
        for (int dc = -1; dc <= 1; dc++) {
            if (dr == 0 && dc == 0)
                continue;
            int nr = (r + dr + rows) % rows;
            int nc = (c + dc + cols) % cols;
            count += currgen[nr * cols + nc];
        }
    return count;
}

void UpdateGeneration(void) {
    memset(nextgen, 0, GRIDCOUNT);
    int n;
    for (int i = 0; i < GRIDCOUNT; i++) {
        n = CountNeighbors(i);
        if (currgen[i])
            nextgen[i] = (n == 2 || n == 3) ? 1 : 0;
        else
            nextgen[i] = (n == 3) ? 1 : 0;
    }
    memcpy(currgen, nextgen, GRIDCOUNT);
}

int main(void) {

    InitWindow(WIN_W, WIN_H, TITLE);
    memset(currgen, 0, GRIDCOUNT);
    memset(nextgen, 0, GRIDCOUNT);

    bool paused = true;
    int col, row, idx;

    float timer = 0.0f;
    float updateInterval = UTIME;

    int cols = WIN_W / GRID_W;

    CalculatePos();

    SetTargetFPS(30);

    while (!WindowShouldClose()) {

        if (IsKeyPressed(KEY_SPACE))
            paused = !paused;

        Vector2 mouse = GetMousePosition();

        col = mouse.x / GRID_W;
        row = mouse.y / GRID_H;

        idx = row * cols + col;

        if (paused) {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                currgen[idx] = 1;
            else if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
                currgen[idx] = 0;
        }

        if (IsKeyPressed(KEY_RIGHT))
            updateInterval += INCREASE;
        else if (IsKeyPressed(KEY_LEFT))
            updateInterval -= INCREASE;

        if (updateInterval < UMIN)
            updateInterval = UMIN;
        else if (updateInterval > UMAX)
            updateInterval = UMAX;

        if (!paused) {
            timer += GetFrameTime();
            if (timer >= updateInterval) {
                timer = 0;
                UpdateGeneration();
            }
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);
        if (paused)
            _DrawGrid(BLACK);
        else
            _DrawGrid(LIGHTGRAY);
        DrawCell();
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
