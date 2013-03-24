#ifndef __PIXELPARTICLES_H__
#define __PIXELPARTICLES_H__

#define NumberOfPixelParticles (1024*6+512)

#include <stdint.h>

struct PixelParticlesData
{
	struct
	{
		int32_t x,y;
	} particles[NumberOfPixelParticles];
};

void PixelParticles();

#endif
