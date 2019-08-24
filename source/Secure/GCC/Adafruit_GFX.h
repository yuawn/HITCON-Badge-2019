#ifndef _ADAFRUIT_GFX_H
#define _ADAFRUIT_GFX_H
#include <arm_cmse.h>
#include <stdio.h>
#include "NuMicro.h"                      /* Device header */


void drawPixel(int16_t x, int16_t y, int color);
void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int color);
void drawFastVLine(int16_t x, int16_t y, int16_t h, int color);
void drawFastHLine(int16_t x, int16_t y, int16_t w, int color);
void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, int color);
void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, int color);
void fillScreen(int color);
void invertDisplay(int i);

void  drawCircle(int16_t x0, int16_t y0, int16_t r, int color);
void  drawCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername,
    int color);
void  fillCircle(int16_t x0, int16_t y0, int16_t r, int color);
void  fillCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername,
    int16_t delta, int color);
void  drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
    int16_t x2, int16_t y2, int color);
void  fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
    int16_t x2, int16_t y2, int color);
void  drawRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h,
    int16_t radius, int color);
void  fillRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h,
    int16_t radius, int color);
void  drawBitmap(int16_t x, int16_t y, uint8_t *bitmap,
    int16_t w, int16_t h, int color);
void  drawXBitmap(int16_t x, int16_t y, const uint8_t *bitmap,
    int16_t w, int16_t h, int color);

const int16_t
  WIDTH, HEIGHT;   // This is the 'raw' display w/h - never changes

const int16_t _width, _height;

#endif // _ADAFRUIT_GFX_H
