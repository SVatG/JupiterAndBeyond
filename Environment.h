#ifndef __ENVIRONMENT_H__
#define __ENVIRONMENT_H__

#include "VectorLibrary/VectorFixed.h"

#define NumberOfSegments 24
#define NumberOfSectors 24

struct EnvironmentData
{
	ivec2_t p[NumberOfSegments][NumberOfSectors];
	ivec2_t t[NumberOfSegments][NumberOfSectors];
};

void Environment();

#endif
