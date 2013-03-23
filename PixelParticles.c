#include "PixelParticles.h"
#include "VGA.h"
#include "Random.h"
#include "Utils.h"
#include "Global.h"

#include "Graphics/Bitmap.h"
#include "Graphics/Drawing.h"

#include <string.h>

static inline uint16_t PixelSum(uint16_t a,uint16_t b)
{
	uint16_t halfa=(a>>1)&(PixelAllButHighBits*0x0101);
	uint16_t halfb=(b>>1)&(PixelAllButHighBits*0x0101);
	uint16_t carry=a&b&(PixelLowBits*0x0101);
	return halfa+halfb+carry;
}

extern uint8_t BlendTable[256][256];

void PixelParticles()
{
	uint8_t *framebuffer1=(uint8_t *)0x20000000;
	uint8_t *framebuffer2=(uint8_t *)0x20010000;
	memset(framebuffer1,0,320*200);
	memset(framebuffer2,0,320*200);

	SetVGAScreenMode320x200_60Hz(framebuffer1);

	for(int i=0;i<NumberOfPixelParticles;i++)
	{
		data.pp.particles[i].x=RandomInteger()%Fix(320);
		data.pp.particles[i].y=RandomInteger()%Fix(200);
	}

	int t=0;
	int32_t tx1=0,ty1=0;
	int32_t tx2=0,ty2=0;
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

		for(int y=1;y<199;y++)
		for(int x=1;x<319;x++)
		{
			int offs=x+y*320;
			uint8_t sum1=BlendTable[source[offs-1]][source[offs+1]];
			uint8_t sum2=BlendTable[source[offs-320]][source[offs+320]];
			uint8_t sum3=BlendTable[sum1][sum2];
			uint8_t sum4=BlendTable[sum3][source[offs]];
//sum4+=(sum3>>2)&PixelLowBits;
			destination[offs]=sum4;
		}

		Bitmap screen;
		InitializeBitmap(&screen,320,200,320,destination);

		int x0=FixedToInt(50*icos(t*30*2))+FixedToInt(50*icos(t*23*2))+159;
		int y0=FixedToInt(50*isin(t*30*2))+FixedToInt(50*isin(t*23*2))+99;

		tx1+=icos(t*3);
		ty1+=isin(t*3);
		tx2+=icos(-t*2);
		ty2+=isin(-t*2);

		for(int i=0;i<NumberOfPixelParticles;i++)
		{
			int32_t x=data.pp.particles[i].x;
			int32_t y=data.pp.particles[i].y;
			int newx=x+isin(y/32+tx1/32);
			int newy=y-icos(x/32+ty1/32);
			//newx+=isin(y/96+tx2/96);
			//newy-=icos(x/96+ty2/96);
			if(newx<0 || newx>=Fix(320) || newy<0 || newy>=Fix(200))
			{
				newx=RandomInteger()%Fix(320);
				newy=RandomInteger()%Fix(200);
			}
			data.pp.particles[i].x=newx;
			data.pp.particles[i].y=newy;
			DrawPixelNoClip(&screen,FixedToInt(newx),FixedToInt(newy),RawRGB(7,6,3));
		}

		t++;
	}

	while(UserButtonState());
}

