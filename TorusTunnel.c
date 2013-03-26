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

static inline uint32_t PixelAverage(uint32_t a,uint32_t b)
{
	uint32_t halfa=(a>>1)&(PixelAllButHighBits*0x01010101);
	uint32_t halfb=(b>>1)&(PixelAllButHighBits*0x01010101);
	uint32_t carry=a&b&(PixelLowBits*0x01010101);
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

	for(int i=0;i<NumberOfPixelParticles;i++)
	{
		data.pp.particles[i].x=RandomInteger()%Fix(320);
		data.pp.particles[i].y=RandomInteger()%Fix(200);
	}

/*	Bitmap screen;
	InitializeBitmap(&screen,320,200,320,framebuffer1);
	DrawHorizontalLine(&screen,0,0,320,0xff);
	DrawHorizontalLine(&screen,0,199,320,0xff);
	InitializeBitmap(&screen,320,200,320,framebuffer2);
	DrawHorizontalLine(&screen,0,0,320,0xff);
	DrawHorizontalLine(&screen,0,199,320,0xff);*/

	uint32_t map[64];
	for(int i=0;i<64;i++) map[i]=RandomInteger();

	int t=0;
	int32_t tx1=0,ty1=0;
	int32_t tx2=0,ty2=0;
	int32_t tx3=0,ty3=0;
	int32_t s1=Fix(0.5);
	int32_t s2=Fix(1);
	int32_t s3=Fix(2);
	while(!UserButtonState())
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
		for(int i=0;i<64;i++)
		{
			int step=a0/(4096/128);
			int a1=4096*i/128-a0%(4096/128);
			int a2=4096*(i-1)/128-a0%(4096/128);
			int32_t sin_a1=isin(a1);
			int32_t cos_a1=icos(a1);
			int32_t sin_a2=isin(a2);
			int32_t cos_a2=icos(a2);
			for(int j=0;j<48;j++)
			{
				if((i^j^step)&1) continue;

				int b1=4096*j/48;
				int b2=4096*(j+1)/48;
				int32_t sin_b1=isin(b1);
				int32_t cos_b1=icos(b1);
				int32_t sin_b2=isin(b2);
				int32_t cos_b2=icos(b2);

				int x1=imul(Fix(10),cos_b1);
				int y1=imul(Fix(10),sin_b1)-a1*140;
				int z1=a1*100;
				int x2=imul(Fix(10),cos_b2);
				int y2=imul(Fix(10),sin_b2)-a1*140;
				int z2=a1*100;
				int x3=imul(Fix(10),cos_b1);
				int y3=imul(Fix(10),sin_b1)-a2*140;
				int z3=a2*100;
				int x4=imul(Fix(10),cos_b2);
				int y4=imul(Fix(10),sin_b2)-a2*140;
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
					uint32_t val=Hash32(i+step+j*1000);
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

		DrawRLEBitmap(&screen,&JupiterCyborg2,0,0);

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
