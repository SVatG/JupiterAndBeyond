#ifndef __BEZIER_H_
#define __BEZIER_H_

#define BEZ_SCALEDOWN 16
//#define BEZ_STEPS 128
//#define BEZ_ZERO 32768
#define BEZ_ZERO 16384
#define BEZ_ZERO_SCREEN (BEZ_ZERO/BEZ_SCALEDOWN)


//#include "main.h"
#include "fontheader.h"

#include "Graphics/Bitmap.h"


//void bezier_draw_path(Bitmap *dest, bezier_t *data, int len);
void bezier_draw(Bitmap *dest, bezier_t bez);
void bezier_draw_partial(Bitmap *dest, bezier_t bez, int percent);

void bezier_setpixel(Bitmap *dest, point_t p, uint8_t colour);
void bezier_draw_colored(Bitmap *dest, bezier_t bez, uint8_t color);

fillpixel_t* bezier_fill_writepixels(Bitmap *dest, bezier_t bez, fillpixel_t* pixelptr, uint8_t color);
void floodfill(Bitmap *dest, fillpixel_t* start, fillpixel_t* end, uint8_t color);

#endif // __BEZIER_H_
