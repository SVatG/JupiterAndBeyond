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

inline static void RasterizeTriangle(uint8_t* image, triangle_t* tri ) {
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
	UdX = idiv(imul(temp, (R(lowerVertex.c)-R(upperVertex.c))) + (R(upperVertex.c)-R(centerVertex.c)),width);
	VdX = idiv(imul(temp, (G(lowerVertex.c)-G(upperVertex.c))) + (G(upperVertex.c)-G(centerVertex.c)),width);

        
	// 16 bit packed registers
	int32_t dUV;
	int32_t UV;
	int32_t tmpAdd = 0x80008000;
	int32_t unpackMagic = 0x00400200;
	int32_t unpackMask = 0x70007000;
	
	__asm__ volatile(
		"pkhbt %[dUV], %[VdX], %[UdX], lsl #16\n"
		: [dUV] "=r" (dUV)
		: [UdX] "r" (UdX), [VdX] "r" (VdX)
	);
	
	// guard against special case B: flat upper edge
	if(upperDiff == 0 ) {

		if(upperVertex.p.x < centerVertex.p.x) {
			leftX = upperVertex.p.x;
			leftU = IntToFixed(upperVertex.c & (7 << 5))>>5;
			leftV = IntToFixed(upperVertex.c & (7 << 2))>>2;
			rightX = centerVertex.p.x;

			leftXd = idiv(upperVertex.p.x - lowerVertex.p.x, lowerDiff);
			rightXd = idiv(centerVertex.p.x - lowerVertex.p.x, lowerDiff);
		}
		else {
			leftX = centerVertex.p.x;
			leftU = IntToFixed(centerVertex.c & (7 << 5))>>5;
			leftV = IntToFixed(centerVertex.c & (7 << 2))>>2;
			rightX = upperVertex.p.x;

			leftXd = idiv(centerVertex.p.x - lowerVertex.p.x, lowerDiff);
			rightXd = idiv(upperVertex.p.x - lowerVertex.p.x, lowerDiff);
		}

		leftUd = idiv(leftU - (IntToFixed(lowerVertex.c & (7 << 5))>>5), lowerDiff);
		leftVd = idiv(leftV - (IntToFixed(lowerVertex.c & (7 << 2))>>2), lowerDiff);

		goto lower_half_render;
	}

	// calculate deltas
	upperCenter = idiv(upperVertex.p.x - centerVertex.p.x, upperDiff);
	upperLower = idiv(upperVertex.p.x - lowerVertex.p.x, lowerDiff);

	// upper triangle half
	leftX = rightX = upperVertex.p.x;

	leftU = IntToFixed(upperVertex.c & (7 << 5))>>5;
	leftV = IntToFixed(upperVertex.c & (7 << 2))>>2;

	if(upperCenter < upperLower) {
		leftXd = upperCenter;
		rightXd = upperLower;

		leftUd = idiv(leftU - (IntToFixed(centerVertex.c & (7 << 5))>>5), upperDiff);
		leftVd = idiv(leftV - (IntToFixed(centerVertex.c & (7 << 2))>>2), upperDiff);
	}
	else {
		leftXd = upperLower;
		rightXd = upperCenter;

		leftUd = idiv(leftU - (IntToFixed(lowerVertex.c & (7 << 5))>>5), lowerDiff);
		leftVd = idiv(leftV - (IntToFixed(lowerVertex.c & (7 << 2))>>2), lowerDiff);
	}

	U = leftU;
	V = leftV;
	__asm__ volatile(
		"pkhbt %[UV], %[V], %[U], lsl #16\n"
		: [UV] "=r" (UV)
		: [U] "r" (U), [V] "r" (V)
	);
	
        scanlineMax = imin(FixedToRoundedInt(centerVertex.p.y), HEIGHT-1);
	for(scanline = FixedToRoundedInt(upperVertex.p.y); scanline < scanlineMax; scanline++ ) {
                if(scanline > 0) {
                    uint32_t xMax = imin(FixedToRoundedInt(rightX), WIDTH-1);
                    uint32_t offset = scanline*WIDTH;
                    int32_t x = FixedToRoundedInt(leftX);

                    __asm__ volatile(
                            "cmp %[x], #-1\n"
                            "bgt start_inner_loop_a\n"
                            "pre_inner_loop_a:\n"
                            "and r0, %[UV], %[unpackMask]\n"
                            "umull r1, r0, r0, %[unpackMagic]\n"
                            "orr r0, r0, #2\n"
                            "qadd16 %[UV], %[UV], %[dUV]\n"
                            "qadd16 %[UV], %[UV], %[tmpAdd]\n"
                            "qsub16 %[UV], %[UV], %[tmpAdd]\n"
                            "add %[x], #1\n"
                            "cmp %[x], #-1\n"
                            "ble pre_inner_loop_a\n"
                            "start_inner_loop_a:\n"
                            
                            "cmp %[x], %[xMax]\n"
                            "bgt end_inner_loop_a\n"
                            "inner_loop_a:\n"
                            "and r0, %[UV], %[unpackMask]\n"
                            "umull r1, r0, r0, %[unpackMagic]\n"
                            "orr r0, r0, #2\n"
                            "add r1, %[x], %[offset]\n"
                            "strb r0, [%[image], r1]\n"
                            "qadd16 %[UV], %[UV], %[dUV]\n"
                            "qadd16 %[UV], %[UV], %[tmpAdd]\n"
                            "qsub16 %[UV], %[UV], %[tmpAdd]\n"
                            "add %[x], #1\n"
                            "cmp %[x], %[xMax]\n"
                            "ble inner_loop_a\n"
                            "end_inner_loop_a:\n"
                            : [UV] "+r" (UV)
                            : [dUV] "r" (dUV),
                            [x] "r" (x),
                            [xMax] "r" (xMax),
                            [offset] "r" (offset),
                            [image] "r" (image),
                            [unpackMask] "r" (unpackMask),
                            [unpackMagic] "r" (unpackMagic),
                            [tmpAdd] "r" (tmpAdd)
                            : "r0", "r1"
                    );
                }

                leftX += leftXd;
                rightX += rightXd;
                leftU += leftUd;
                leftV += leftVd;

                U = leftU;
                V = leftV;

                __asm__ volatile(
                "pkhbt %[UV], %[V], %[U], lsl #16\n"
                : [UV] "=r" (UV)
                : [U] "r" (U), [V] "r" (V)
                );
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

		leftU = IntToFixed(centerVertex.c & (7 << 5))>>5;
		leftV = IntToFixed(centerVertex.c & (7 << 2))>>2;

		leftUd = idiv(leftU - (IntToFixed(lowerVertex.c & (7 << 5))>>5), centerDiff);
		leftVd = idiv(leftV - (IntToFixed(lowerVertex.c & (7 << 2))>>2), centerDiff);
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

	__asm__ volatile(
		"pkhbt %[UV], %[V], %[U], lsl #16\n"
		: [UV] "=r" (UV)
		: [U] "r" (U), [V] "r" (V)
	);

	for(scanline = FixedToRoundedInt(centerVertex.p.y); scanline < scanlineMax; scanline++ ) {
                if(scanline > 0) {
                    uint32_t xMax = imin(FixedToRoundedInt(rightX), WIDTH-1);
                    uint32_t offset = scanline*WIDTH;
                    int32_t x = FixedToRoundedInt(leftX);
                    __asm__ volatile(
                            "cmp %[x], #-1\n"
                            "bgt start_inner_loop_b\n"
                            "pre_inner_loop_b:\n"
                            "and r0, %[UV], %[unpackMask]\n"
                            "umull r1, r0, r0, %[unpackMagic]\n"
                            "orr r0, r0, #2\n"
                            "qadd16 %[UV], %[UV], %[dUV]\n"
                            "qadd16 %[UV], %[UV], %[tmpAdd]\n"
                            "qsub16 %[UV], %[UV], %[tmpAdd]\n"
                            "add %[x], #1\n"
                            "cmp %[x], #-1\n"
                            "ble pre_inner_loop_b\n"
                            "start_inner_loop_b:\n"

                            "cmp %[x], %[xMax]\n"
                            "bgt end_inner_loop_b\n"
                            "inner_loop_b:\n"
                            "and r0, %[UV], %[unpackMask]\n"
                            "umull r1, r0, r0, %[unpackMagic]\n"
                            "orr r0, r0, #2\n"
                            "add r1, %[x], %[offset]\n"
                            "strb r0, [%[image], r1]\n"
                            "qadd16 %[UV], %[UV], %[dUV]\n"
                            "qadd16 %[UV], %[UV], %[tmpAdd]\n"
                            "qsub16 %[UV], %[UV], %[tmpAdd]\n"
                            "add %[x], #1\n"
                            "cmp %[x], %[xMax]\n"
                            "ble inner_loop_b\n"
                            "end_inner_loop_b:\n"
                            : [UV] "+r" (UV)
                            : [dUV] "r" (dUV),
                            [x] "r" (x),
                            [xMax] "r" (xMax),
                            [offset] "r" (offset),
                            [image] "r" (image),
                            [unpackMask] "r" (unpackMask),
                            [unpackMagic] "r" (unpackMagic),
                            [tmpAdd] "r" (tmpAdd)
                            : "r0", "r1"
                    );
                }
                
		leftX += leftXd;
		rightX += rightXd;
		leftU += leftUd;
		U = leftU;
		leftV += leftVd;
		V = leftV;

		__asm__ volatile(
			"pkhbt %[UV], %[V], %[U], lsl #16\n"
			: [UV] "=r" (UV)
			: [U] "r" (U), [V] "r" (V)
		);
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
	for(int i=0;i<NumberOfDotStars;i++){
		data.rasterizer.dotstars[i].x=(RandomInteger()%352-16)<<12;
		data.rasterizer.dotstars[i].y=RandomInteger()%200;

		int z=isqrt((NumberOfDotStars-1-i)*NumberOfDotStars)*1000/NumberOfDotStars;
		data.rasterizer.dotstars[i].dx=(RandomInteger()%8000+5000);
	}
	
	memcpy(data.rasterizer.sortedTriangles,faces,sizeof(index_triangle_t)*numFaces);
	startFrame = VGAFrame;
}

imat4x4_t notGluLookAt(ivec3_t eye, ivec3_t center, ivec3_t up ){
    ivec3_t forward = ivec3norm(ivec3sub(center, eye));
    ivec3_t side = ivec3norm(ivec3cross(forward, up));
    ivec3_t realup = ivec3cross(forward, side);
    ivec3_t backward = ivec3sub(ivec3(0, 0, 0), forward);
    ivec3_t eyeinv = ivec3sub(ivec3(0, 0, 0), eye);
    imat3x3_t lookdir = imat3x3cols(side, realup, backward);
    imat4x4_t lookat = imat4x4affine3x3(lookdir, eyeinv);
    return lookat;
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
        
	// Do a background
	for(int i=0;i<NumberOfDotStars;i++){
		int32_t x = iabs(data.rasterizer.dotstars[i].x + ((data.rasterizer.dotstars[i].dx * rotcnt)>>2));
		int32_t y = (data.rasterizer.dotstars[i].y + (rotcnt>>1))%HEIGHT;
		image[FixedToInt(x)%WIDTH + y*WIDTH] = RastRGB(7,7,3);
	}
	
	// Projection matrix
	imat4x4_t proj = imat4x4diagonalperspective(IntToFixed(45),idiv(IntToFixed(WIDTH),IntToFixed(HEIGHT)),4096,IntToFixed(400));
	
	// Modelview matrix
	int rotdir = /*(rowd>>4)%2 == 0 ? -1 : */1;
//         imat4x4_t modelview = imat4x4affinemul(imat4x4translate(ivec3(IntToFixed(0),IntToFixed(0),IntToFixed(40))),imat4x4rotatex(-512));
//         modelview = imat4x4affinemul(modelview,imat4x4rotatey(rotdir*rotcnt*8));
//         modelview = imat4x4affinemul(modelview,imat4x4translate(ivec3(IntToFixed(0),IntToFixed(0),rotcnt<<4)));
        imat4x4_t modelview = imat4x4lookat(
            ivec3(IntToFixed(5), IntToFixed(-20), IntToFixed(-60)+(rotcnt<<8)),
            ivec3(IntToFixed(0), IntToFixed(5), IntToFixed(0)),
            ivec3(IntToFixed(0), IntToFixed(1), IntToFixed(0))
        );
	
	// Transform
	vertex_t transformVertex;
        srand(233);
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

                // Find same-height verts
                init_vertex_t origVertices[3];
                for(int ver = 0; ver < 3; ver++) {
                    origVertices[ver] = vertices[data.rasterizer.sortedTriangles[i].v[ver]];
                }
                
                if(origVertices[0].y - origVertices[1].y < 8) {
                    // top vertices are 01
                    tri.v[0].c = RastRGB(7,0,0);
                    tri.v[1].c = RastRGB(0,7,0);
                    tri.v[2].c = RastRGB(0,0,0);
                }
                else {
                    if(origVertices[0].y - origVertices[2].y < 8) {
                        // top vertices are 02
                        tri.v[0].c = RastRGB(7,0,0);
                        tri.v[2].c = RastRGB(0,7,0);
                        tri.v[1].c = RastRGB(0,0,0);
                    }
                    else {
                        // top vertices are 12
                        tri.v[1].c = RastRGB(7,0,0);
                        tri.v[2].c = RastRGB(0,7,0);
                        tri.v[0].c = RastRGB(0,0,0);
                    }
                }
                
//                 for(int ver = 0; ver < 3; ver++) {
//                         tri.v[ver].c = ivec3dot(
//                             tolight,
//                             normals[data.rasterizer.sortedTriangles[i].v[3]]
//                         );
//                         tri.v[ver].c = imin(imax(F(0), tri.v[ver].c)>>9,7);
//                         tri.v[ver].c = RastRGB(tri.v[ver].c,tri.v[ver].c,0);
//                 }
                
                RasterizeTriangle(image, &tri);
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
