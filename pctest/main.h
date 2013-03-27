
#ifndef __MAIN_H_
#define __MAIN_H_

/*
 * This includes references to things in the main compilation file
 * (currently fonttest.c), so other functions can access them.
 */

#include <stdint.h>

#define WIDTH 320
#define HEIGHT 200
#define UPSCALE 2


extern uint8_t image[];


#ifdef DEBUG
extern int callcount_bezier;
extern int callcount_setpixel;
#endif //DEBUG

#endif //__MAIN_H_
