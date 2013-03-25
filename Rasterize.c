/**
 * Rasterizer
 */

#include <stdlib.h>
#include <string.h>

#include "Random.h"
#include "VectorLibrary/VectorFixed.h"
#include "VectorLibrary/MatrixFixed.h"

#include "VGA.h"
#include "LED.h"
#include "Button.h"
#include "Utils.h"
#include "Graphics/Bitmap.h"
#include "Graphics/Drawing.h"

#define WIDTH 320
#define HEIGHT 200

#define R(x) (F( ((x) & (7 << 5)) >> 5 ))
#define G(x) (F( ((x) & (7 << 2)) >> 2 ))
#define B(x) (F( (x) & (3) ))

#define RastRGB(r,g,b) ((r)<<5 | (g)<<2 | (b))

#define V(x,y,z) {F(x),F(y),F(z),F(1)}

#define Viewport(x,w,s) (imul(idiv((x),(w))+IntToFixed(1),IntToFixed((s)/2)))

#define MAP_W 50
#define UV(x,y) (

#include "Rasterize.h"
#include "Global.h"

static inline int32_t approxabs(int32_t x) { return x^(x>>31); }

inline static void RasterizeTriangle(uint8_t* image, triangle_t* tri, uint8_t shade) {
	// Vertex sorting
	ss_vertex_t upperVertex;
	ss_vertex_t centerVertex;
	ss_vertex_t lowerVertex;

	if(tri->v[0].p.y < tri->v[1].p.y) {
		upperVertex = tri->v[0];
		lowerVertex = tri->v[1];
	}
	else {
		upperVertex = tri->v[1];
		lowerVertex = tri->v[0];
	}

	if(tri->v[2].p.y < upperVertex.p.y) {
		centerVertex = upperVertex;
		upperVertex = tri->v[2];
	}
	else {
		if(tri->v[2].p.y > lowerVertex.p.y) {
			centerVertex = lowerVertex;
			lowerVertex = tri->v[2];
		}
		else {
			centerVertex = tri->v[2];
		}
	}

	// scanline counters
	int32_t scanline;
	int32_t scanlineMax;

	// left / right x and deltas
	int32_t leftX;
	int32_t leftXd;
	int32_t rightX;
	int32_t rightXd;

	// left color and color delta
	int32_t leftU;
	int32_t leftV;
	int32_t leftUd;
	int32_t leftVd;

	// color and color x deltas
	int16_t U;
	int16_t V;
	int16_t UdX;
	int16_t VdX;

	// calculate y differences
	int32_t upperDiff = upperVertex.p.y - centerVertex.p.y;
	int32_t lowerDiff = upperVertex.p.y - lowerVertex.p.y;


	// deltas
	int32_t upperCenter;
	int32_t upperLower;

	// check if we have a triangle at all (Special case A)
	if(lowerDiff == 0 && upperDiff == 0) {
		return;
	}

	// calculate whole-triangle deltas
	int32_t temp = idiv(centerVertex.p.y-upperVertex.p.y,lowerVertex.p.y-upperVertex.p.y);
	int32_t width = imul(temp, (lowerVertex.p.x-upperVertex.p.x)) + (upperVertex.p.x-centerVertex.p.x);
	if(width == 0) {
		return;
	}
	UdX = idiv(imul(temp, IntToFixed(lowerVertex.uw-upperVertex.uw)) + IntToFixed(upperVertex.uw-centerVertex.uw),width);
        VdX = idiv(imul(temp, IntToFixed(lowerVertex.vw-upperVertex.vw)) + IntToFixed(upperVertex.vw-centerVertex.vw),width);
	
	// guard against special case B: flat upper edge
	if(upperDiff == 0 ) {

		if(upperVertex.p.x < centerVertex.p.x) {
			leftX = upperVertex.p.x;
                        leftU = IntToFixed(upperVertex.uw);
                        leftV = IntToFixed(upperVertex.vw);
			rightX = centerVertex.p.x;

			leftXd = idiv(upperVertex.p.x - lowerVertex.p.x, lowerDiff);
			rightXd = idiv(centerVertex.p.x - lowerVertex.p.x, lowerDiff);
		}
		else {
			leftX = centerVertex.p.x;
			leftU = IntToFixed(centerVertex.uw);
                        leftV = IntToFixed(centerVertex.vw);
			rightX = upperVertex.p.x;

			leftXd = idiv(centerVertex.p.x - lowerVertex.p.x, lowerDiff);
			rightXd = idiv(upperVertex.p.x - lowerVertex.p.x, lowerDiff);
		}

		leftUd = idiv(leftU - IntToFixed(lowerVertex.uw), lowerDiff);
                leftVd = idiv(leftV - IntToFixed(lowerVertex.vw), lowerDiff);

		goto lower_half_render;
	}

	// calculate deltas
	upperCenter = idiv(upperVertex.p.x - centerVertex.p.x, upperDiff);
	upperLower = idiv(upperVertex.p.x - lowerVertex.p.x, lowerDiff);

	// upper triangle half
	leftX = rightX = upperVertex.p.x;

	leftU = IntToFixed(upperVertex.uw);
	leftV = IntToFixed(upperVertex.vw);

	if(upperCenter < upperLower) {
		leftXd = upperCenter;
		rightXd = upperLower;

		leftUd = idiv(leftU - IntToFixed(centerVertex.uw), upperDiff);
		leftVd = idiv(leftV - IntToFixed(centerVertex.vw), upperDiff);
	}
	else {
		leftXd = upperLower;
		rightXd = upperCenter;

		leftUd = idiv(leftU - IntToFixed(lowerVertex.uw), lowerDiff);
		leftVd = idiv(leftV - IntToFixed(lowerVertex.vw), lowerDiff);
	}

	U = leftU;
	V = leftV;
	
        scanlineMax = imin(FixedToRoundedInt(centerVertex.p.y), HEIGHT-1);
	for(scanline = FixedToRoundedInt(upperVertex.p.y); scanline < scanlineMax; scanline++ ) {
                if(scanline >= 0) {
                    int32_t xMax = imin(FixedToRoundedInt(rightX), WIDTH-1);
                    if(xMax >= 0) {
                        int32_t offset = scanline*WIDTH;
                        int32_t x = FixedToRoundedInt(leftX);
        
                        while(x <= -1) {
                            U += UdX;
                            V += VdX;
                            x++;
                        }
                        while(x <= xMax) {
                            uint8_t hb = U > IntToFixed(1) && U < IntToFixed(6);
                            uint8_t vb = FixedToInt(V) & 1;
                            uint8_t tb = (hb&&vb);
                            image[x+offset] = RastRGB(
                                tb * 7 + (1-tb) * (shade<<1),
                                tb * 6 + (1-tb) * (shade<<1),
                                tb * 0 + (1-tb) * shade
                            );
                            x++;
                            U += UdX;
                            V += VdX;
                        }
                    }
                }

                leftX += leftXd;
                rightX += rightXd;
                leftU += leftUd;
                leftV += leftVd;

                U = leftU;
                V = leftV;
	}
        
	// Guard against special case C: flat lower edge
	int32_t centerDiff = centerVertex.p.y - lowerVertex.p.y;
	if(centerDiff == 0) {
		return;
	}

	// calculate lower triangle half deltas
	if(upperCenter < upperLower) {
		leftX = centerVertex.p.x;
		leftXd = idiv(centerVertex.p.x - lowerVertex.p.x, centerDiff);

		leftU = IntToFixed(centerVertex.uw);
		leftV = IntToFixed(centerVertex.vw);

		leftUd = idiv(leftU - IntToFixed(lowerVertex.uw), centerDiff);
		leftVd = idiv(leftV - IntToFixed(lowerVertex.vw), centerDiff);
	}
	else {
		rightX = centerVertex.p.x;
		rightXd = idiv(centerVertex.p.x - lowerVertex.p.x, centerDiff);
	}

lower_half_render:

	// lower triangle half
	scanlineMax = imin(FixedToRoundedInt(lowerVertex.p.y), HEIGHT-1);
        
	U = leftU;
	V = leftV;

	for(scanline = FixedToRoundedInt(centerVertex.p.y); scanline < scanlineMax; scanline++ ) {
                if(scanline >= 0) {
                    int32_t xMax = imin(FixedToRoundedInt(rightX), WIDTH-1);
                    if(xMax >= 0) {
                        int32_t offset = scanline*WIDTH;
                        int32_t x = FixedToRoundedInt(leftX);

                        while(x <= -1) {
                            U += UdX;
                            V += VdX;
                            x++;
                        }
                        while(x <= xMax) {
                            uint8_t hb = U > IntToFixed(1) && U < IntToFixed(6);
                            uint8_t vb = FixedToInt(V) & 1;
                            uint8_t tb = (hb&&vb);
                            image[x+offset] = RastRGB(
                                tb * 7 + (1-tb) * (shade<<1),
                                tb * 6 + (1-tb) * (shade<<1),
                                tb * 0 + (1-tb) * shade
                            );
                            x++;
                            U += UdX;
                            V += VdX;
                        }
                    }
                }
                
		leftX += leftXd;
		rightX += rightXd;
		leftU += leftUd;
		U = leftU;
		leftV += leftVd;
		V = leftV;
	}
}

// Terrible code because I got tired of trying to fix things
inline static void RasterizeTriangleSingle(uint8_t* image, triangle_t* tri, uint8_t shade) {
        // Vertex sorting
        ss_vertex_t upperVertex;
        ss_vertex_t centerVertex;
        ss_vertex_t lowerVertex;

        if(tri->v[0].p.y < tri->v[1].p.y) {
                upperVertex = tri->v[0];
                lowerVertex = tri->v[1];
        }
        else {
                upperVertex = tri->v[1];
                lowerVertex = tri->v[0];
        }

        if(tri->v[2].p.y < upperVertex.p.y) {
                centerVertex = upperVertex;
                upperVertex = tri->v[2];
        }
        else {
                if(tri->v[2].p.y > lowerVertex.p.y) {
                        centerVertex = lowerVertex;
                        lowerVertex = tri->v[2];
                }
                else {
                        centerVertex = tri->v[2];
                }
        }

        // scanline counters
        int32_t scanline;
        int32_t scanlineMax;

        // left / right x and deltas
        int32_t leftX;
        int32_t leftXd;
        int32_t rightX;
        int32_t rightXd;

        // left color and color delta
        int32_t leftU;
        int32_t leftV;
        int32_t leftUd;
        int32_t leftVd;

        // color and color x deltas
        int16_t U;
        int16_t V;
        int16_t UdX;
        int16_t VdX;

        // calculate y differences
        int32_t upperDiff = upperVertex.p.y - centerVertex.p.y;
        int32_t lowerDiff = upperVertex.p.y - lowerVertex.p.y;


        // deltas
        int32_t upperCenter;
        int32_t upperLower;

        // check if we have a triangle at all (Special case A)
        if(lowerDiff == 0 && upperDiff == 0) {
                return;
        }

        // calculate whole-triangle deltas
        int32_t temp = idiv(centerVertex.p.y-upperVertex.p.y,lowerVertex.p.y-upperVertex.p.y);
        int32_t width = imul(temp, (lowerVertex.p.x-upperVertex.p.x)) + (upperVertex.p.x-centerVertex.p.x);
        if(width == 0) {
                return;
        }
        UdX = idiv(imul(temp, IntToFixed(lowerVertex.uw-upperVertex.uw)) + IntToFixed(upperVertex.uw-centerVertex.uw),width);
        VdX = idiv(imul(temp, IntToFixed(lowerVertex.vw-upperVertex.vw)) + IntToFixed(upperVertex.vw-centerVertex.vw),width);

        // guard against special case B: flat upper edge
        if(upperDiff == 0 ) {

                if(upperVertex.p.x < centerVertex.p.x) {
                        leftX = upperVertex.p.x;
                        leftU = IntToFixed(upperVertex.uw);
                        leftV = IntToFixed(upperVertex.vw);
                        rightX = centerVertex.p.x;

                        leftXd = idiv(upperVertex.p.x - lowerVertex.p.x, lowerDiff);
                        rightXd = idiv(centerVertex.p.x - lowerVertex.p.x, lowerDiff);
                }
                else {
                        leftX = centerVertex.p.x;
                        leftU = IntToFixed(centerVertex.uw);
                        leftV = IntToFixed(centerVertex.vw);
                        rightX = upperVertex.p.x;

                        leftXd = idiv(centerVertex.p.x - lowerVertex.p.x, lowerDiff);
                        rightXd = idiv(upperVertex.p.x - lowerVertex.p.x, lowerDiff);
                }

                leftUd = idiv(leftU - IntToFixed(lowerVertex.uw), lowerDiff);
                leftVd = idiv(leftV - IntToFixed(lowerVertex.vw), lowerDiff);

                goto lower_half_render2;
        }

        // calculate deltas
        upperCenter = idiv(upperVertex.p.x - centerVertex.p.x, upperDiff);
        upperLower = idiv(upperVertex.p.x - lowerVertex.p.x, lowerDiff);

        // upper triangle half
        leftX = rightX = upperVertex.p.x;

        leftU = IntToFixed(upperVertex.uw);
        leftV = IntToFixed(upperVertex.vw);

        if(upperCenter < upperLower) {
                leftXd = upperCenter;
                rightXd = upperLower;

                leftUd = idiv(leftU - IntToFixed(centerVertex.uw), upperDiff);
                leftVd = idiv(leftV - IntToFixed(centerVertex.vw), upperDiff);
        }
        else {
                leftXd = upperLower;
                rightXd = upperCenter;

                leftUd = idiv(leftU - IntToFixed(lowerVertex.uw), lowerDiff);
                leftVd = idiv(leftV - IntToFixed(lowerVertex.vw), lowerDiff);
        }

        U = leftU;
        V = leftV;

        scanlineMax = imin(FixedToRoundedInt(centerVertex.p.y), HEIGHT-1);
        for(scanline = FixedToRoundedInt(upperVertex.p.y); scanline < scanlineMax; scanline++ ) {
                if(scanline >= 0) {
                    int32_t xMax = imin(FixedToRoundedInt(rightX), WIDTH-1);
                    if(xMax >= 0) {
                        int32_t offset = scanline*WIDTH;
                        int32_t x = FixedToRoundedInt(leftX);

                        while(x <= -1) {
                            U += UdX;
                            V += VdX;
                            x++;
                        }
                        while(x <= xMax) {
                            image[x+offset] = RastRGB(
                                (shade<<1),
                                (shade<<1),
                                shade
                            );
                            x++;
                            U += UdX;
                            V += UdX;
                        }
                    }
                }

                leftX += leftXd;
                rightX += rightXd;
                leftU += leftUd;
                leftV += leftVd;

                U = leftU;
                V = leftV;
        }

        // Guard against special case C: flat lower edge
        int32_t centerDiff = centerVertex.p.y - lowerVertex.p.y;
        if(centerDiff == 0) {
                return;
        }

        // calculate lower triangle half deltas
        if(upperCenter < upperLower) {
                leftX = centerVertex.p.x;
                leftXd = idiv(centerVertex.p.x - lowerVertex.p.x, centerDiff);

                leftU = IntToFixed(centerVertex.uw);
                leftV = IntToFixed(centerVertex.vw);

                leftUd = idiv(leftU - IntToFixed(lowerVertex.uw), centerDiff);
                leftVd = idiv(leftV - IntToFixed(lowerVertex.vw), centerDiff);
        }
        else {
                rightX = centerVertex.p.x;
                rightXd = idiv(centerVertex.p.x - lowerVertex.p.x, centerDiff);
        }

lower_half_render2:

        // lower triangle half
        scanlineMax = imin(FixedToRoundedInt(lowerVertex.p.y), HEIGHT-1);

        U = leftU;
        V = leftV;

        for(scanline = FixedToRoundedInt(centerVertex.p.y); scanline < scanlineMax; scanline++ ) {
                if(scanline >= 0) {
                    int32_t xMax = imin(FixedToRoundedInt(rightX), WIDTH-1);
                    if(xMax >= 0) {
                        int32_t offset = scanline*WIDTH;
                        int32_t x = FixedToRoundedInt(leftX);

                        while(x <= -1) {
                            U += UdX;
                            V += VdX;
                            x++;
                        }
                        while(x <= xMax) {
                            image[x+offset] = RastRGB(
                                (shade<<1),
                                (shade<<1),
                                shade
                            );
                            x++;
                            U += UdX;
                            V += VdX;
                        }
                    }
                }

                leftX += leftXd;
                rightX += rightXd;
                leftU += leftUd;
                U = leftU;
                leftV += leftVd;
                V = leftV;
        }
}

static int triAvgDepthCompare(const void *p1, const void *p2) {
	index_triangle_t* t1 = (index_triangle_t*)p1;
	index_triangle_t* t2 = (index_triangle_t*)p2;
	return(
		data.rasterizer.transformedVertices[t1->v[0]].p.z +
		data.rasterizer.transformedVertices[t1->v[1]].p.z +
		data.rasterizer.transformedVertices[t1->v[2]].p.z -
		data.rasterizer.transformedVertices[t2->v[0]].p.z -
		data.rasterizer.transformedVertices[t2->v[1]].p.z -
		data.rasterizer.transformedVertices[t2->v[2]].p.z
	);
}

static int triClosestDepthCompare(const void *p1, const void *p2) {
    index_triangle_t* t1 = (index_triangle_t*)p1;
    index_triangle_t* t2 = (index_triangle_t*)p2;
    int d1 = imin(imin(
            data.rasterizer.transformedVertices[t1->v[0]].p.z,
            data.rasterizer.transformedVertices[t1->v[1]].p.z
        ),
        data.rasterizer.transformedVertices[t1->v[2]].p.z
    );
    int d2 = imin(imin(
            data.rasterizer.transformedVertices[t2->v[0]].p.z,
            data.rasterizer.transformedVertices[t2->v[1]].p.z
        ),
        data.rasterizer.transformedVertices[t2->v[2]].p.z
    );
    return(
        d2 - d1
    );
}

uint32_t startFrame;
void RasterizeInit() {
	
	memcpy(data.rasterizer.sortedTriangles,faces,sizeof(index_triangle_t)*numFaces);
	startFrame = VGAFrame;

        #define ALIGN_THRESH 4096
        
        for(int32_t i = 0; i < numFaces; i++ ) {
            // Find same-height verts
            int32_t flatvert1;
            int32_t flatvert2;
            int32_t thirdvert;
            
            init_vertex_t origVertices[3];
            for(int ver = 0; ver < 3; ver++) {
                origVertices[ver] = vertices[data.rasterizer.sortedTriangles[i].v[ver]];
            }

            if(iabs(origVertices[0].y - origVertices[1].y) < ALIGN_THRESH) {
                // top vertices are 01
                flatvert1 = 0;
                flatvert2 = 1;
                thirdvert = 2;
            }
            else {
                if(iabs(origVertices[0].y - origVertices[2].y) < ALIGN_THRESH) {
                    // top vertices are 02
                    flatvert1 = 0;
                    flatvert2 = 2;
                    thirdvert = 1;
                }
                else {
                    // top vertices are 12
                    flatvert1 = 1;
                    flatvert2 = 2;
                    thirdvert = 0;
                }
            }

            // Find which top-vertex bottom vertex aligns with
            int32_t alignedvert;
            if(iabs(origVertices[flatvert1].x - origVertices[thirdvert].x) < ALIGN_THRESH) {
                if(iabs(origVertices[flatvert1].z - origVertices[thirdvert].z)  < ALIGN_THRESH) {
                    alignedvert = flatvert1;
                }
                else {
                    alignedvert = flatvert2;
                }
            }
            else {
                alignedvert = flatvert2;
            }

            // Store triangle info
            data.rasterizer.sortedTriangles[i].indices = (flatvert1 << 6) | (flatvert2 << 4) | (thirdvert << 2) | (alignedvert);
        }
}

inline static void RasterizeTest(uint8_t* image) {
        
	int32_t rotcnt = (VGAFrame - startFrame);
        int32_t rowd = rotcnt;
        
	int32_t render_faces_total_start = 0;
	int32_t render_faces_total_end = numFaces;

        ivec3_t tolight = ivec3norm(imat3x3transform(
                imat3x3rotatey(256),
                ivec3(F(0),isin((rotcnt*20)%4096), icos((rotcnt*20)%4096))
        ));
	
	// Projection matrix
	imat4x4_t proj = imat4x4diagonalperspective(IntToFixed(45),idiv(IntToFixed(WIDTH),IntToFixed(HEIGHT)),4096,IntToFixed(400));
	
	// Modelview matrix
	int rotdir = /*(rowd>>4)%2 == 0 ? -1 : */1;
        imat4x4_t modelview = imat4x4lookat(
            ivec3(IntToFixed(5), IntToFixed(-20), IntToFixed(-150)+(rotcnt<<10)),
            ivec3(IntToFixed(0), IntToFixed(5), IntToFixed(0)),
            ivec3(IntToFixed(0), IntToFixed(1), IntToFixed(0))
        );
	
	// Transform
	vertex_t transformVertex;
	for(int32_t i = 0; i < numVertices; i++) {
		transformVertex.p = imat4x4transform(modelview,ivec4(vertices[i].x,vertices[i].y,vertices[i].z,F(1)));
                if(transformVertex.p.z <= 4096) {
                    data.rasterizer.transformedVertices[i].clip = 1;
                    continue;
                }
                else {
                    data.rasterizer.transformedVertices[i].clip = 0;
                }
                
		// Project
		transformVertex.p = imat4x4transform(proj,transformVertex.p);
                
		// Perspective divide and viewport transform
		data.rasterizer.transformedVertices[i].p = ivec3(
			Viewport(transformVertex.p.x,transformVertex.p.w,WIDTH),
			Viewport(transformVertex.p.y,transformVertex.p.w,HEIGHT),
			transformVertex.p.z
		);
        }

        
	// Depth sort
	qsort(data.rasterizer.sortedTriangles,numFaces,sizeof(index_triangle_t),&triAvgDepthCompare);

        
	// For each triangle
	triangle_t tri;
        uint32_t dontrasterize = 0;
        for(int32_t i = render_faces_total_start; i < render_faces_total_end; i++ ) {
                dontrasterize = 0;

                for(int ver = 0; ver < 3; ver++) {
                        tri.v[ver] = data.rasterizer.transformedVertices[data.rasterizer.sortedTriangles[i].v[ver]];
                }
                
                for(int ver = 0; ver < 3; ver++) {
                        // Whole-triangle clipper
                        if(tri.v[ver].clip == 1) {
                            dontrasterize = 1;
                            break;
                        }
                        
                        // Winding test
                        if(
                            imul(tri.v[1].p.x - tri.v[0].p.x, tri.v[2].p.y - tri.v[0].p.y) -
                            imul(tri.v[2].p.x - tri.v[0].p.x, tri.v[1].p.y - tri.v[0].p.y)
                            > 0
                        ) {
                            dontrasterize = 1;
                            break;
                        }
                }

                if(dontrasterize == 1) {
                        continue;
                }

                // Get indices
                uint8_t indices = data.rasterizer.sortedTriangles[i].indices;
                uint8_t flatvert1 = (indices >> 6) & 3;
                uint8_t flatvert2 = (indices >> 4) & 3;
                uint8_t thirdvert = (indices >> 2) & 3;
                uint8_t alignedvert = (indices) & 3;
                
                // Texturize
                tri.v[flatvert1].uw = 0;
                tri.v[flatvert2].uw = 7;
                tri.v[thirdvert].uw = tri.v[alignedvert].uw;

                tri.v[0].vw = vertices[data.rasterizer.sortedTriangles[i].v[0]].y >> 11;
                tri.v[1].vw = vertices[data.rasterizer.sortedTriangles[i].v[1]].y >> 11;
                tri.v[2].vw = vertices[data.rasterizer.sortedTriangles[i].v[2]].y >> 11;

                // Shade
                uint32_t shadev = ivec3dot(
                    tolight,
                    normals[data.rasterizer.sortedTriangles[i].v[3]]
                );
                shadev = imin(imax(F(0), shadev)>>10,2);

                if(tri.v[0].vw == tri.v[1].vw && tri.v[0].vw == tri.v[2].vw) {
                    RasterizeTriangleSingle(image, &tri, shadev+1);
                }
                else {
                    RasterizeTriangle(image, &tri, shadev+1);
                }
	}
	
	rotcnt++;
}

void Rasterize() {
	uint8_t *framebuffer1=(uint8_t *)0x20000000;
	uint8_t *framebuffer2=(uint8_t *)0x20010000;
	memset(framebuffer1,0,320*200);
	memset(framebuffer2,0,320*200);

	Bitmap frame1,frame2;
	InitializeBitmap(&frame1,320,200,320,framebuffer1);
	InitializeBitmap(&frame2,320,200,320,framebuffer2);

        SetVGAScreenMode320x200_60Hz(framebuffer1);
	
	int t=0;

	RasterizeInit();

        while(!UserButtonState())
	{
		WaitVBL();

		Bitmap *currframe;
		if(t&1)
		{
			SetFrameBuffer(framebuffer1);
			ClearBitmap(&frame2);
			currframe=&frame2;
		}
		else
		{
			SetFrameBuffer(framebuffer2);
			ClearBitmap(&frame1);
			currframe=&frame1;
		}

		SetLEDs(0);

		uint8_t* pixels = currframe->pixels;
		RasterizeTest(pixels);

		t++;
	}

	while(UserButtonState());
}
