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
        uint8_t clip;
	int16_t uw;
        int16_t vw;
} ss_vertex_t;

typedef struct {
	ss_vertex_t v[3];
} triangle_t;

typedef struct {
	int16_t v[4];
        uint8_t indices;
} index_triangle_t;

typedef ivec3_t init_vertex_t;

#include "Cityscape.h"

#define NumberOfDotStars 2
struct RasterizeData {
	ss_vertex_t transformedVertices[numVertices];
	index_triangle_t sortedTriangles[numFaces];
};


#endif
