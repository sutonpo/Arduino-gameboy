// LittleRookChess.h
#ifndef LITTLE_ROOK_CHESS_H
#define LITTLE_ROOK_CHESS_H

#include <U8g2lib.h>

void chess_Init(u8g2_t *u8g, uint8_t body_color);
void chess_Draw();
void chess_Step(uint8_t keycode);

#endif
