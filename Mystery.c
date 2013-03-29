#include "Mystery.h"
#include "VGA.h"
#include "Random.h"
#include "Utils.h"
#include "Global.h"

#include "Graphics/Bitmap.h"
#include "Graphics/RLEBitmap.h"
#include "Graphics/Drawing.h"

#include <string.h>

extern RLEBitmap JupiterHands;

void CodaFadeOut4(uint8_t* pixels, int t, int speedmul, int speeddiv) {
    float a = (((float)((t*speedmul)/speeddiv))/50.0f)*1.4f;
    int32_t palsub_r = (int32_t)(((181.0f*a)/255.0f)*7.0f+0.5f);
    int32_t palsub_g = (int32_t)(((235.0f*a)/255.0f)*7.0f+0.5f);
    int32_t palsub_b = (int32_t)(((145.0f*a)/255.0f)*3.0f+0.5f);
    for(int32_t y = 0; y < 200; y++ ) {
        for(int32_t x = 0; x < 320; x++) {
            int32_t pos = x+y*320;
            
            uint8_t pixel = pixels[pos];
            int32_t r = (pixel&(7<<5))>>5;
            int32_t g = (pixel&(7<<2))>>2;
            int32_t b = (pixel&(3));
            
            r = r - palsub_r;
            r = r > 0 ? r : 0;
            g = g - palsub_g;
            g = g > 0 ? g : 0;
            b = b - palsub_b;
            b = b > 0 ? b : 0;
            
            pixels[pos] = (uint8_t)(r<<5)|(g<<2)|b;
        }
    }
}

void CodaFadeIn4(uint8_t* pixels, int t, int speedmul, int speeddiv) {
    int t1 = 50-t;
    t1 = t1 > 0 ? t1 : 0;
    CodaFadeOut4(pixels, t1, speedmul, speeddiv);
}

void IDontEvenKnow()
{
	uint8_t *framebuffer1=(uint8_t *)0x20000000;
	uint8_t *framebuffer2=(uint8_t *)0x20010000;
	memset(framebuffer1,0,320*200);
	memset(framebuffer2,0,320*200);

	SetVGAScreenMode320x200_60Hz(framebuffer1);

	int t=0;
        while(CurrentBitBinRow(songp) < 1850)
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

		int32_t a=t-300;
		int32_t sin_a=isin(a);
		int32_t cos_a=icos(a);
		int32_t u=0;
		int32_t v=0;

		#define NumberOfStrips2 64

		for(int i=0;i<NumberOfStrips2;i++)
		{
			Pixel c=RGB(255*i/NumberOfStrips2,255*i*i/NumberOfStrips2/NumberOfStrips2,255*i/NumberOfStrips2);

			int32_t z=Fix(i*2);
			int32_t rz=idiv(Fix(256*4),z);

			int32_t du=imul(z,-sin_a)/320;
			int32_t dv=imul(z,cos_a)/320;
			int32_t u=imul(z,cos_a)-du*200/2;
			int32_t v=imul(z,sin_a)-dv*200/2;

			/*for(int x=0;x<320;x++)
			{
				int32_t h=isin(u)+isin(v);
				int32_t y=100-FixedToInt(imul(h,rz))+isin(x*10+t*20)/512;

				DrawPixel(&screen,x,y,c);

				u+=du;
				v+=dv;
			}*/
			for(int y=0;y<200;y++)
			{
				int32_t h=isin(u)+isin(v);
				int32_t x=160-FixedToInt(imul(h,rz))+isin(y*10+t*20)/256;

				//CompositePixel(&screen,x,y,c,FastHalfTransparentCompositionMode);
				DrawPixel(&screen,x,y,c);

				u+=du;
				v+=dv;
			}
		}

		DrawRLEBitmapNoClip(&screen,&JupiterHands,0,0);

		t++;

                if(t < 50) {
                    CodaFadeIn4(destination,t+25,4,1);
                }
                else if(t>450 && t<500){
                    CodaFadeOut4(destination,t-450,2,1);
                }
                else if(t>=500) {
                    CodaFadeOut4(destination,49,2,1);
                }
	}

	while(UserButtonState());
}

