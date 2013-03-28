#include "TorusTunnel.h"
#include "VGA.h"
#include "Random.h"
#include "Utils.h"
#include "Global.h"

#include "Graphics/Bitmap.h"
#include "Graphics/RLEBitmap.h"
#include "Graphics/Drawing.h"

#include <string.h>

extern RLEBitmap JupiterCyborg2;
extern Bitmap JupiterCyborg2Texture;

typedef struct {
	ivec3_t p;
	int32_t uw;
	int32_t vw;
} t_vertex_t;

static void DrawTriangleAtAngle(Bitmap *screen,int angle);
inline static void RasterizeTriangle(uint8_t *image,t_vertex_t v1,t_vertex_t v2,t_vertex_t v3);

static inline uint32_t PixelAverage(uint32_t a,uint32_t b)
{
	uint32_t halfa=(a>>1)&(PixelAllButHighBits*0x01010101);
	uint32_t halfb=(b>>1)&(PixelAllButHighBits*0x01010101);
	uint32_t carry=a&b&(PixelLowBits*0x01010101);
//	uint32_t carry=a&b&(0x21*0x01010101);
	return halfa+halfb+carry;
}

static uint32_t Hash32(uint32_t val)
{
	val^=val>>16;
	val^=61;
	val+=val<<3;
	val^=val>>4;
	val*=0x27d4eb2d;
	val^=val>>15;
	return val;
}


void TorusTunnel()
{
	uint8_t *framebuffer1=(uint8_t *)0x20000000;
	uint8_t *framebuffer2=(uint8_t *)0x20010000;
	memset(framebuffer1,RawRGB(0,0,0),320*200);
	memset(framebuffer2,RawRGB(0,0,0),320*200);

	SetVGAScreenMode320x200_60Hz(framebuffer1);

	uint32_t map[64];
	for(int i=0;i<64;i++) map[i]=RandomInteger();

	int t=0;
    int rotate = 0; // total amount of rotation
	int32_t tx1=0,ty1=0;
	int32_t tx2=0,ty2=0;
	int32_t tx3=0,ty3=0;
	int32_t s1=Fix(0.5);
	int32_t s2=Fix(1);
	int32_t s3=Fix(2);
        while(CurrentBitBinRow(songp) < 800)
	{
		WaitVBL();

		uint8_t *source,*destination;
		if(t&1)
		{
			source=framebuffer1;
			destination=framebuffer2;
		}
		else
		{
			source=framebuffer2;
			destination=framebuffer1;
		}
		SetFrameBuffer(source);

		uint32_t line1=VGALine;

		uint32_t *sourceptr=(uint32_t *)&source[320];
		uint32_t *destinationptr=(uint32_t *)&destination[320];
		uint32_t previous=RawRGB(0,0,0)*0x01010101;
		uint32_t current=*sourceptr++;
		for(int y=1;y<199;y++)
		{
			for(int x=0;x<320/4;x++)
			{
				uint32_t sum1=PixelAverage(sourceptr[-320/4-1],sourceptr[320/4-1]);
				uint32_t next=*sourceptr++;
				uint32_t sum2=PixelAverage((current<<8)|(previous>>24),(current>>8)|(next<<24));
				uint32_t sum3=PixelAverage(sum1,sum2);
				uint32_t sum4=PixelAverage(sum3,current);
				*destinationptr++=sum4;
				previous=current;
				current=next;
			}
		}

		uint32_t line2=VGALine;

		Bitmap screen;
		InitializeBitmap(&screen,320,198,320,&destination[320]);

		map[RandomInteger()&63]^=RandomInteger();

		int a0=t*6;
        if(CurrentBitBinRow(songp)>714)
        {
            rotate += 6;
        }
		for(int i=0;i<64;i++)
		{
			int step=a0/(4096/128);
			int a1=4096*i/128-a0%(4096/128);
			int a2=4096*(i-1)/128-a0%(4096/128);
			for(int j=0;j<48;j++)
			{
				if((i^j^step)&1) continue;

				int b1=4096*j/48+4096/96;
				int b2=4096*(j+1)/48+4096/96;
                // make rotate later --ryx
                b1 += rotate;
                b2 += rotate;
				int32_t sin_b1=isin(b1);
				int32_t cos_b1=icos(b1);
				int32_t sin_b2=isin(b2);
				int32_t cos_b2=icos(b2);

				int x1=10*cos_b1;
				int y1=10*sin_b1-a1*140;
				int z1=a1*100;
				int x2=10*cos_b2;
				int y2=10*sin_b2-a1*140;
				int z2=a1*100;
				int x3=10*cos_b1;
				int y3=10*sin_b1-a2*140;
				int z3=a2*100;
				int x4=10*cos_b2;
				int y4=10*sin_b2-a2*140;
				int z4=a2*100;

				/*int x1=imul(Fix(10),cos_b1);
				int y1=imul(imul(Fix(10),sin_b1)+Fix(10),cos_a)-Fix(8);
				int z1=imul(imul(Fix(10),sin_b1)+Fix(10),sin_a)+Fix(4);
				int x2=imul(Fix(10),cos_b2);
				int y2=imul(imul(Fix(10),sin_b2)+Fix(10),cos_a)-Fix(8);
				int z2=imul(imul(Fix(10),sin_b2)+Fix(10),sin_a)+Fix(4);*/

				int sx1=50*x1/z1+159;
				int sy1=50*y1/z1+99;
				int sx2=50*x2/z2+159;
				int sy2=50*y2/z2+99;
				int sx3=50*x3/z3+159;
				int sy3=50*y3/z3+99;
				int sx4=50*x4/z4+159;
				int sy4=50*y4/z4+99;

				if(z1>0 && z2>0 && z3>0 && z4>0)
				{
					uint32_t val=Hash32(i+step+j*1024);
					uint32_t modifier=map[(val>>16)&63]>>((val>>24)&31);
					if((val^modifier)&1)
					{
						DrawLine(&screen,sx1,sy1,sx2,sy2,RawRGB(7,7,3));
						DrawLine(&screen,sx3,sy3,sx4,sy4,RawRGB(7,7,3));
					}
					else
					{
						DrawLine(&screen,sx1,sy1,sx3,sy3,RawRGB(7,7,3));
						DrawLine(&screen,sx2,sy2,sx4,sy4,RawRGB(7,7,3));
					}
				}
			}
		}

		uint32_t line3=VGALine;

/*		if(t&32)
		DrawRLEBitmap(&screen,&JupiterCyborg2,0,0);
		else
		RasterizeTriangle(screen.pixels,
		(t_vertex_t){ .p=ivec3(Fix(160),Fix(13),0), .uw=Fix(160.5-32), .vw=Fix(13.5-9) },
		(t_vertex_t){ .p=ivec3(Fix(29),Fix(189),0), .uw=Fix(29.5-32), .vw=Fix(189.5-9) },
		(t_vertex_t){ .p=ivec3(Fix(291),Fix(189),0), .uw=Fix(291.5-32), .vw=Fix(189.5-9) });*/

		int angle=t*21;
		angle&=1023;

		if(angle>512)
		{
			DrawTriangleAtAngle(&screen,angle);
			DrawTriangleAtAngle(&screen,angle-1024);
		}
		else
		{
			DrawTriangleAtAngle(&screen,angle-1024);
			DrawTriangleAtAngle(&screen,angle);
		}


/*int diff1=line2-line1;
if(diff1<0) diff1+=480;
int diff2=line3-line2;
if(diff2<0) diff2+=480;

DrawHorizontalLine(&screen,0,0,diff1,RGB(0,255,0));
DrawHorizontalLine(&screen,0,199,diff2,RGB(0,255,0));*/

		t++;
	}

	while(UserButtonState());
}

static void DrawTriangleAtAngle(Bitmap *screen,int angle)
{
	int32_t centerx=Fix(160);
	int32_t centery=Fix(120);

	int32_t project=200;
	int32_t distance=Fix(4000);
	int32_t sx=Fix(291)-centerx;
	int32_t sy=Fix(189)-centery;

	int32_t x=idiv(imul(distance,sx),Fix(project)+sx);
	int32_t y=imul(sy,distance-x)/project;
	int32_t z=x;

	int32_t x1=imul(icos(angle),-x)+imul(isin(angle),z);
	int32_t z1=-imul(isin(angle),-x)+imul(icos(angle),z);
	int32_t x2=imul(icos(angle),x)+imul(isin(angle),z);
	int32_t z2=-imul(isin(angle),x)+imul(icos(angle),z);

	int32_t px1=idiv(project*x1,distance-z1)+centerx;
	int32_t py1=idiv(project*y,distance-z1)+centery;
	int32_t px2=idiv(project*x2,distance-z2)+centerx;
	int32_t py2=idiv(project*y,distance-z2)+centery;

	RasterizeTriangle(screen->pixels,
	(t_vertex_t){ .p=ivec3(Fix(160),Fix(13),0), .uw=Fix(160.5-32), .vw=Fix(13.5-9) },
	(t_vertex_t){ .p=ivec3(px1,py1,0), .uw=Fix(29.5-32), .vw=Fix(189.5-9) },
	(t_vertex_t){ .p=ivec3(px2,py2,0), .uw=Fix(291.5-32), .vw=Fix(189.5-9) });
}



#define WIDTH 320
#define HEIGHT 199

inline static void RasterizeTriangle(uint8_t *image,t_vertex_t v1,t_vertex_t v2,t_vertex_t v3)
{
     uint8_t *shadetex=JupiterCyborg2Texture.pixels;
        
	// Vertex sorting
	t_vertex_t upperVertex;
	t_vertex_t centerVertex;
	t_vertex_t lowerVertex;

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
