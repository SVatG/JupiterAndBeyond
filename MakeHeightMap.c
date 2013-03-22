#include <stdio.h>
#include <stdint.h>
#include <limits.h>
#include <math.h>

#include "Utils.h"

#define Width 256
#define Height 256

static int64_t map[Width*Height];

static uint32_t s1=0xc7ff5f16,s2=0x0dc556ae,s3=0x78010089;

void SeedRandom(uint32_t seed)
{
	s1=seed*1664525+1013904223|0x10;
	s2=seed*1103515245+12345|0x1000;
	s3=seed*214013+2531011|0x100000;
}

uint32_t RandomInteger()
{
	s1=((s1&0xfffffffe)<<12)^(((s1<<13)^s1)>>19);
	s2=((s2&0xfffffff8)<<4)^(((s2<<2)^s2)>>25);
	s3=((s3&0xfffffff0)<<17)^(((s3<<3)^s3)>>11);
	return s1^s2^s3;
}

int main()
{
	for(int y=0;y<Height;y++)
	for(int x=0;x<Width;x++)
	{
		int32_t fx=Fix(x)/(Width);
		int32_t fy=Fix(y)/(Height);

		//int h=(isin(fx)+isin(fy*4)/2+Fix(2))>>6;

		int64_t h=0;
		SeedRandom(1);
		for(int u=1;u<5;u++)
		for(int v=1;v<5;v++)
		{
			h+=(int64_t)isin(fx*u)*isin(fy*v)*(RandomInteger()&0xff)/u/v;
			h+=(int64_t)icos(fx*u)*isin(fy*v)*(RandomInteger()&0xff)/u/v;
			h+=(int64_t)isin(fx*u)*icos(fy*v)*(RandomInteger()&0xff)/u/v;
			h+=(int64_t)icos(fx*u)*icos(fy*v)*(RandomInteger()&0xff)/u/v;
		}

		h=copysign(pow(fabs(h),0.4),h);

		map[x+y*Width]=h;
	}

	int64_t maxh=INT64_MIN;
	int64_t minh=INT64_MAX;

	for(int y=0;y<Height;y++)
	for(int x=0;x<Width;x++)
	{
		int64_t h=map[x+y*Width];
		if(h>maxh) maxh=h;
		if(h<minh) minh=h;
	}

	for(int y=0;y<Height;y++)
	for(int x=0;x<Width;x++)
	{
		int64_t h=map[x+y*Width];

		h=255*(h-minh)/(maxh-minh);

		if(h>255) h=255;
		if(h<0) h=0;
		map[x+y*Width]=h;
	}

	printf("#include <stdint.h>\n");
	printf("const uint8_t HeightMap[%d*%d]=\n",Width,Height);
	printf("{");
	for(int i=0;i<Width*Height;i++)
	{
		if(i%16==0) printf("\n\t");
		printf("0x%02x,",(int)map[i]);
	}
	printf("\n");
	printf("};\n");
}
