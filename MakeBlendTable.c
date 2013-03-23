#include <stdio.h>
#include <stdint.h>

#define RGB8Pixels
#include "Graphics/Bitmap.h"

static uint8_t table[256*256];

int main()
{
	for(int c1=0;c1<256;c1++)
	for(int c2=0;c2<256;c2++)
	{
		uint16_t halfc1=(c1>>1)&PixelAllButHighBits;
		uint16_t halfc2=(c2>>1)&PixelAllButHighBits;
		uint16_t carry=c1&c2&PixelLowBits;

		table[c1+c2*256]=halfc1+halfc2+carry;

/*		int r1=ExtractRed(c1);
		int g1=ExtractGreen(c1);
		int b1=ExtractBlue(c1);
		int r2=ExtractRed(c2);
		int g2=ExtractGreen(c2);
		int b2=ExtractBlue(c2);

//		int r=(r1+r2+2*(255-(255-r1)*(255-r2)/255))/4;
//		int g=(g1+g2+2*(255-(255-g1)*(255-g2)/255))/4;
//		int b=(b1+b2+2*(255-(255-b1)*(255-b2)/255))/4;
		int r=(r1+r2)/2;
		int g=(g1+g2)/2;
		int b=(b1+b2)/2;

		table[c1+c2*256]=RGB(r,g,b);*/
	}

	printf("#include <stdint.h>\n");
	printf("const uint8_t BlendTable[256*256]=\n");
	printf("{");
	for(int i=0;i<256*256;i++)
	{
		if(i%16==0) printf("\n\t");
		printf("0x%02x,",table[i]);
	}
	printf("\n");
	printf("};\n");
}
