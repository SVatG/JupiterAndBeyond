#ifndef __CREDITS_H_
#define __CREDITS_H_


#include "Graphics/Bitmap.h"
#include "fontheader.h"

void Credits();
void credits_init();
void credits_inner(Bitmap* screen);

struct CreditsData
{
    fillpixel_t fillpixels[5000];
};




#endif // __CREDITS_H_

