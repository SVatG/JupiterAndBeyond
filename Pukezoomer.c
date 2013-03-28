#include "Pukezoomer.h"
#include "VGA.h"
#include "Random.h"
#include "Utils.h"
#include "Global.h"

#include "Graphics/Bitmap.h"
#include "Graphics/Drawing.h"

#include <string.h>

static Pixel AddRed(Pixel a,Pixel b)
{
	if(a>=0xe0) return a;
	else return a+0x20;
}

static Pixel SubRed(Pixel a,Pixel b)
{
	if(a<0x20) return a;
	else return a-0x20;
}

static Pixel SubGreen(Pixel a,Pixel b)
{
    if(a<(1<<3)) return a;
    else return a-(1<<3);
}

static Pixel AddBlue(Pixel a,Pixel b)
{
    if(a>=(1<<3)) return a;
    else return a+1;
}

void Pukezoomer()
{
	uint8_t *framebuffer1=(uint8_t *)0x20000000;
	uint8_t *framebuffer2=(uint8_t *)0x20010000;
	memset(framebuffer1,255,320*200);
	memset(framebuffer2,255,320*200);

	SetVGAScreenMode320x200_60Hz(framebuffer1);

	uint32_t colour=(uint32_t)RawRGB(0x6,0x6,0x3)*0x01010101;

	int t=0;
        while(CurrentBitBinRow(songp) < 384)
	{
		WaitVBL();

		uint8_t *source,*destination;
		if(t&1)
		{
			source=framebuffer1;
			destination=framebuffer2;
			SetFrameBuffer(framebuffer1);
		}
		else
		{
			source=framebuffer2;
			destination=framebuffer1;
			SetFrameBuffer(framebuffer2);
		}

		//for(int i=0;i<100;i++) source[RandomInteger()%(320*200)]=RandomInteger();
		//if((RandomInteger()&63)==0) colour=(RandomInteger()&0xff)*0x01010101;

		uint8_t *destination2=destination;
		uint32_t *destination32=(uint32_t *)destination;

		#define Ratio 16

		int xskips[320/Ratio+1]={0};
		int yskips[200/Ratio+1]={0};

		for(int i=0;i<320/Ratio;i++) xskips[i]=i*Ratio+(t*11)%Ratio;//RandomInteger()%Ratio;
		for(int i=0;i<200/Ratio;i++) yskips[i]=i*Ratio+((t+6)*9)%Ratio;//RandomInteger()%Ratio;
		if(yskips[0]==0) yskips[0]=1;
		if(yskips[200/Ratio-1]==199) yskips[200/Ratio-1]=200;

		int xcenter=320/2;//+isin(t*20)/500;
		int ycenter=200/2;//+icos(t*20)/500;

		source[xcenter/Ratio*Ratio+ycenter/Ratio*Ratio*320]=0xff;

		Bitmap screen;
		InitializeBitmap(&screen,320,200,320,source);

		for(int i=0;i<6;i++)
		CompositeLine(&screen,
		xcenter/Ratio*Ratio+FixedToInt(2*icos(t*2*(i+1))),
		ycenter/Ratio*Ratio+FixedToInt(2*isin(t*2*(i+1))),
		xcenter/Ratio*Ratio+FixedToInt(100*icos(t*2*(i+1))),
		ycenter/Ratio*Ratio+FixedToInt(100*isin(t*2*(i+1))),
		0,AddBlue);

		for(int i=0;i<6;i++)
		CompositeLine(&screen,
		xcenter/Ratio*Ratio+FixedToInt(2*icos(t*(2*(i+1)+1))),
		ycenter/Ratio*Ratio+FixedToInt(2*isin(t*(2*(i+1)+1))),
		xcenter/Ratio*Ratio+FixedToInt(100*icos(t*(2*(i+1)+1))),
		ycenter/Ratio*Ratio+FixedToInt(100*isin(t*(2*(i+1)+1))),
		0,SubGreen);

		int sourcerow=ycenter/Ratio;

		int *yskipptr=yskips;
		for(int y=0;y<200;y++)
		{
			if(y==*yskipptr) { yskipptr++; destination+=320; continue; }

			uint8_t *sourceptr=&source[sourcerow*320+xcenter/Ratio];
			int *xskipptr=xskips;

			for(int x=0;x<320;x++)
			{
				if(x!=*xskipptr)
				{
					*destination++=*sourceptr++;
				}
				else
				{
					xskipptr++;

					uint8_t p1=sourceptr[-1]^0xffffffff;
					uint8_t p2=sourceptr[0]^0xffffffff;
					uint8_t halfp1=(p1>>1)&PixelAllButHighBits;
					uint8_t halfp2=(p2>>1)&PixelAllButHighBits;
                	uint8_t carry=p1&p2&PixelLowBits;
					uint32_t r=RandomInteger();
					r&=r>>16;
					r&=r>>8;
					r&=RandomInteger();
					r&=0xe0e0e0e0;
					r|=(r>>3)|((r>>6)&0x03030303);
					*destination++=((halfp1+halfp2+carry)|(r&colour))^0xffffffff;
				}
			}

			sourcerow++;
		}

		yskipptr=yskips;
		for(int y=0;y<200;y++)
		{
			if(y!=*yskipptr) { destination32+=320/4; continue; }
			yskipptr++;

			uint32_t *sourceptr1=(uint32_t *)&destination32[-1*320/4];
			uint32_t *sourceptr2=(uint32_t *)&destination32[1*320/4];

			for(int i=0;i<320/4;i++)
			{
				uint32_t p1=*sourceptr1++^0xffffffff;
				uint32_t p2=*sourceptr2++^0xffffffff;
				uint32_t halfp1=(p1>>1)&((uint32_t)PixelAllButHighBits*0x01010101);
				uint32_t halfp2=(p2>>1)&((uint32_t)PixelAllButHighBits*0x01010101);
				uint32_t carry=p1&p2&(PixelLowBits*0x01010101);
				uint32_t r=RandomInteger();
				r&=RandomInteger();
				r&=RandomInteger();
				r&=RandomInteger();
				r&=0xe0e0e0e0;
				r|=(r>>3)|((r>>6)&0x03030303);
				*destination32++=((halfp1+halfp2+carry)|(r&colour))^0xffffffff;
			}
		}

/*		for(int y=0;y<200;y++)
		for(int x=320/2-8;x<320/2+8;x++)
		{
			destination2[x+y*320]=0;
		}*/

		t++;
	}

	while(UserButtonState());
}

