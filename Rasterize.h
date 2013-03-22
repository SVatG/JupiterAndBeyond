#ifndef __RASTERIZE_H__
#define __RASTERIZE_H__

#include "VectorLibrary/VectorFixed.h"
#include "VectorLibrary/MatrixFixed.h"

void RasterizeInit();
void Rasterize();

typedef struct {
	ivec4_t p;
	ivec3_t n;
	uint32_t c;
} vertex_t;

typedef struct {
	ivec3_t p;
//         ivec3_t n;
	uint32_t c;
} ss_vertex_t;

typedef struct {
	ss_vertex_t v[3];
} triangle_t;

typedef struct {
	int32_t v[4];
} index_triangle_t;

typedef ivec3_t init_vertex_t;

#include "Cityscape.h"

#define NumberOfDotStars 300
struct RasterizeData {
	ss_vertex_t transformedVertices[numVertices];
	index_triangle_t sortedTriangles[numFaces];
	struct DotStar1 {
		int x,y,dx,f;
	} dotstars[NumberOfDotStars];
};


#endif
