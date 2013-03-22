#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include "Rasterize.h"
#include "LogoShow.h"
#include "Voxelscape.h"

#include "Button.h"

extern union GlobalData
{
    struct RasterizeData rasterizer;
    struct LogoShowData logoshow;
    struct VoxelscapeData voxel;
} data;

#endif
