#ifndef __GREETSTARS_H_
#define __GREETSTARS_H_

#include "Graphics/Bitmap.h"

#define GREETSTARS_COUNT 100

typedef struct {
    int x;
    int y; 
    int delta;
} greetstars_t;

void greetstars_draw(Bitmap* dest);
void greetstars_init();





#endif //__GREETSTARS_H_
