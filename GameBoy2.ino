#include <U8g2lib.h>
#include <Arduino_FreeRTOS.h>
#include <SPI.h>
#include "LittleRookChess.h"
#include "SnakeGame.h"
#include "BreakoutGame.h"
#include "DinoGame.h"
#include "snake_logo_pixels_new.h"
#include "controller_logo_pixels.h"
#include "gameboy_title_pixels.h"
#include "GameState.h"
#include "CityDriveGame.h"



#define MENU_DISPLAY_COUNT 3  // 每頁最多顯示幾個項目

#define RIGHT 4
#define LEFT 5
#define UP 6
#define DOWN 7
#define JOYSTICK_X A1
#define JOYSTICK_Y A0

U8G2_ST7920_128X64_F_SW_SPI u8g2(U8G2_R0, 13, 11, 10, 9);

// === Snake pixelart logo ===
GameState gameState = STATE_SPLASH;

#define GAME_COUNT 6
const char *gameMenuList[GAME_COUNT] = {
  "Snake", "Chess", "Breakout", "Dino", "Logo View", "City Drive"
};

int menuIndex = 0;
int topIndex = 0;
TaskHandle_t TaskHandle_1;
TaskHandle_t TaskHandle_2;
CityDriveGame cityDrive(u8g2);
struct Firework {
  int centerX;
  int centerY;
  int frame;
  bool active;
};

#define MAX_FIREWORKS 3
Firework fireworks[MAX_FIREWORKS];

void drawFireworks() {
  for (int i = 0; i < MAX_FIREWORKS; i++) {
    if (!fireworks[i].active && random(0, 50) == 0) {
      fireworks[i].centerX = random(20, 108);
      fireworks[i].centerY = random(10, 30);
      fireworks[i].frame = 0;
      fireworks[i].active = true;
    }

    if (fireworks[i].active) {
      int cx = fireworks[i].centerX;
      int cy = fireworks[i].centerY;
      int f = fireworks[i].frame;

      for (int j = 0; j < 12; j++) {
        float angle = j * 3.14159 * 2 / 12;
        int x = cx + cos(angle) * (f + 1);
        int y = cy + sin(angle) * (f + 1);
        u8g2.drawPixel(x, y);
        u8g2.drawPixel(x + 1, y);
        u8g2.drawPixel(x, y + 1);
      }

      fireworks[i].frame++;
      if (fireworks[i].frame > 10) {
        fireworks[i].active = false;
      }
    }
  }
}

void drawLogoAnimation() {
  static int upToStartOffset = 128;
  static int upToStartDir = -1;

  u8g2.firstPage();
  do {
    drawGameBoyTitle(u8g2);  // 當作背景

    // 畫白色粒子煙火（前景）
    for (int i = 0; i < MAX_FIREWORKS; i++) {
      if (!fireworks[i].active && random(0, 50) == 0) {
        fireworks[i].centerX = random(20, 108);
        fireworks[i].centerY = random(10, 50);
        fireworks[i].frame = 0;
        fireworks[i].active = true;
      }

      if (fireworks[i].active) {
        int cx = fireworks[i].centerX;
        int cy = fireworks[i].centerY;
        int f = fireworks[i].frame;

        u8g2.setDrawColor(1); // 白色煙火
        for (int j = 0; j < 12; j++) {
          float angle = j * 3.14159 * 2 / 12;
          int x = cx + cos(angle) * (f + 1);
          int y = cy + sin(angle) * (f + 1);
          u8g2.drawPixel(x, y);
          u8g2.drawPixel(x + 1, y);
          u8g2.drawPixel(x, y + 1);
        }

        fireworks[i].frame++;
        if (fireworks[i].frame > 10) {
          fireworks[i].active = false;
        }
      }
    }

    // 飄動的提示字（可選）
    u8g2.setDrawColor(1);
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(upToStartOffset, 62, "UP to Start");

  } while (u8g2.nextPage());

  upToStartOffset += upToStartDir;
  if (upToStartOffset <= 22) upToStartDir = 0;
  delay(80);
}
void updateMenuSelection(bool goingDown) {
  if (goingDown) {
    menuIndex = (menuIndex + 1) % GAME_COUNT;
    if (menuIndex >= topIndex + MENU_DISPLAY_COUNT)
      topIndex = menuIndex - MENU_DISPLAY_COUNT + 1;
    else if (menuIndex < topIndex)
      topIndex = 0;
  } else {
    menuIndex = (menuIndex - 1 + GAME_COUNT) % GAME_COUNT;
    if (menuIndex < topIndex)
      topIndex = menuIndex;
    else if (menuIndex >= topIndex + MENU_DISPLAY_COUNT)
      topIndex = GAME_COUNT - MENU_DISPLAY_COUNT;
  }
}
void TaskDisplayLCD(void *pvParameters) {
  static bool showedControllerLogo = false;
  (void) pvParameters;
  chess_Init(u8g2.getU8g2(), 0);
  static uint8_t last_chess_input = 0;

  for (;;) {
    switch (gameState) {
      case STATE_SPLASH:
        if (!showedControllerLogo) {
          drawControllerLogoImage(u8g2);
          vTaskDelay(5000 / portTICK_PERIOD_MS);
          showedControllerLogo = true;
        }

        drawLogoAnimation();
        if (digitalRead(UP) == LOW) {
          while (digitalRead(UP) == LOW);
          delay(100);
          gameState = STATE_GAME_MENU;
        }
        break;

      
        case STATE_GAME_MENU: {
          if (digitalRead(DOWN) == LOW) {
            updateMenuSelection(true);
            vTaskDelay(150 / portTICK_PERIOD_MS);
          } else if (digitalRead(UP) == LOW) {
            updateMenuSelection(false);
            vTaskDelay(150 / portTICK_PERIOD_MS);
          } else if (digitalRead(LEFT) == LOW) {
            switch (menuIndex) {
            case 0: gameState = STATE_SNAKE_SPEED; break;
            case 1: gameState = STATE_CHESS_GAME; break;
            case 2: gameState = STATE_BREAKOUT_GAME; break;
            case 3: gameState = STATE_DINO_GAME; break;
            case 4: gameState = STATE_LOGO_VIEW; break;
            case 5: 
              cityDrive.begin();
              gameState = STATE_CITY_DRIVE_GAME; 
              break;

          }

            vTaskDelay(150 / portTICK_PERIOD_MS);
          } else if (digitalRead(RIGHT) == LOW) {
            gameState = STATE_SPLASH;
            vTaskDelay(150 / portTICK_PERIOD_MS);
          }

          // 畫面更新
          u8g2.firstPage();
          do {
            u8g2.setFont(u8g2_font_ncenB10_tr);
            u8g2.drawStr(20, 18, "Select Game");

            for (int i = 0; i < MENU_DISPLAY_COUNT; i++) {
              int index = topIndex + i;
              if (index >= GAME_COUNT) break;

              if (index == menuIndex)
                u8g2.drawBox(10, 30 + i * 10 - 8, 100, 10);

              u8g2.setDrawColor(index == menuIndex ? 0 : 1);
              u8g2.drawStr(15, 30 + i * 10, gameMenuList[index]);
              u8g2.setDrawColor(1);
            }

            // 畫出簡單滑桿
            int scrollbarHeight = 30;
            int scrollbarTop = 22;
            int barHeight = scrollbarHeight * MENU_DISPLAY_COUNT / GAME_COUNT;
            int barPosition = scrollbarHeight * topIndex / GAME_COUNT;
            u8g2.drawFrame(112, scrollbarTop, 3, scrollbarHeight);
            u8g2.drawBox(112, scrollbarTop + barPosition, 3, barHeight);
          } while (u8g2.nextPage());
          break;
        }



      case STATE_SNAKE_SPEED:
        SnakeGame_SpeedScreen(u8g2, (int&)gameState);
        break;

      case STATE_SNAKE_GAME:
        SnakeGame_Play(u8g2, (int&)gameState);
        break;

      case STATE_SNAKE_SCORE:
        SnakeGame_ScoreScreen(u8g2, (int&)gameState);
        break;

      case STATE_BREAKOUT_GAME:
        BreakoutGame_Loop(u8g2, (int&)gameState);
        break;

      case STATE_DINO_GAME:
        DinoGame_Loop((int&)gameState);
        break;

      case STATE_LOGO_VIEW:
        drawSnakeLogoImage(u8g2);
        if (digitalRead(UP) == LOW || digitalRead(DOWN) == LOW || digitalRead(LEFT) == LOW || digitalRead(RIGHT) == LOW) {
          while (digitalRead(UP) == LOW || digitalRead(DOWN) == LOW || digitalRead(LEFT) == LOW || digitalRead(RIGHT) == LOW);
          gameState = STATE_GAME_MENU;
        }
        break;

      case STATE_CHESS_GAME: {
        uint8_t input = 0;
        if (digitalRead(DOWN) == LOW) input = U8X8_MSG_GPIO_MENU_NEXT;
        else if (digitalRead(UP) == LOW) input = U8X8_MSG_GPIO_MENU_PREV;
        else if (digitalRead(LEFT) == LOW) input = U8X8_MSG_GPIO_MENU_HOME;
        else if (digitalRead(RIGHT) == LOW) input = U8X8_MSG_GPIO_MENU_SELECT;

        if (input != last_chess_input) {
          if (input != 0) chess_Step(input);
          last_chess_input = input;
        }

        u8g2.firstPage();
        do {
          chess_Draw();
        } while (u8g2.nextPage());
        break;
      }
      case STATE_CITY_DRIVE_GAME:
        if (digitalRead(DOWN) == LOW) {
          while (digitalRead(DOWN) == LOW);
          delay(100);
          gameState = STATE_GAME_MENU;
          break;
        }
        if (digitalRead(LEFT) == LOW) cityDrive.moveLeft();
        if (digitalRead(RIGHT) == LOW) cityDrive.moveRight();
        cityDrive.update();
        cityDrive.draw();
        break;


            
      }
}
}

void TaskHandleButton(void *pvParameters) {
  (void) pvParameters;
  for (;;) {
    SnakeGame_HandleInput();
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(LEFT, INPUT_PULLUP);
  pinMode(RIGHT, INPUT_PULLUP);
  pinMode(DOWN, INPUT_PULLUP);
  pinMode(UP, INPUT_PULLUP);
  pinMode(JOYSTICK_X, INPUT);
  pinMode(JOYSTICK_Y, INPUT);

  u8g2.begin(UP, DOWN, LEFT, U8X8_PIN_NONE, U8X8_PIN_NONE, U8X8_PIN_NONE);
  u8g2.setFont(u8g2_font_ncenB10_tr);
  randomSeed(analogRead(0));

  SnakeGame_Init();
  DinoGame_SetDisplay(u8g2);
  DinoGame_Init();

  xTaskCreate(TaskDisplayLCD, "Task1", 200, NULL, 1, &TaskHandle_1);
  xTaskCreate(TaskHandleButton, "Task2", 100, NULL, 1, &TaskHandle_2);
}

void loop() {}




/*#include <U8g2lib.h>
#include <Arduino_FreeRTOS.h>
#include <SPI.h>
#include "LittleRookChess.h"
#include "SnakeGame.h"  // 匯入貪食蛇模組
#include "BreakoutGame.h"
#include "DinoGame.h"

// 控制腳位定義
#define RIGHT 4
#define LEFT 5
#define UP 6
#define DOWN 7
#define JOYSTICK_X A1
#define JOYSTICK_Y A0

// 畫面定義
U8G2_ST7920_128X64_F_SW_SPI u8g2(U8G2_R0, 13, 11, 10, 9);

// 狀態列舉
enum GameState {
  STATE_SPLASH,
  STATE_GAME_MENU,
  STATE_SNAKE_SPEED,
  STATE_SNAKE_GAME,
  STATE_SNAKE_SCORE,
  STATE_BREAKOUT_GAME,  
  STATE_DINO_GAME,
  STATE_CHESS_GAME
  
};

GameState gameState = STATE_SPLASH;
uint8_t menuSelection = 1;
const char *gameMenu = "Snake\nChess\nBreakout\nDino";


TaskHandle_t TaskHandle_1;
TaskHandle_t TaskHandle_2;

void drawLogoAnimation() {
  static int headX = 0;
  static int visibleCols = 0;
  static bool hiding = false;
  static bool movementStarted = false;
  static int upToStartOffset = 128;
  static int upToStartDir = -1;

  const int boxWidth = 1;
  const int boxHeight = 8;
  const int boxCount = 32;
  const int screenRight = 128;
  const int boxY = 30;

  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_ncenB10_tr);
    u8g2.drawStr(22, 26, "GAME BOY");
    for (int i = boxCount - visibleCols; i < boxCount; i++) {
      int x = headX - (boxCount - 1 - i) * boxWidth;
      if (x >= 0 && x <= screenRight - boxWidth)
        u8g2.drawBox(x, boxY, boxWidth, boxHeight);
    }
    u8g2.drawStr(upToStartOffset, 55, "UP to Start");
  } while (u8g2.nextPage());

  if (!hiding) {
    if (visibleCols < boxCount) visibleCols++;
    else movementStarted = true;
    if (movementStarted) {
      headX += 2;
      if (headX >= screenRight) hiding = true;
    }
  } else {
    if (visibleCols > 0) visibleCols--;
    else {
      headX = 0;
      visibleCols = 0;
      hiding = false;
      movementStarted = false;
    }
  }
  upToStartOffset += upToStartDir;
  if (upToStartOffset <= 22) upToStartDir = 0;
  delay(80);
}

void TaskDisplayLCD(void *pvParameters) {
  (void) pvParameters;
  chess_Init(u8g2.getU8g2(), 0);
  static uint8_t last_chess_input = 0;

  for (;;) {
    switch (gameState) {
      case STATE_SPLASH:
        drawLogoAnimation();
        if (digitalRead(UP) == LOW) {
          while (digitalRead(UP) == LOW);
          delay(100);
          gameState = STATE_GAME_MENU;
        }
        break;

      case STATE_GAME_MENU:
        menuSelection = u8g2.userInterfaceSelectionList("Select Game", menuSelection, gameMenu);
        if (menuSelection == 1) gameState = STATE_SNAKE_SPEED;
        else if (menuSelection == 2) gameState = STATE_CHESS_GAME;
        else if (menuSelection == 3) gameState = STATE_BREAKOUT_GAME;
        else if (menuSelection == 4) gameState = STATE_DINO_GAME;

        break;

      case STATE_SNAKE_SPEED:
        SnakeGame_SpeedScreen(u8g2, (int&)gameState);
        break;

      case STATE_SNAKE_GAME:
        SnakeGame_Play(u8g2, (int&)gameState);
        break;

      case STATE_SNAKE_SCORE:
        SnakeGame_ScoreScreen(u8g2, (int&)gameState);
        break;
      case STATE_BREAKOUT_GAME:
        BreakoutGame_Loop(u8g2, (int&)gameState);
        break;
      case STATE_DINO_GAME:
        DinoGame_Loop((int&)gameState);
        break;

      case STATE_CHESS_GAME: {
        uint8_t input = 0;
        if (digitalRead(DOWN) == LOW) input = U8X8_MSG_GPIO_MENU_NEXT;
        else if (digitalRead(UP) == LOW) input = U8X8_MSG_GPIO_MENU_PREV;
        else if (digitalRead(LEFT) == LOW) input = U8X8_MSG_GPIO_MENU_HOME;
        else if (digitalRead(RIGHT) == LOW) input = U8X8_MSG_GPIO_MENU_SELECT;

        if (input != last_chess_input) {
          if (input != 0) chess_Step(input);
          last_chess_input = input;
        }

        u8g2.firstPage();
        do {
          chess_Draw();
        } while (u8g2.nextPage());
        break;
      }
    }
  }
}

void TaskHandleButton(void *pvParameters) {
  (void) pvParameters;
  for (;;) {
    SnakeGame_HandleInput();
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(LEFT, INPUT_PULLUP);
  pinMode(RIGHT, INPUT_PULLUP);
  pinMode(DOWN, INPUT_PULLUP);
  pinMode(UP, INPUT_PULLUP);
  pinMode(JOYSTICK_X, INPUT);
  pinMode(JOYSTICK_Y, INPUT);

  u8g2.begin(UP, DOWN, LEFT, U8X8_PIN_NONE, U8X8_PIN_NONE, U8X8_PIN_NONE);
  u8g2.setFont(u8g2_font_ncenB10_tr);
  randomSeed(analogRead(0));

  SnakeGame_Init();  // 初始化貪食蛇模組
  DinoGame_SetDisplay(u8g2);  // 告訴 DinoGame 使用這個 u8g2
  DinoGame_Init();            // 初始化遊戲變數

  xTaskCreate(TaskDisplayLCD, "Task1", 200, NULL, 1, &TaskHandle_1);
  xTaskCreate(TaskHandleButton, "Task2", 100, NULL, 1, &TaskHandle_2);
}

void loop() {}*/