#ifndef __DRAW_H__
#define __DRAW_H__

#include <stdio.h>
#include "pico/stdlib.h"


void draw_rec_onbuf (int16_t x, int16_t y, int16_t w, int16_t h, uint8_t* displaybuf, uint16_t color);
void draw_car(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t* displayBuf, uint16_t color);

uint16_t car_x_lim(uint16_t x, uint16_t w);
uint16_t car_y_lim(int16_t y);

#endif