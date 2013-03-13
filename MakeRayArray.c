#include <stdio.h>
#include <stdint.h>
#include <math.h>

#include "Utils.h"

#define Width 212
#define Height 133

static int32_t rayarray[Width*Height*3];

int main()
{
	for(int y=0;y<Height;y++)
	for(int x=0;x<Width;x++)
	{
		int32_t fx=Fix(2*x+1-Width)/(Width);
		int32_t fy=Fix(2*y+1-Height)/(Width);
		int32_t fz=-Fix(1)+isq(fx)/2+isq(fy)/2;

		int32_t r2=isq(fx)+isq(fy)+isq(fz);
		int32_t r=sqrti(r2<<12);

		rayarray[3*(x+y*Width)+0]=idiv(fx,r)<<3;
		rayarray[3*(x+y*Width)+1]=idiv(fy,r)<<3;
		rayarray[3*(x+y*Width)+2]=idiv(fz,r)<<3;
	}

	uint32_t *table;

	printf("#include <stdint.h>\n");
	printf("const int16_t rayarray[%d*%d*3]=\n",Width,Height);
	printf("{");
	for(int i=0;i<Width*Height*3;i++)
	{
		if(i%8==0) printf("\n\t");
		printf("0x%04x,",(uint16_t)rayarray[i]);
	}
	printf("\n");
	printf("};\n");
}
