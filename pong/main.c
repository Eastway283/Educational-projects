#include "raylib.h"
#include <stdlib.h>
#include <stdio.h>
#include "game.h"

int main(void) {

    InitWindow(WIN_W,   WIN_H, "Pong");
    GameState gs;

    InitGame(&gs);

    SetTargetFPS(60);

    while (!WindowShouldClose()) {

        float dt = GetFrameTime();
        UpdateGame(&gs, dt);
        BeginDrawing();
        DrawGame(&gs);
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
