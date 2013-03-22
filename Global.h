#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include "Rasterize.h"
#include "LogoShow.h"

extern union GlobalData
{
    struct RasterizeData rasterizer;
    struct LogoShowData logoshow;
    
} data;

#endif
