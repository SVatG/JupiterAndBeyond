#include "PixelParticles.h"
#include "VGA.h"
#include "Random.h"
#include "Utils.h"
#include "Global.h"

#include "Profiling.h"

#include "Graphics/Bitmap.h"
#include "Graphics/Drawing.h"

#include <string.h>

static inline uint32_t PixelAverage(uint32_t a,uint32_t b)
{
	uint32_t halfa=(a>>1)&(PixelAllButHighBits*0x01010101);
	uint32_t halfb=(b>>1)&(PixelAllButHighBits*0x01010101);
	uint32_t carry=a&b&(PixelLowBits*0x01010101);
	return halfa+halfb+carry;
}

void PixelParticles()
{
	uint8_t *framebuffer1=(uint8_t *)0x20000000;
	uint8_t *framebuffer2=(uint8_t *)0x20010000;
// 	memset(framebuffer1,RawRGB(0,0,0),320*200);
// 	memset(framebuffer2,RawRGB(0,0,0),320*200);

	SetVGAScreenMode320x200_60Hz(framebuffer1);

	for(int i=0;i<NumberOfPixelParticles;i++)
	{
		data.pp.particles[i].x=RandomInteger()%Fix(320);
		data.pp.particles[i].y=RandomInteger()%Fix(200);
	}

	Bitmap screen;
	InitializeBitmap(&screen,320,200,320,framebuffer1);
	DrawHorizontalLine(&screen,0,0,320,0xff);
	DrawHorizontalLine(&screen,0,199,320,0xff);
	InitializeBitmap(&screen,320,200,320,framebuffer2);
	DrawHorizontalLine(&screen,0,0,320,0xff);
	DrawHorizontalLine(&screen,0,199,320,0xff);

	int t=0;
	int32_t tx1=0,ty1=0;
	int32_t tx2=0,ty2=0;
	int32_t tx3=0,ty3=0;
	int32_t s1=Fix(0.5);
	int32_t s2=Fix(1);
	int32_t s3=Fix(2);
        while(CurrentBitBinRow(songp) < 1312)
	{
		WaitVBL();
//        profiling_startframe();

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
		InitializeBitmap(&screen,320,200,320,destination);

		tx1+=icos(t*3);
		ty1+=isin(t*3);
		tx2+=icos(-t*2);
		ty2+=isin(-t*2);
		tx3+=icos(-t*2);
		ty3+=isin(-t*2);

		for(int i=0;i<NumberOfPixelParticles;i++)
		{
			int32_t x=data.pp.particles[i].x;
			int32_t y=data.pp.particles[i].y;
			int32_t newx=x;
			int32_t newy=y;
			newx+=isin((y+ty1)>>6)>>1;
			newx-=icos((x+tx1)>>6)>>1;
			newx+=isin((y+ty2)>>7);
			newy-=icos((x+tx2)>>7);
			newx+=isin((y+ty3)>>8)<<1;
			newy-=icos((x+tx3)>>8)<<1;
			if(newx<0 || newx>=Fix(320) || newy<0 || newy>=Fix(200))
			{
				newx=RandomInteger()%Fix(320);
				newy=RandomInteger()%Fix(200);
			}
			data.pp.particles[i].x=newx;
			data.pp.particles[i].y=newy;
			DrawPixelNoClip(&screen,FixedToInt(newx),FixedToInt(newy),RawRGB(7,6,3));
		}
		uint32_t line3=VGALine;

/*int diff1=line2-line1;
if(diff1<0) diff1+=480;
int diff2=line3-line2;
if(diff2<0) diff2+=480;

DrawHorizontalLine(&screen,0,0,diff1,RGB(0,255,0));
DrawHorizontalLine(&screen,0,199,diff2,RGB(0,255,0));*/

		t++;
//        profiling_endframe(destination+(320*199));
	}

	while(UserButtonState());
}

