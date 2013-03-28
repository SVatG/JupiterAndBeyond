#include "Environment.h"
#include "VGA.h"
#include "Random.h"
#include "Utils.h"
#include "Global.h"

#include "Graphics/Bitmap.h"
#include "Graphics/RLEBitmap.h"
#include "Graphics/Drawing.h"

#include "VectorLibrary/MatrixFixed.h"

#include <string.h>

extern Bitmap JupiterSVATG;
extern Bitmap JupiterSVATGTexture;

typedef struct {
	ivec2_t p;
	int32_t uw;
	int32_t vw;
} e_vertex_t;

inline static void RasterizeTriangle(uint8_t *image,e_vertex_t v1,e_vertex_t v2,e_vertex_t v3);

void Environment()
{
	uint8_t *framebuffer1=(uint8_t *)0x20000000;
	uint8_t *framebuffer2=(uint8_t *)0x20010000;
	memset(framebuffer1,RawRGB(0,0,0),320*200);
	memset(framebuffer2,RawRGB(0,0,0),320*200);

	SetVGAScreenMode320x200_60Hz(framebuffer1);

	int32_t ypos=Fix(10);
	int32_t yspeed=0;

	int frame=0;
        while(CurrentBitBinRow(songp) < 672)
	{
		WaitVBL();

		uint8_t *source,*destination;
		if(frame&1)
		{
			source=framebuffer1;
			destination=framebuffer2;
		}
		else
		{
			source=framebuffer2;
			destination=framebuffer1;
		}
		frame^=1;

		int t=VGAFrameCounter()-180;

		SetFrameBuffer(source);

		memcpy(destination,JupiterSVATG.pixels,320*200);

		Bitmap screen;
		InitializeBitmap(&screen,320,200,320,destination);

		imat3x3_t m=imat3x3mul(
			imat3x3rotatex(t*4),
			imat3x3rotatey(t*7)
		);

		imat3x3_t mn=m;

		int32_t ytarget;

		if(t<0) ytarget=Fix(10);
		else ytarget=Fix(0);

		int32_t yacc=(ytarget-ypos)/16;
		if(t>240) yacc=Fix(0.1);
		yspeed+=yacc;
		yspeed-=yspeed/8;
		ypos+=yspeed;

		for(int i=0;i<NumberOfSegments;i++)
		{
			int a=2048*i/(NumberOfSegments-1);
			int32_t sin_a=isin(a);
			int32_t cos_a=icos(a);
			int32_t R=imul(sin_a,isin(3*a+t*16))+Fix(3);
			int32_t dRda=imul(cos_a,isin(3*a+t*16))+imul(sin_a,3*icos(3*a+t*16));

			int32_t r=imul(sin_a,R);
			int32_t z=imul(cos_a,R);
			int32_t nr=r-imul(cos_a,dRda); // dz/da
			int32_t nz=z+imul(sin_a,dRda); // dr/da
			int32_t n=inorm(nr,nz);
			nr=idiv(nr,n);
			nz=idiv(nz,n);

			for(int j=0;j<NumberOfSectors;j++)
			{
				int b=4096*j/NumberOfSectors;
				int32_t sin_b=isin(b);
				int32_t cos_b=icos(b);

				ivec3_t p=ivec3(imul(r,cos_b),imul(r,sin_b),z);
				ivec3_t n=ivec3(imul(nr,cos_b),imul(nr,sin_b),nz);

				p=imat3x3transform(m,p);
				n=imat3x3transform(mn,n);

				data.env.p[i][j].x=130*idiv(p.x,p.z+Fix(7))+Fix(159);
				data.env.p[i][j].y=130*idiv(p.y+ypos,p.z+Fix(7))+Fix(99);
				data.env.t[i][j]=ivec2(128*n.x+Fix(128),128*n.y+Fix(128));
			}
		}

		if(imat3x3_z(m).z>0)
		{
			for(int i=0;i<NumberOfSegments-1;i++)
			{
				for(int j=0;j<NumberOfSectors;j++)
				{
					int j2=(j+1)%NumberOfSectors;
					RasterizeTriangle(destination,
					(e_vertex_t){ .p=data.env.p[i][j], .uw=data.env.t[i][j].x, .vw=data.env.t[i][j].y },
					(e_vertex_t){ .p=data.env.p[i+1][j], .uw=data.env.t[i+1][j].x, .vw=data.env.t[i+1][j].y },
					(e_vertex_t){ .p=data.env.p[i+1][j2], .uw=data.env.t[i+1][j2].x, .vw=data.env.t[i+1][j2].y });
					RasterizeTriangle(destination,
					(e_vertex_t){ .p=data.env.p[i][j], .uw=data.env.t[i][j].x, .vw=data.env.t[i][j].y },
					(e_vertex_t){ .p=data.env.p[i+1][j2], .uw=data.env.t[i+1][j2].x, .vw=data.env.t[i+1][j2].y },
					(e_vertex_t){ .p=data.env.p[i][j2], .uw=data.env.t[i][j2].x, .vw=data.env.t[i][j2].y });				
				}
			}
		}
		else
		{
			for(int i=NumberOfSegments-2;i>=0;i--)
			{
				for(int j=0;j<NumberOfSectors;j++)
				{
					int j2=(j+1)%NumberOfSectors;
					RasterizeTriangle(destination,
					(e_vertex_t){ .p=data.env.p[i][j], .uw=data.env.t[i][j].x, .vw=data.env.t[i][j].y },
					(e_vertex_t){ .p=data.env.p[i+1][j], .uw=data.env.t[i+1][j].x, .vw=data.env.t[i+1][j].y },
					(e_vertex_t){ .p=data.env.p[i+1][j2], .uw=data.env.t[i+1][j2].x, .vw=data.env.t[i+1][j2].y });
					RasterizeTriangle(destination,
					(e_vertex_t){ .p=data.env.p[i][j], .uw=data.env.t[i][j].x, .vw=data.env.t[i][j].y },
					(e_vertex_t){ .p=data.env.p[i+1][j2], .uw=data.env.t[i+1][j2].x, .vw=data.env.t[i+1][j2].y },
					(e_vertex_t){ .p=data.env.p[i][j2], .uw=data.env.t[i][j2].x, .vw=data.env.t[i][j2].y });				
				}
			}
		}
	}

	while(UserButtonState());
}


#define WIDTH 320
#define HEIGHT 200

inline static void RasterizeTriangle(uint8_t *image,e_vertex_t v1,e_vertex_t v2,e_vertex_t v3)
{
	if(imul(v2.p.x-v1.p.x,v3.p.y-v1.p.y)>imul(v3.p.x-v1.p.x,v2.p.y-v1.p.y)) return;

     uint8_t *shadetex=JupiterSVATGTexture.pixels;

	// Vertex sorting
	e_vertex_t upperVertex;
	e_vertex_t centerVertex;
	e_vertex_t lowerVertex;

	if(v1.p.y < v2.p.y) {
		upperVertex = v1;
		lowerVertex = v2;
	}
	else {
		upperVertex = v2;
		lowerVertex = v1;
	}

	if(v3.p.y < upperVertex.p.y) {
		centerVertex = upperVertex;
		upperVertex = v3;
	}
	else {
		if(v3.p.y > lowerVertex.p.y) {
			centerVertex = lowerVertex;
			lowerVertex = v3;
		}
		else {
			centerVertex = v3;
		}
	}

	if(upperVertex.p.y>=Fix(200)) return;
	if(lowerVertex.p.y<=Fix(0)) return;

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
	int32_t U;
	int32_t V;
	int32_t UdX;
	int32_t VdX;

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
	UdX = idiv(imul(temp, lowerVertex.uw-upperVertex.uw) + upperVertex.uw-centerVertex.uw,width);
        VdX = idiv(imul(temp, lowerVertex.vw-upperVertex.vw) + upperVertex.vw-centerVertex.vw,width);
	
	// guard against special case B: flat upper edge
	if(upperDiff == 0 ) {

		if(upperVertex.p.x < centerVertex.p.x) {
			leftX = upperVertex.p.x;
            leftU = upperVertex.uw;
            leftV = upperVertex.vw;
			rightX = centerVertex.p.x;

			leftXd = idiv(upperVertex.p.x - lowerVertex.p.x, lowerDiff);
			rightXd = idiv(centerVertex.p.x - lowerVertex.p.x, lowerDiff);
		}
		else {
			leftX = centerVertex.p.x;
			leftU = centerVertex.uw;
            leftV = centerVertex.vw;
			rightX = upperVertex.p.x;

			leftXd = idiv(centerVertex.p.x - lowerVertex.p.x, lowerDiff);
			rightXd = idiv(upperVertex.p.x - lowerVertex.p.x, lowerDiff);
		}

		leftUd = idiv(leftU - lowerVertex.uw, lowerDiff);
                leftVd = idiv(leftV - lowerVertex.vw, lowerDiff);

		goto lower_half_render;
	}

	// calculate deltas
	upperCenter = idiv(upperVertex.p.x - centerVertex.p.x, upperDiff);
	upperLower = idiv(upperVertex.p.x - lowerVertex.p.x, lowerDiff);

	// upper triangle half
	leftX = rightX = upperVertex.p.x;

	leftU = upperVertex.uw;
	leftV = upperVertex.vw;

	if(upperCenter < upperLower) {
		leftXd = upperCenter;
		rightXd = upperLower;

		leftUd = idiv(leftU - centerVertex.uw, upperDiff);
		leftVd = idiv(leftV - centerVertex.vw, upperDiff);
	}
	else {
		leftXd = upperLower;
		rightXd = upperCenter;

		leftUd = idiv(leftU - lowerVertex.uw, lowerDiff);
		leftVd = idiv(leftV - lowerVertex.vw, lowerDiff);
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
                            int32_t hb = U>>12&0xff;
                            int32_t vb = (V>>4)&0xff00;
                            image[x+offset] = shadetex[hb|vb];
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

		leftU = centerVertex.uw;
		leftV = centerVertex.vw;

		leftUd = idiv(leftU - lowerVertex.uw, centerDiff);
		leftVd = idiv(leftV - lowerVertex.vw, centerDiff);
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
                            int32_t hb = U>>12&0xff;
                            int32_t vb = (V>>4)&0xff00;
                            image[x+offset] = shadetex[hb|vb];
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
