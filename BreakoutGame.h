#ifndef BREAKOUT_GAME_H
#define BREAKOUT_GAME_H

#include <U8g2lib.h>

void BreakoutGame_Init();
void BreakoutGame_Loop(U8G2 &u8g2, int &gameState);

#endif
