#ifndef _APA102_H_
#define _APA102_H_


#include <arm_cmse.h>
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;

uint8_t    g_PixelBrightness;                             // Global brightness setting



#define DOTSTAR_RGB (0 | (1 << 2) | (2 << 4))
#define DOTSTAR_RBG (0 | (2 << 2) | (1 << 4))
#define DOTSTAR_GRB (1 | (0 << 2) | (2 << 4))
#define DOTSTAR_GBR (2 | (0 << 2) | (1 << 4))
#define DOTSTAR_BRG (1 | (2 << 2) | (0 << 4))
#define DOTSTAR_BGR (2 | (1 << 2) | (0 << 4))
#define DOTSTAR_MONO 0 // Single-color strip WIP DO NOT USE YET


//void updateLength(uint16_t n);

void sw_spi_out(uint8_t n);
void PixelShow(void);
void ClearPixels();
void SetPixelColorRGB(uint16_t n, uint8_t r, uint8_t g, uint8_t b);
void SetPixelColor(uint16_t n, uint32_t c);

#endif // _APA102_H_
