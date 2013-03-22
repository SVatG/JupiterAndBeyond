#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include "Fields.h"
#include "LogoShow.h"
#include "Rasterize.h"
#include "Voxelscape.h"

#include "Button.h"

extern union GlobalData
{
    struct FieldsData fields;
    struct RasterizeData rasterizer;
    struct LogoShowData logoshow;
    struct VoxelscapeData voxel;
} data;

#endif
