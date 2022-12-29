#include <stdio.h>
#include "pico/stdlib.h"


void draw_rec_onbuf (int16_t x, int16_t y, int16_t w, int16_t h, uint8_t* displayBuf, uint16_t color){
    //x=>w y=>h
	//start point: 80*y+x
    if (x < 0){
        w = w + x;
        x = 0;
    }
    else if (w + x > 80){
        w = 80 - x;
    }

    if (y < 0){
        h = h + y;
        y = 0;
    }
    else if (h + y > 160){
        h = 160 - y;
    }

    int index = (y * 80 + x) * 2;
    //printf("%d,%d\n",w,x);
    for (int i = 0; i < h; i++){
        for (int j = 0; j < w; j++){
            displayBuf[index+j*2] = (uint8_t)(color >> 8) & 0xFF;
            displayBuf[index+j*2+1] = (uint8_t)(color)&0xFF;
        }
        index = index + 80 * 2;
    }
}

void draw_car(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t* displayBuf, uint16_t color){
    draw_rec_onbuf(x,y,w,h,displayBuf,color);
}

uint16_t car_x_lim(uint16_t x, uint16_t w){
    if (x < 6) x = 6;
    else if (x + w > 74) x = 74 - w;
    return x;
}

uint16_t car_y_lim(int16_t y){
    if (y > 120) y = 120;
    else if (y < 0) y = 0;
    return y;
}