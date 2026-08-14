
#ifndef CITY_DRIVE_GAME_H
#define CITY_DRIVE_GAME_H

#include <Arduino.h>
#include <U8g2lib.h>

class CityDriveGame {
public:
    CityDriveGame(U8G2& display);
    void begin();
    void update();
    void draw();
    void moveLeft();
    void moveRight();

private:
    U8G2& u8g2;

    static const int screenWidth = 128;
    static const int screenHeight = 64;
    static const int carY = 52;
    static const int carWidth = 16;
    static const int carHeight = 8;
    static const int laneWidth = 20;
    static const int laneCount = 3;

    int playerLane;
    int roadScroll;
};

#endif
