// DinoGame.cpp
#include "DinoGame.h"
#include <Arduino_FreeRTOS.h>
#include "GameState.h"

#define DINO_WIDTH 25
#define DINO_HEIGHT 26
#define DINO_INIT_X 10
#define DINO_INIT_Y 37

#define BASE_LINE_Y 56
#define JUMP_PIXEL 22
#define BTN_JUMP 6
#define BTN_DUCK 4
// audio flag
static bool pinHighActive = false;
static int pinHighCounter = 0;
//
static U8G2* u8g2ptr = nullptr;

static int dinoY = DINO_INIT_Y;
static int jumpState = 0;
static float score = 0.0f;
static int treeX = 127;
static int treeX2 = 195;
static int treeType = 0;
static int treeType2 = 1;
static int jumpSpeed = 2;
static int obstacleSpeed = 2;

#define MAX_OBSTACLES 5

struct Obstacle {
  int x;
  int y;
  int type; // 0: cactus, 1: bird
  int variant; // 0 = tree1, 1 = tree2

};

Obstacle obstacles[5];
int obstacleCount = 0;
void trySpawnObstacle() {
  if (obstacleCount >= MAX_OBSTACLES) return;

  const int minGap = 40;  // 距離畫面右邊的最小間隔
  const int spawnChance = 15;

  // 找出最右邊的障礙物 x
  int rightmostX = -999;
  for (int i = 0; i < obstacleCount; ++i) {
    if (obstacles[i].x > rightmostX) {
      rightmostX = obstacles[i].x;
    }
  }

  if ((128 - rightmostX) < minGap || random(0, spawnChance) != 0) return;

  // 決定要出現 1 根或 2 根障礙物
  int numTrees = (random(0, 10) < 3) ? 2 : 1;  // 30% 機率出現兩根

  for (int t = 0; t < numTrees; ++t) {
    if (obstacleCount >= MAX_OBSTACLES) break;

    int variant = random(0, 2); // 0 = tree1, 1 = tree2
    int spacing = t * (30 + random(5, 10)); // 第二根往右偏移（保持落腳點）
    obstacles[obstacleCount].x = 128 + spacing;
    obstacles[obstacleCount].y = BASE_LINE_Y;
    obstacles[obstacleCount].type = 0;     // cactus 類
    obstacles[obstacleCount].variant = variant;
    obstacleCount++;
  }
}





void drawDino() {
  int x = DINO_INIT_X;
  int y = dinoY;

  if (digitalRead(BTN_DUCK) == LOW && dinoY == DINO_INIT_Y) {
    // 蹲下狀態
    u8g2ptr->drawBox(x, y + 4, 8, 4);
    u8g2ptr->drawBox(x + 6, y + 5, 2, 2);
  } else {
    // 一般站立狀態
    for (int i = 0; i <= 7; i++) u8g2ptr->drawPixel(x +10 + i, y );
    for (int i = 0; i <= 9; i++) u8g2ptr->drawPixel(x +9 + i, y+1 );
    for (int i = 0; i <= 9; i++) u8g2ptr->drawPixel(x + 9+i, y +2);
    for (int i = 0; i <= 9; i++) u8g2ptr->drawPixel(x + 9+i, y +3);
    for (int i = 0; i <= 9; i++) u8g2ptr->drawPixel(x + 9+i, y +4);//上顎
    for (int i = 0; i <= 3; i++) u8g2ptr->drawPixel(x + 9+i, y + 5);
    for (int i = 0; i <= 6; i++) u8g2ptr->drawPixel(x + 9+i, y + 6);//下顎
    for (int i = 0; i <= 4; i++) u8g2ptr->drawPixel(x + 8+i, y + 7);
    for (int i = 0; i <= 5; i++) u8g2ptr->drawPixel(x + 7+i, y + 8);
    for (int i = 0; i <= 9; i++) u8g2ptr->drawPixel(x + 6+i, y + 9);//手
    for (int i = 0; i <= 8; i++) u8g2ptr->drawPixel(x + 4+i, y + 10);
    u8g2ptr->drawPixel(x + 15, y + 10);
    for (int i = 0; i <= 13; i++) u8g2ptr->drawPixel(x -1+i, y + 11);
    for (int i = 0; i <= 12; i++) u8g2ptr->drawPixel(x +i, y + 12);
    for (int i = 0; i <= 10; i++) u8g2ptr->drawPixel(x +1 +i, y + 13);
    for (int i = 0; i <= 9; i++) u8g2ptr->drawPixel(x +2 +i, y + 14);
    for (int i = 0; i <= 7; i++) u8g2ptr->drawPixel(x +3 +i, y + 15);
    u8g2ptr->drawPixel(x + 3, y + 16);
    u8g2ptr->drawPixel(x + 8, y + 16);
    for (int i = 0; i <= 3; i++)u8g2ptr->drawPixel(x +4, y + 16+i);
    for (int i = 0; i <= 3; i++)u8g2ptr->drawPixel(x +9, y + 16+i);


    /////
    
  }
}


void drawTree(int x, int y) {
  u8g2ptr->drawPixel(x +1, y -10);
  u8g2ptr->drawPixel(x +2, y -10);
  u8g2ptr->drawBox(x, y - 9, 4, 10); // 從底部往上畫
  u8g2ptr->drawPixel(x +4, y -4);
  u8g2ptr->drawPixel(x +4, y -5);///長出來的節點
  u8g2ptr->drawPixel(x +5, y -5);///
  u8g2ptr->drawBox(x +5, y -9,2,4);
  ////左邊分支
  u8g2ptr->drawPixel(x -1, y -3);
  u8g2ptr->drawPixel(x -1, y -4);///長出來的節點
  u8g2ptr->drawPixel(x -2, y -4);
  u8g2ptr->drawBox(x -3, y -8,1,4);//
  u8g2ptr->drawBox(x -4, y -8,1,4);
}
void drawTree2(int x, int y) {
  u8g2ptr->drawPixel(x +1, y -10);
  u8g2ptr->drawPixel(x +2, y -10);
  u8g2ptr->drawBox(x, y - 10, 4, 11); // 從底部往上畫
  u8g2ptr->drawPixel(x +4, y -3);
  u8g2ptr->drawPixel(x +4, y -4);///長出來的節點
  u8g2ptr->drawPixel(x +5, y -4);///
  u8g2ptr->drawBox(x +5, y -9,2,5);
  ////左邊分支
}


void drawScore() {
  u8g2ptr->setFont(u8g2_font_5x7_tr);
  u8g2ptr->setCursor(70, 10);
  u8g2ptr->print("Score: ");
  u8g2ptr->print(score);
}

void showGameOver(int &gameState) {
  while (true) {
    u8g2ptr->firstPage();
    do {
      u8g2ptr->setFont(u8g2_font_ncenB08_tr);
      u8g2ptr->drawStr(30, 20, "Game Over");
      u8g2ptr->setCursor(35, 40);
      u8g2ptr->print("Score : ");
      u8g2ptr->print(score,1);
      u8g2ptr->setFont(u8g2_font_5x7_tr);
    } while (u8g2ptr->nextPage());
    
    if (digitalRead(BTN_DUCK) == LOW) {
      while (digitalRead(BTN_DUCK) == LOW);  // 等待按鍵放開
      delay(100); // 消抖，防止彈跳
      gameState = STATE_GAME_MENU;
      DinoGame_Init();    
      return;
    }


    vTaskDelay(10);
  }
}



void DinoGame_Init() {
  pinMode(BTN_JUMP, INPUT_PULLUP);
  pinMode(BTN_DUCK, INPUT_PULLUP);
  pinMode(31, OUTPUT);
  digitalWrite(31, LOW); // 預設關閉

  dinoY = DINO_INIT_Y;
  score = 0;
  jumpState = 0;
  obstacleCount = 0;

}

void DinoGame_Loop(int &gameState) {
  // 處理跳躍
  if (digitalRead(BTN_JUMP) == LOW && jumpState == 0) {
    jumpState = 1;
    pinHighActive = true;
    pinHighCounter = 3;  // 高電位維持 10 frame
  }

  if (jumpState == 1) {
    dinoY -= jumpSpeed*2;
    if (dinoY <= (DINO_INIT_Y - JUMP_PIXEL)) {
      jumpState = 2;

    }
  } else if (jumpState == 2) {
    dinoY += jumpSpeed;
    if (dinoY >= DINO_INIT_Y) {
      dinoY = DINO_INIT_Y;
      jumpState = 0;
    }
  }

  static int frameCount = 0;
  frameCount++;
  if (frameCount % 500 == 0 && obstacleSpeed < 6) {
    obstacleSpeed++;
  }
  if (pinHighActive) {
  digitalWrite(31, HIGH);
  pinHighCounter--;
  if (pinHighCounter <= 0) {
    pinHighActive = false;
    digitalWrite(31, LOW);
  }
  }

  // 每幀嘗試產生一個新障礙物
  trySpawnObstacle();

  // 移動障礙物 + 清除出畫面者
  for (int i = 0; i < obstacleCount; ++i) {
    obstacles[i].x -= obstacleSpeed;

    if (obstacles[i].x < -30) {
      // ✅ 加分公式：跳過的障礙物 × 速度 × 0.2
      score += (obstacleSpeed * 0.2f);  // 可改成 float score 取得更精細

      // 刪除障礙物（shift queue）
      for (int j = i; j < obstacleCount - 1; ++j) {
        obstacles[j] = obstacles[j + 1];
      }
      obstacleCount--;
      i--;
    }

  }
// 撞擊判斷
  for (int i = 0; i < obstacleCount; ++i) {
    int dinoHitboxX = DINO_INIT_X + 6;
    int dinoHitboxY = dinoY + 4;
    int dinoHitboxW = 13;
    int dinoHitboxH = 18;

    int obsHitboxX = obstacles[i].x;
    int obsHitboxY = (obstacles[i].type == 1) ? (obstacles[i].y - 3) : (obstacles[i].y - 9);
    int obsHitboxW = 3;
    int obsHitboxH = (obstacles[i].type == 1) ? 3 : 9;

    bool xOverlap = (dinoHitboxX < obsHitboxX + obsHitboxW) && (dinoHitboxX + dinoHitboxW > obsHitboxX);
    bool yOverlap = (dinoHitboxY < obsHitboxY + obsHitboxH) && (dinoHitboxY + dinoHitboxH > obsHitboxY);

    if (xOverlap && yOverlap) {
      showGameOver(gameState);
      return;
    }
  }



  // 繪圖階段
  u8g2ptr->firstPage();
  do {
    drawScore();

  for (int i = 0; i < obstacleCount; ++i) {
    if (obstacles[i].variant == 0)
      drawTree(obstacles[i].x, obstacles[i].y);
    else
      drawTree2(obstacles[i].x, obstacles[i].y);
  }

    drawDino();
    u8g2ptr->drawHLine(0, BASE_LINE_Y, 128);
  } while (u8g2ptr->nextPage());

  vTaskDelay(1);
}



void DinoGame_SetDisplay(U8G2 &display) {
  u8g2ptr = &display;
}

void DinoGame_RenderScene(int16_t i) {
  u8g2ptr->firstPage();
  do {
    drawDino();
    drawTree(50, 0);
    drawTree(100, 1);
    u8g2ptr->drawHLine(0, BASE_LINE_Y, 128);
    u8g2ptr->drawPixel(i, 60);
  } while (u8g2ptr->nextPage());
}