#include "Fields.h"
#include "VGA.h"
#include "Random.h"
#include "Utils.h"
#include "Global.h"
#include "LED.h"

#include "Graphics/Bitmap.h"

#include <stdint.h>
#include <string.h>

#define Width 212
#define Height 133

static void InitializeField();
static void DrawField(uint8_t *pixels,int t);
static void DrawField2(uint8_t *pixels,int t);
static void DrawField3(uint8_t *pixels,int t);
static void DrawField4(uint8_t *pixels,int t);

extern const int16_t rayarray[Width*Height*3];

#define intmin(x,y) (((x) < (y)) ? (x) : (y))

void Fields()
{
	InitializeField();

	uint8_t *framebuffer1=(uint8_t *)0x20000000;
	uint8_t *framebuffer2=(uint8_t *)0x20010000;
	memset(framebuffer1,0,212*133);
	memset(framebuffer2,0,212*133);

	SetVGAScreenMode212x133_60Hz(framebuffer1);

	int frame=0;
        while(CurrentBitBinRow(songp) < 1184)
	{
		int t=VGAFrameCounter();

		SetLEDs(t>>3);

		WaitVBL();

		uint8_t *framebuffer;
		if(frame)
		{
			framebuffer=framebuffer2;
			SetFrameBuffer(framebuffer1);
			frame=0;
		}
		else
		{
			framebuffer=framebuffer1;
			SetFrameBuffer(framebuffer2);
			frame=1;
		}

		switch((t>>8)&3)
		{
			case 0: DrawField(framebuffer,t); break;
			case 1: DrawField2(framebuffer,t); break;
			case 2: DrawField3(framebuffer,t); break;
			case 3: DrawField4(framebuffer,t); break;
		}
	}

	while(UserButtonState());
}

static void InitializeField()
{
	for(int i=0;i<32;i++)
	{
		data.fields.palette[i]=RGB(
			i*i*255/(31*31),
			i*i*i*255/(31*31*31),
			i*255/31
		);

		data.fields.palette2[i]=RGB(
			i*i*i*255/(31*31*31),
			i*255/31,
			i*i*255/(31*31)
		);

		data.fields.palette3[i]=RGB(
			i*255/31,
			i*i*i*255/(31*31*31),
			i*i*255/(31*31)
		);

		data.fields.palette4[i]=RGB(
			i*255/31,
			i*255/31,
			i*i*i*255/(31*31*31)
		);
	}
}

static inline int32_t approxabs(int32_t x) { return x^(x>>31); }

static void DrawField(uint8_t *pixels,int tv)
{
	const int16_t *rays=rayarray;

/*	switch(t&3)
	{
		case 1: pixels+=Width+1; break;
		case 2: pixels+=1; break;
		case 3: pixels+=Width; break;
	}*/
        
	int32_t x0=(0+Fix(0.5))<<20;
	int32_t y0=(isin(tv*20/4)+Fix(0.5))<<20;
	int32_t z0=(tv*150/4+Fix(0.5))<<20;

	int32_t sin_a=isin(tv*9/4);
	int32_t cos_a=icos(tv*9/4);
        
	for(int y=0;y<Height;y++)
	{
		for(int x=0;x<Width;x++)
		{
			int32_t dx=*rays++;
			int32_t dy=*rays++;
			int32_t dz=*rays++;

			int32_t t=(dx*cos_a+dz*sin_a)>>12;
			dz=(-dx*sin_a+dz*cos_a)>>12;
			dx=t;

			int32_t x=x0,y=y0,z=z0;

			int i=31;
			int32_t dist = 0;
			while(i)
			{
/*				int32_t tx=approxabs(x)>>15;
				int32_t ty=approxabs(y)>>15;
				int32_t tz=approxabs(z)>>15;

				int32_t dist=tx;
				if(ty>dist) dist=ty;
				if(tz>dist) dist=tz;

				dist-=0x8000;

				if(dist<0x800) break;*/

/*				int32_t tx=approxabs(x)>>16;
				int32_t ty=approxabs(y)>>16;
				int32_t tz=approxabs(z)>>16;

				int32_t dist=tx+ty+tz-0x8000;

				if(dist<0x200) break;*/

/*				int32_t tx=approxabs(x)>>15;
				int32_t ty=approxabs(y)>>15;
				int32_t tz=approxabs(z)>>15;

				int32_t dist1=tx;
				if(tz>dist1) dist1=tz;

				int32_t dist2=tx;
				if(ty>dist2) dist2=ty;

				int32_t dist=dist1;
				if(dist2<dist1) dist=dist2;

				dist-=0x4000;

				if(dist<0x400) break;*/

/*				int32_t tx=approxabs(x)>>15;
				int32_t ty=approxabs(y)>>15;
				int32_t tz=approxabs(z)>>15;

				int32_t dist1=tx;
				if(tz>dist1) dist1=tz;

				int32_t dist2=tx;
				if(ty>dist2) dist2=ty;

				int32_t dist3=ty;
				if(tz>dist3) dist3=tz;

				int32_t dist=dist1;
				if(dist2<dist) dist=dist2;
				if(dist3<dist) dist=dist3;

				dist-=0x4000;

				if(dist<0x400) break;*/

/*				int32_t tx=approxabs(x)>>16;
				int32_t ty=approxabs(y)>>16;
				int32_t tz=approxabs(z)>>16;

//				int32_t dist=approxabs(ty-tz)+tx-0x3000;
				int32_t dist=(tx|(ty+tz))-0x5000;

				if(dist<0x200) break;*/

/*				int32_t tx=approxabs(x)>>16;
				int32_t ty=approxabs(y)>>15;
				int32_t tz=approxabs(z)>>16;

				int32_t dist=(tx+(ty&0xc3c3c000)+(tz&0xF0F0F0F0))-0x8000;

				if(dist<0x200) break;*/

				int32_t tx=approxabs(x)>>16;
				int32_t ty=approxabs(y)>>16;
				int32_t tz=approxabs(z)>>16;
//                                 int32_t tx2=approxabs(x+(1<<31))>>16;
//                                 int32_t ty2=approxabs(y+(1<<31))>>16;
//                                 int32_t tz2=approxabs((tv<<28))>>18;
//                              tx2+=+(1<<12);
//                                 ty2 = ty2 + ((int32_t)tv<<6);
//                                 tx2 = tx2 + ((int32_t)tv);
//                              tz2+=+(1<<10);
                                
//				int32_t dist=(tx|ty^tz)-0x5000;
//				int32_t dist=(tx|ty&tz)-0x5000;
//				int32_t dist=(tx|ty+tz)-0x5000;
//				int32_t dist=(tx+ty)&tz-0x8000;
				dist=(tx+ty+(tz&0xf0f0))-0x8000;
//				int32_t dist=(tx+ty+(tz&0x005555))-0x8000;
//                                 int32_t dist2=(tz2|(tx2+ty2))-0x1000;
                                
//                                 dist = intmin(dist,dist2);
                                
				if(dist<0x200) break;

				x+=dx*dist;
				y+=dy*dist;
				z+=dz*dist;

				i--;
			}
// 			dist = approxabs(dist) >> 11;
			*pixels=data.fields.palette[i];
			pixels++;
			//pixels+=2;
		}
		//pixels+=Width;
	}
}

static void DrawField2(uint8_t *pixels,int t)
{
	const int16_t *rays=rayarray;

	int32_t x0=(-t*150/4+Fix(0.5))<<20;
	int32_t y0=(0+Fix(0.5))<<20;
	int32_t z0=(0+Fix(0.5))<<20;

	int32_t sin_a=isin(isin(t*2/4));
	int32_t cos_a=icos(isin(t*2/4));

	for(int y=0;y<Height;y++)
	{
		for(int x=0;x<Width;x++)
		{
			int32_t dy=*rays++;
			int32_t dz=*rays++;
			int32_t dx=*rays++;

			int32_t t=(dy*cos_a+dz*sin_a)>>12;
			dz=(-dy*sin_a+dz*cos_a)>>12;
			dy=t;

			int32_t x=x0,y=y0,z=z0;

			int i=31;
			while(i)
			{
				int32_t tx=approxabs(x)>>16;
				int32_t ty=approxabs(y)>>16;
				int32_t tz=approxabs(z)>>16;

				int32_t dist=(tx|(ty+tz))-0x5000;

				if(dist<0x200) break;

				x+=dx*dist;
				y+=dy*dist;
				z+=dz*dist;

				i--;
			}

			*pixels=data.fields.palette2[i];
			pixels++;
		}
	}
}

static void DrawField3(uint8_t *pixels,int t)
{
	const int16_t *rays=rayarray;

	int32_t x0=(0+Fix(0.5))<<20;
	int32_t y0=(0+Fix(0.5))<<20;
	int32_t z0=(t*150/4+Fix(0.5))<<20;

	int32_t sin_a=isin(t*17/4);
	int32_t cos_a=icos(t*17/4);

	for(int y=0;y<Height;y++)
	{
		for(int x=0;x<Width;x++)
		{
			int32_t dx=*rays++;
			int32_t dy=*rays++;
			int32_t dz=*rays++;

			int32_t t=(dx*cos_a+dz*sin_a)>>12;
			dz=(-dx*sin_a+dz*cos_a)>>12;
			dx=t;

			int32_t x=x0,y=y0,z=z0;

			int i=31;
			while(i)
			{
				int32_t tx=approxabs(x)>>16;
				int32_t ty=approxabs(y)>>15;
				int32_t tz=approxabs(z)>>16;

				int32_t dist=(tx+(ty&0xc3c3c000)+(tz&0xF0F0F0F0))-0x8000;

				if(dist<0x200) break;

				x+=dx*dist;
				y+=dy*dist;
				z+=dz*dist;

				i--;
			}

			*pixels=data.fields.palette3[i];
			pixels++;
		}
	}
}

static void DrawField4(uint8_t *pixels,int t)
{
	const int16_t *rays=rayarray;

	int32_t x0=(0+Fix(0.5))<<20;
	int32_t y0=(0+Fix(0.5))<<20;
	int32_t z0=(t*220/4+Fix(0.5))<<20;

	int32_t sin_a=isin(t*5/4);
	int32_t cos_a=icos(t*5/4);

	for(int y=0;y<Height;y++)
	{
		for(int x=0;x<Width;x++)
		{
			int32_t dx=*rays++;
			int32_t dy=*rays++;
			int32_t dz=*rays++;

			int32_t t=(dy*cos_a+dz*sin_a)>>12;
			dz=(-dy*sin_a+dz*cos_a)>>12;
			dy=t;

			int32_t x=x0,y=y0,z=z0;

			int i=31;
			while(i)
			{
				int32_t tx=approxabs(x)>>16;
				int32_t ty=approxabs(y)>>16;
				int32_t tz=approxabs(z)>>16;

				int32_t dist=approxabs(tx-tz)+ty-0x3000;
//				int32_t dist=(tx|(ty+tz))-0x5000;

				if(dist<0x200) break;

				x+=dx*dist;
				y+=dy*dist;
				z+=dz*dist;

				i--;
			}

			*pixels=data.fields.palette4[i];
			pixels++;
		}
	}
}

