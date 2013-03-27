#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include "Fields.h"
#include "LogoShow.h"
#include "PixelParticles.h"
#include "Rasterize.h"
#include "Starfield.h"
#include "Voxelscape.h"
#include "Greets.h"

#include "Button.h"

extern union GlobalData
{
    struct FieldsData fields;
    struct LogoShowData logoshow;
	struct PixelParticlesData pp;
    struct RasterizeData rasterizer;
    struct StarfieldData stars;
    struct VoxelscapeData voxel;
    struct GreetsData greets;
} data;

#endif
