#include "Mystery.h"
#include "VGA.h"
#include "Random.h"
#include "Utils.h"
#include "Global.h"

#include "Graphics/Bitmap.h"
#include "Graphics/Drawing.h"

#include <string.h>

void IDontEvenKnow()
{
	uint8_t *framebuffer1=(uint8_t *)0x20000000;
	uint8_t *framebuffer2=(uint8_t *)0x20010000;
	memset(framebuffer1,0,320*200);
	memset(framebuffer2,0,320*200);

	SetVGAScreenMode320x200_60Hz(framebuffer1);

	int t=0;
	while(!UserButtonState())
	{
		WaitVBL();

		uint8_t *destination;
		if(t&1)
		{
			destination=framebuffer2;
			SetFrameBuffer(framebuffer1);
		}
		else
		{
			destination=framebuffer1;
			SetFrameBuffer(framebuffer2);
		}

		Bitmap screen;
		InitializeBitmap(&screen,320,200,320,destination);

		#define NumberOfStrips2 32

		int32_t a=t*4;
		int32_t sin_a=isin(a);
		int32_t cos_a=icos(a);
		int32_t u=0;
		int32_t v=0;

		for(int i=0;i<NumberOfStrips2;i++)
		{
			Pixel c=RGB(255*i/NumberOfStrips2,255*i/NumberOfStrips2,255*i/NumberOfStrips2);

			int32_t z=Fix(i*4);
			int32_t rz=idiv(Fix(256*4),z);

			int32_t du=imul(z,-sin_a)/320;
			int32_t dv=imul(z,cos_a)/320;
			int32_t u=imul(z,cos_a)-du*320/2;
			int32_t v=imul(z,sin_a)-dv*320/2;

			for(int x=0;x<320;x++)
			{
				int32_t h=isin(u)+isin(v);
				int32_t y=100-FixedToInt(imul(h,rz))+isin(x*10+t*20)/512;

				DrawPixel(&screen,x,y,c);

				u+=du;
				v+=dv;
			}
		}

		t++;
	}

	while(UserButtonState());
}
