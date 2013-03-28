#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include "Fields.h"
#include "LogoShow.h"
#include "PixelParticles.h"
#include "Rasterize.h"
#include "Starfield.h"
#include "Voxelscape.h"
#include "Credits.h"
#include "Metablobs.h"
#include "Environment.h"

#include "BitBin.h"

#include "Button.h"

extern BitBinSong* songp;

extern union GlobalData
{
    struct FieldsData fields;
    struct LogoShowData logoshow;
    struct PixelParticlesData pp;
    struct RasterizeData rasterizer;
    struct StarfieldData stars;
    struct VoxelscapeData voxel;
    struct MetablobsData metablobs;
    struct EnvironmentData env;
    struct CreditsData credits;
} data;

#endif
