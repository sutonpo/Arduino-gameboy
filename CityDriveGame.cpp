#include "racing_scene_pixels.h"

#include "CityDriveGame.h"

CityDriveGame::CityDriveGame(U8G2& display) : u8g2(display) {}

void CityDriveGame::begin() {
    playerLane = 1;
    roadScroll = 0;
}

void CityDriveGame::update() {
    roadScroll = (roadScroll + 1) % 16;
}

void CityDriveGame::draw() {
    u8g2.clearBuffer();

    // 👉 1. 背景圖（底圖）
    drawRacingScene(u8g2);



    // 👉 3. 最後送出畫面
    u8g2.sendBuffer();
}

void CityDriveGame::moveLeft() {
    if (playerLane > 0) playerLane--;
}

void CityDriveGame::moveRight() {
    if (playerLane < laneCount - 1) playerLane++;
}
