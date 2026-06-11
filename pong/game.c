#include "raylib.h"
#include <stdbool.h>
#include "game.h"


static float addspd = 10.0f;
static int rebounds = 0;

void InitGame(GameState *gs) {
    if (!gs)
        return;
    /* Ball initializating */
    gs->ball.center.x = WIN_W / 2.0f;
    gs->ball.center.y = WIN_H / 2.0f;
    gs->ball.radius = BALL_RADIUS;
    gs->ball.vel.x = 300.0f;
    gs->ball.vel.y = 150.0f;
    /* Paddles */
    gs->left.rect.x = WIN_W - WIN_W + 50;
    gs->left.rect.y = WIN_H / 2.0f - PADDLE_H / 2.0f;
    gs->left.rect.height = PADDLE_H;
    gs->left.rect.width = PADDLE_W;
    gs->left.score = 0;

    gs->right.rect.x = WIN_W - 50;
    gs->right.rect.y = WIN_H / 2.0f - PADDLE_H / 2.0f;
    gs->right.rect.height = PADDLE_H;
    gs->right.rect.width = PADDLE_W;
    gs->right.score = 0;
    
    gs->paused = true;
}

static void BallReset(GameState *gs) {
    gs->paused = true;
    gs->ball.center.x = WIN_W / 2.0f;
    gs->ball.center.y = WIN_H / 2.0f;
    gs->ball.vel.x = GetRandomValue(100, 255);
    gs->ball.vel.y = GetRandomValue(100, 255);
    rebounds = 0;
}

void UpdateGame(GameState *gs, float dt) {

    if (IsKeyPressed(KEY_SPACE)) {
        gs->paused = !gs->paused;
    }

    if (gs->paused)
        return;

    // Левый игрок
    if (IsKeyDown(KEY_W) && gs->left.rect.y > 0)
        gs->left.rect.y -= (PADDLE_SPEED * dt);
    else if (IsKeyDown(KEY_S) && gs->left.rect.y + PADDLE_H < WIN_H)
        gs->left.rect.y += (PADDLE_SPEED * dt);

    // Правый игрок
    if (IsKeyDown(KEY_UP) && gs->right.rect.y > 0)
        gs->right.rect.y -= (PADDLE_SPEED * dt);
    else if (IsKeyDown(KEY_DOWN) && gs->right.rect.y + PADDLE_H < WIN_H)
        gs->right.rect.y += (PADDLE_SPEED * dt);

    // Логика мяча
    gs->ball.center.x += (gs->ball.vel.x * dt);
    gs->ball.center.y += (gs->ball.vel.y * dt);

    // Логика отскока мяча от верхних границ
    if ((gs->ball.center.y - gs->ball.radius <= 0) || (gs->ball.center.y + gs->ball.radius >= WIN_H)) {
        gs->ball.vel.y = -gs->ball.vel.y;

    }

    // Логика откока мяча от боковых границ
    if ((gs->ball.center.x - gs->ball.radius <= 0)) { // Очко правому игроку
        BallReset(gs);
        ++gs->right.score;
    } else if ((gs->ball.center.x + gs->ball.radius >= WIN_W)) { // Очко левому игроку
        BallReset(gs);
        ++gs->left.score;
    }

    // Логика коллизий с ракетками
    if (CheckCollisionCircleRec(gs->ball.center, gs->ball.radius, gs->left.rect) || 
        CheckCollisionCircleRec(gs->ball.center, gs->ball.radius, gs->right.rect)) {
        ++rebounds;
        if (rebounds % 5 == 0)
            addspd *= 2;
        gs->ball.vel.x = -gs->ball.vel.x;

    float cur_speed = sqrt(gs->ball.vel.x * gs->ball.vel.x + 
                            gs->ball.vel.y * gs->ball.vel.y);
    float new_speed = cur_speed + addspd;

    if (cur_speed > 0.0f) {
        float scale = new_speed / cur_speed;
        gs->ball.vel.x *= scale;
        gs->ball.vel.y *= scale;
    }
    }
}

void DrawGame(const GameState *gs) {
    
    ClearBackground(RAYWHITE);

    DrawCircleV(gs->ball.center, gs->ball.radius, RED);
    DrawCircleLinesV(gs->ball.center, gs->ball.radius, BLACK);
    DrawRectangleRec(gs->left.rect, RED);
    DrawRectangleRec(gs->right.rect, RED);

    DrawText(TextFormat("%d", gs->left.score), 20, 20, 20, BLACK);
    DrawText(TextFormat("%d", gs->right.score), WIN_W - 20, 20, 20, BLACK);

    if (gs->paused)
        DrawText("Paused", WIN_W / 2 - MeasureText("Paused", 40) / 2, WIN_H / 2 - 100 / 2, 40, GRAY);
    else
    DrawLine(WIN_W / 2, 0, WIN_W / 2, WIN_H, LIGHTGRAY);
}
