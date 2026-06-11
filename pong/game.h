#ifndef GAME_H
#define GAME_H

#include "raylib.h"

#define WIN_W 900
#define WIN_H 600
#define PADDLE_W 5
#define PADDLE_H 100
#define PADDLE_SPEED 400
#define BALL_RADIUS 10
#define BALL_SPEED 300

typedef struct {
    Rectangle rect;
    int score;
} Paddle;

typedef struct {
    Vector2 center;
    float radius;
    Vector2 vel;
} Ball;

typedef struct {
    Paddle left, right;
    Ball ball;
    bool paused;
} GameState;

void InitGame(GameState *gs);

void UpdateGame(GameState *gs, float dt);

void DrawGame(const GameState *gs);

#endif // GAME_H
