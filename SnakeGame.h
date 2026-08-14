#ifndef SNAKE_GAME_H
#define SNAKE_GAME_H

#include <U8g2lib.h>

extern int SPEED;
extern int score;
extern int currentDelay;

void SnakeGame_Init();
void SnakeGame_Reset();
void SnakeGame_SpeedScreen(U8G2 &u8g2, int &gameState);
void SnakeGame_Play(U8G2 &u8g2, int &gameState);
void SnakeGame_ScoreScreen(U8G2 &u8g2, int &gameState);
void SnakeGame_HandleInput();

#endif

