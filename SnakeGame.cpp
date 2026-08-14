#include "SnakeGame.h"
#include <Arduino.h>
#include <U8g2lib.h>
#include <Arduino_FreeRTOS.h>

#define RIGHT 4
#define LEFT 5
#define UP 6
#define DOWN 7

#define MAX_LENGTH 100

struct gameItem {
  unsigned int X;
  unsigned int Y;
};

static gameItem snake[MAX_LENGTH];
static gameItem snakeFood;

static unsigned int snakeSize = 4;
static unsigned int gameItemSize = 4;
static unsigned int snakeDir = 1;

int SPEED = 1;
int score = 0;
int currentDelay = 50;

void SnakeGame_Init() {
  snakeSize = 4;
  SPEED = 1;
  currentDelay = 200 - SPEED * 50;
  score = 0;
}

void SnakeGame_Reset() {
  unsigned int startX = 40, startY = 32;
  for (unsigned int i = 0; i < snakeSize; i++) {
    snake[i].X = startX - i * gameItemSize;
    snake[i].Y = startY;
  }
  snakeDir = 1;
}

static void spawnSnakeFood() {
  unsigned int i = 1;
  do {
    snakeFood.X = random(2, 126);
    while (snake[i].X == snakeFood.X || i != snakeSize) {
      snakeFood.X = random(2, 126);
      i++;
    }
  } while (snakeFood.X % 4 != 0);

  i = 1;
  do {
    snakeFood.Y = random(2, 62);
    while (snake[i].Y == snakeFood.Y || i != snakeSize) {
      snakeFood.Y = random(2, 62);
      i++;
    }
  } while (snakeFood.Y % 4 != 0);
}

static void drawSnake(U8G2 &u8g2) {
  for (unsigned int i = 0; i < snakeSize; i++)
    u8g2.drawFrame(snake[i].X, snake[i].Y, gameItemSize, gameItemSize);
}

static void drawFood(U8G2 &u8g2) {
  u8g2.drawBox(snakeFood.X, snakeFood.Y, gameItemSize, gameItemSize);
}

static void updateValues() {
  for (int i = snakeSize - 1; i > 0; i--)
    snake[i] = snake[i - 1];

  if (snakeDir == 0) snake[0].X -= gameItemSize;
  else if (snakeDir == 1) snake[0].X += gameItemSize;
  else if (snakeDir == 2) snake[0].Y += gameItemSize;
  else if (snakeDir == 3) snake[0].Y -= gameItemSize;
}

static void handleCollisions(int &gameState) {
  if (snake[0].X == snakeFood.X && snake[0].Y == snakeFood.Y) {
    snakeSize++;
    score++;
    spawnSnakeFood();
  } else {
    for (unsigned int i = 1; i < snakeSize; i++)
      if (snake[0].X == snake[i].X && snake[0].Y == snake[i].Y)
        gameState = 4; // STATE_SNAKE_SCORE
  }

  if (snake[0].X < 0 || snake[0].Y < 0 || snake[0].X > 124 || snake[0].Y > 60)
    gameState = 4;
}

void SnakeGame_Play(U8G2 &u8g2, int &gameState) {
  while (gameState == 3) { // STATE_SNAKE_GAME
    if (digitalRead(DOWN) == LOW) {
      while (digitalRead(DOWN) == LOW);
      delay(50);
      gameState = 1; // STATE_GAME_MENU
      return;
    }
    handleCollisions(gameState);
    updateValues();

    u8g2.firstPage();
    do {
      drawSnake(u8g2);
      drawFood(u8g2);
    } while (u8g2.nextPage());

    delay(currentDelay); // ? �ϥ� Arduino �з� delay()
  }
}


void SnakeGame_SpeedScreen(U8G2 &u8g2, int &gameState) {
  char speedText[] = {SPEED + '0', '\0'};
  u8g2.firstPage();
  do {
    u8g2.drawStr(15, 20, "Select Speed");
    u8g2.drawStr(62, 40, speedText);
    u8g2.drawStr(22, 60, "L to Start");
  } while (u8g2.nextPage());

  if (digitalRead(UP) == LOW && SPEED < 4) {
    SPEED++;
    while (digitalRead(UP) == LOW);
    delay(50);
  }

  if (digitalRead(DOWN) == LOW && SPEED > 1) {
    SPEED--;
    while (digitalRead(DOWN) == LOW);
    delay(50);
  }

  if (digitalRead(LEFT) == LOW) {
    while (digitalRead(LEFT) == LOW);
    delay(50);
    currentDelay = 200 - SPEED * 50;
    SnakeGame_Reset();
    spawnSnakeFood();
    score = 0;
    gameState = 3; // STATE_SNAKE_GAME
  }
}

void SnakeGame_ScoreScreen(U8G2 &u8g2, int &gameState) {
  char scoreStr[4];
  snprintf(scoreStr, sizeof(scoreStr), "%03d", score);
  u8g2.firstPage();
  do {
    u8g2.drawStr(12, 20, "-<< SCORE >>-");
    u8g2.drawStr(50, 40, scoreStr);
    u8g2.drawStr(12, 60, "R to menu");
  } while (u8g2.nextPage());

  if (digitalRead(RIGHT) == LOW) {
    while (digitalRead(RIGHT) == LOW);
    delay(100);
    SnakeGame_Init();
    gameState = 1; // STATE_GAME_MENU
  }
}

void SnakeGame_HandleInput() {
  int x = analogRead(A1);
  int y = analogRead(A0);
  if (x < 50 && snakeDir != 1) snakeDir = 1;
  else if (x > 700 && snakeDir != 0) snakeDir = 0;
  else if (y < 300 && snakeDir != 3) snakeDir = 2;
  else if (y > 700 && snakeDir != 2) snakeDir = 3;
}

