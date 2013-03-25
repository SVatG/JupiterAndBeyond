#ifndef __STARFIELD_H__
#define __STARFIELD_H__

#include <stdint.h>

#define SquareRootOfNumberOfStars 64
#define NumberOfStars (SquareRootOfNumberOfStars*SquareRootOfNumberOfStars)

struct StarfieldData
{
	struct
	{
		int16_t x,y,z;
	} stars[NumberOfStars];
};

void Starfield();

#endif
