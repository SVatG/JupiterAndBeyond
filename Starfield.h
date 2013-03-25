#ifndef __STARFIELD_H__
#define __STARFIELD_H__

#include <stdint.h>

#define NumberOfStars 4096

struct StarfieldData
{
	struct
	{
		int16_t x,y,z;
	} stars[NumberOfStars];
};

void Starfield();

#endif
