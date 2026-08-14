// DinoGame.h
#ifndef DINOGAME_H
#define DINOGAME_H

#include <U8g2lib.h>

// 設定 U8g2 顯示器指標（由主程式傳入）
void DinoGame_SetDisplay(U8G2 &display);

// 初始化 Dino 遊戲狀態
void DinoGame_Init();

// 主遊戲迴圈（由主程式每幀呼叫）
void DinoGame_Loop(int &gameState);

// （可選）渲染靜態畫面
void DinoGame_RenderScene(int16_t i = 0);

#endif // DINOGAME_H