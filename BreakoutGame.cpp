#include "BreakoutGame.h"
#include <Arduino.h>

#define JOYSTICK_X A1
#define UP 6
#define MENU_KEY 4  // 按下此鍵可強制返回選單
#define RIGHT 4
#define LEFT 5
#define DOWN 7

const int screenW = 128;
const int screenH = 64;

// 磚塊設定
const int brickW = 2;
const int brickH = 2;
const int bricksPerRow = screenW / brickW;
const int brickRows = 5;
bool bricks[brickRows][bricksPerRow];

// 球
int ballX = 64, ballY = 50;
int ballDX = 1, ballDY = -1;
const int ballSize = 2;

// 平台
int paddleX = 59;
const int paddleY = 60;
const int paddleW = 15;
const int paddleH = 2;

// 狀態
bool waitingToLaunch = true;
int life = 7;
bool breakoutInitialized = false;
int breakoutSpeed = 2;  // 1 (slow), 2 (normal), 3 (fast), 4 (very fast)
bool breakoutSpeedSelected = false;
void drawHeart(U8G2 &u8g2, int x, int y) {
  u8g2.drawPixel(x + 1, y);
  u8g2.drawPixel(x + 3, y);
  for (int i = 0; i <= 4; i++) {
    u8g2.drawPixel(x + i, y + 1);
  }

  u8g2.drawPixel(x + 1, y + 2);
  u8g2.drawPixel(x + 2, y + 2);
  u8g2.drawPixel(x + 3, y + 2);

  u8g2.drawPixel(x + 2, y + 3);
}


void BreakoutGame_Init() {
  // 初始化磚塊
  for (int row = 0; row < brickRows; row++) {
    for (int col = 0; col < bricksPerRow; col++) {
      bricks[row][col] = true;
    }
  }
  ballX = 64;
  ballY = 50;
  ballDX = 1;
  ballDY = -1;
  paddleX = 59;
  waitingToLaunch = true;
  life = 5;
  breakoutInitialized = true;
  breakoutSpeedSelected = false;
  breakoutSpeed = 2;
}

void BreakoutGame_DrawSpeedScreen(U8G2 &u8g2, int &gameState) {
  char speedText[] = {char(breakoutSpeed + '0'), '\0'};
  u8g2.firstPage();
  do {
    u8g2.drawStr(15, 20, "Select Speed");
    u8g2.drawStr(62, 40, speedText);
    u8g2.drawStr(22, 60, "L to Start");
  } while (u8g2.nextPage());

  if (digitalRead(UP) == LOW && breakoutSpeed < 4) {
    breakoutSpeed++;
    while (digitalRead(UP) == LOW);
    delay(50);
  }
  if (digitalRead(DOWN) == LOW && breakoutSpeed > 1) {
    breakoutSpeed--;
    while (digitalRead(DOWN) == LOW);
    delay(50);
  }
  if (digitalRead(LEFT) == LOW) {
    while (digitalRead(LEFT) == LOW);
    delay(50);
    breakoutSpeedSelected = true;
  }
}

void BreakoutGame_Loop(U8G2 &u8g2, int &gameState) {
  if (!breakoutInitialized) {
    BreakoutGame_Init();
  }

  if (!breakoutSpeedSelected) {
    BreakoutGame_DrawSpeedScreen(u8g2, gameState);
    return;
  }

  // ✅ 強制回主選單
  if (digitalRead(MENU_KEY) == LOW) {
    while (digitalRead(MENU_KEY) == LOW);
    u8g2.setFont(u8g2_font_ncenB10_tr);  // 還原主畫面字體
    gameState = 1;
    breakoutInitialized = false;
    return;
  }

  // 🎮 撩桿控制平臺左右
  int xVal = analogRead(JOYSTICK_X);
  if (xVal < 400 && paddleX > 0) {
    paddleX += 2;
  } else if (xVal > 600 && paddleX + paddleW < screenW) {
    paddleX -= 2;
  }

  // 🏑 等待發球狀態
  if (waitingToLaunch) {
    ballX = paddleX + paddleW / 2 - ballSize / 2;
    ballY = paddleY - ballSize - 1;
    if (digitalRead(UP) == LOW) {
      while (digitalRead(UP) == LOW);
      waitingToLaunch = false;
    }
  } else {
    // 🎾 移動球
    ballX += ballDX;
    ballY += ballDY;

    // 撞牆
    if (ballX <= 0 || ballX + ballSize >= screenW) ballDX *= -1;
    if (ballY <= 0) ballDY *= -1;

    // 掉落
    if (ballY + ballSize >= screenH) {
      life--;
      if (life <= 0) {
        u8g2.setFont(u8g2_font_ncenB10_tr);  // 還原字體
        gameState = 1;
        breakoutInitialized = false;
        return;
      }
      waitingToLaunch = true;
      return;
    }

    // 撞平臺
    if (ballY + ballSize >= paddleY && ballY <= paddleY + paddleH &&
        ballX + ballSize >= paddleX && ballX <= paddleX + paddleW) {
      ballDY *= -1;
      ballY = paddleY - ballSize - 1;
    }

    // 撞磚塊
    for (int row = 0; row < brickRows; row++) {
      for (int col = 0; col < bricksPerRow; col++) {
        if (bricks[row][col]) {
          int bx = col * brickW;
          int by = row * brickH;
          if (ballX + ballSize >= bx && ballX <= bx + brickW &&
              ballY + ballSize >= by && ballY <= by + brickH) {
            bricks[row][col] = false;
            if (row != brickRows - 1) {
              // 最下面一排不反彈
              ballDY *= -1;
            }
          }
        }
      }
    }
  }

  // 🖼️ 畫面繪製
  u8g2.firstPage();
  do {
    // 磚塊
    for (int row = 0; row < brickRows; row++) {
      for (int col = 0; col < bricksPerRow; col++) {
        if (bricks[row][col]) {
          u8g2.drawBox(col * brickW, row * brickH, brickW, brickH);
        }
      }
    }

    // 球
    u8g2.drawBox(ballX, ballY, ballSize, ballSize);

    // 平臺
    u8g2.drawBox(paddleX, paddleY, paddleW, paddleH);

    // 生命顯示
    char lifeStr[10];
    sprintf(lifeStr, "%d", life);
    u8g2.setFont(u8g2_font_5x7_tr);
    u8g2.drawStr(12, 60, lifeStr);
    drawHeart(u8g2, 5, 55);  // 每顆心間距 6px
  } while (u8g2.nextPage());

  // 還原字體（以免影響主畫面）
  u8g2.setFont(u8g2_font_ncenB10_tr);

  // 根據選擇速度調整遊戲節奏（調整為明顯分段）
  switch (breakoutSpeed) {
    case 1: delay(150); break;  // 慢
    case 2: delay(100); break;  // 中
    case 3: delay(60); break;   // 快
    case 4: delay(10); break;   // 很快
  }
}