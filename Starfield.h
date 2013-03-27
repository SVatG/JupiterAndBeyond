#ifndef __STARFIELD_H__
#define __STARFIELD_H__

#include <stdint.h>
#include "fontheader.h"

#define SquareRootOfNumberOfStars 64
#define NumberOfStars (SquareRootOfNumberOfStars*SquareRootOfNumberOfStars)

struct StarfieldData
{
	struct
	{
		int16_t x,y,z;
	} stars[NumberOfStars];
    bezier_t bez[400];
    int16_t bez_z[400];
};

void Starfield();

void Starfield_inner(uint8_t *source, uint8_t *destination);
void Starfield_init();
#endif
