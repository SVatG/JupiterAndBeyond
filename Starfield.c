#include "PixelParticles.h"
#include "VGA.h"
#include "Random.h"
#include "Utils.h"
#include "Global.h"

#include "Graphics/Bitmap.h"
#include "Graphics/RLEBitmap.h"
#include "Graphics/Drawing.h"

#include <string.h>

extern RLEBitmap JupiterCyborg;

void Starfield()
{
	uint8_t *framebuffer1=(uint8_t *)0x20000000;
	uint8_t *framebuffer2=(uint8_t *)0x20010000;
	memset(framebuffer1,RawRGB(0,0,0),320*200);
	memset(framebuffer2,RawRGB(0,0,0),320*200);

	SetVGAScreenMode320x200_60Hz(framebuffer1);

	for(int i=0;i<NumberOfStars;i++)
	{
		data.stars.stars[i].x=RandomInteger()%320-159-6000;
		data.stars.stars[i].y=RandomInteger()%200-99;
		data.stars.stars[i].z=3000-RandomInteger()%3000;
	}

	uint8_t palette[8]={
		RawRGB(7,7,3),RawRGB(7,7,3),RawRGB(7,7,3),RawRGB(7,7,3),
		RawRGB(6,6,3),RawRGB(6,6,3),RawRGB(5,5,3),RawRGB(5,5,3),
	};

	int t=0;
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

		uint32_t *sourceptr=(uint32_t *)&source[0];
		uint32_t *destinationptr=(uint32_t *)&destination[0];

		if(t&1)
		{
			for(int y=0;y<200;y++)
			for(int x=0;x<320/4;x++)
			{
				uint32_t pixels=*sourceptr++;
				uint32_t isnotzero=pixels&0x25252525;
				isnotzero|=(pixels&0x4a4a4a4a)>>1;
				isnotzero|=(pixels&0x90909090)>>2;

				*destinationptr++=pixels-isnotzero;
			}
		}
		else
		{
			for(int y=0;y<200;y++)
			for(int x=0;x<320/4;x++)
			{
				uint32_t pixels=*sourceptr++;
				uint32_t isnotzero=pixels&0x24242424;
				isnotzero|=(pixels&0x48484848)>>1;
				isnotzero|=(pixels&0x90909090)>>2;

				*destinationptr++=pixels-isnotzero;
			}
		}

		uint32_t line2=VGALine;

		Bitmap screen;
		InitializeBitmap(&screen,320,200,320,destination);

		for(int i=0;i<NumberOfStars;i++)
		{
			int ox=100*data.stars.stars[i].x/data.stars.stars[i].z+159;
			int oy=100*data.stars.stars[i].y/data.stars.stars[i].z+99;

			data.stars.stars[i].x+=80/3;
			data.stars.stars[i].y-=20/3;
			data.stars.stars[i].z-=30/3;

			int x=100*data.stars.stars[i].x/data.stars.stars[i].z+159;
			int y=100*data.stars.stars[i].y/data.stars.stars[i].z+99;

			DrawLine(&screen,ox,oy,x,y,palette[i&7]);

			if(x>=320 || y<0 || y>=200 || data.stars.stars[i].z<=40)
			{
				int z=RandomInteger()%2000+40;
				int y=RandomInteger()%2000;
				data.stars.stars[i].x=(0-159)*z/100;
				data.stars.stars[i].y=(y-990)*z/100/10;
				data.stars.stars[i].z=z;
			}
		}

		DrawRLEBitmap(&screen,&JupiterCyborg,0,0);

		uint32_t line3=VGALine;

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

