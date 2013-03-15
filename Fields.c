#include "Fields.h"
#include "Utils.h"

#include "Graphics/Bitmap.h"

#include <stdint.h>
#include <string.h>

#define Width 212
#define Height 133

extern const int16_t rayarray[Width*Height*3];
static uint8_t palette[32],palette2[32],palette3[32],palette4[32];

void InitializeField()
{
	for(int i=0;i<32;i++)
	{
		palette[i]=RGB(
			i*i*255/(31*31),
			i*i*i*255/(31*31*31),
			i*255/31
		);

		palette2[i]=RGB(
			i*i*i*255/(31*31*31),
			i*255/31,
			i*i*255/(31*31)
		);

		palette3[i]=RGB(
			i*255/31,
			i*i*i*255/(31*31*31),
			i*i*255/(31*31)
		);

		palette4[i]=RGB(
			i*255/31,
			i*255/31,
			i*i*i*255/(31*31*31)
		);
	}
}

static inline int32_t approxabs(int32_t x) { return x^(x>>31); }

void DrawField(uint8_t *pixels,int t)
{
	const int16_t *rays=rayarray;

/*	switch(t&3)
	{
		case 1: pixels+=Width+1; break;
		case 2: pixels+=1; break;
		case 3: pixels+=Width; break;
	}*/

	int32_t x0=(0+Fix(0.5))<<20;
	int32_t y0=(isin(t*20)+Fix(0.5))<<20;
	int32_t z0=(t*150+Fix(0.5))<<20;

	int32_t sin_a=isin(t*9);
	int32_t cos_a=icos(t*9);

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

//				int32_t dist=(tx|ty^tz)-0x5000;
//				int32_t dist=(tx|ty&tz)-0x5000;
//				int32_t dist=(tx|ty+tz)-0x5000;
//				int32_t dist=(tx+ty)&tz-0x8000;
				int32_t dist=(tx+ty+(tz&0xf0f0))-0x8000;
//				int32_t dist=(tx+ty+(tz&0x005555))-0x8000;

				if(dist<0x200) break;

				x+=dx*dist;
				y+=dy*dist;
				z+=dz*dist;

				i--;
			}

			*pixels=palette[i];
			pixels++;
			//pixels+=2;
		}
		//pixels+=Width;
	}
}

void DrawField2(uint8_t *pixels,int t)
{
	const int16_t *rays=rayarray;

	int32_t x0=(-t*150+Fix(0.5))<<20;
	int32_t y0=(0+Fix(0.5))<<20;
	int32_t z0=(0+Fix(0.5))<<20;

	int32_t sin_a=isin(isin(t*2));
	int32_t cos_a=icos(isin(t*2));

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

			*pixels=palette2[i];
			pixels++;
		}
	}
}

void DrawField3(uint8_t *pixels,int t)
{
	const int16_t *rays=rayarray;

	int32_t x0=(0+Fix(0.5))<<20;
	int32_t y0=(0+Fix(0.5))<<20;
	int32_t z0=(t*150+Fix(0.5))<<20;

	int32_t sin_a=isin(t*17);
	int32_t cos_a=icos(t*17);

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

			*pixels=palette3[i];
			pixels++;
		}
	}
}

void DrawField4(uint8_t *pixels,int t)
{
	const int16_t *rays=rayarray;

	int32_t x0=(0+Fix(0.5))<<20;
	int32_t y0=(0+Fix(0.5))<<20;
	int32_t z0=(t*220+Fix(0.5))<<20;

	int32_t sin_a=isin(t*5);
	int32_t cos_a=icos(t*5);

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

			*pixels=palette4[i];
			pixels++;
		}
	}
}
