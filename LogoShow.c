/**
 * Show a bitmap from flash.
 */

#include "Global.h"
#include "VGA.h"
#include "LED.h"
#include "Utils.h"
#include "Button.h"
#include "Random.h"

#include "Graphics/Bitmap.h"
#include "Graphics/Drawing.h"

#include <string.h>

#include "FullscreenBitmaps.h"

void DiagonalFadeIn(uint8_t* pixels, int t, int speedmul, int speeddiv) {
    for(int32_t y = 0; y < 200; y++ ) {
        for(int32_t x = 0; x < 320; x++) {
            int32_t shval = -((((y<<1)-x)>>2)+((t*speedmul)/speeddiv)-((32*speedmul)/speeddiv))>>4;
            shval = shval < 0 ? 0 : shval;
            shval = shval > 7 ? 7 : shval;
            int32_t pos = x+y*320;
            pixels[pos] = data.logoshow.colourLut[pixels[pos]+shval*256];
        }
    }
}

void DiagonalFadeOut(uint8_t* pixels, int t, int speedmul, int speeddiv) {
    for(int32_t y = 0; y < 200; y++ ) {
        for(int32_t x = 0; x < 320; x++) {
            int32_t shval  = ((((y<<1)-x)>>2)+((t*speedmul)/speeddiv)-((32*speedmul)/speeddiv))>>4;
            shval = shval < 0 ? 0 : shval;
            shval = shval > 7 ? 7 : shval;
            int32_t pos = x+y*320;
            pixels[pos] = data.logoshow.colourLut[pixels[pos]+shval*256];
        }
    }
}

void CodaFadeOut(uint8_t* pixels, int t, int speedmul, int speeddiv) {
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

void CodaFadeIn(uint8_t* pixels, int t, int speedmul, int speeddiv) {
    int t1 = 50-t;
    t1 = t1 > 0 ? t1 : 0;
    CodaFadeOut(pixels, t1, speedmul, speeddiv);
}

void LogoShow() {
	uint8_t *framebuffer1=(uint8_t *)0x20000000;
	uint8_t *framebuffer2=(uint8_t *)0x20010000;

	Bitmap frame1,frame2;
	InitializeBitmap(&frame1,320,200,320,framebuffer1);
	InitializeBitmap(&frame2,320,200,320,framebuffer2);

	ClearBitmap(&frame1);
	ClearBitmap(&frame2);
	
	SetVGAScreenMode320x200_60Hz(framebuffer1);

	int t=0;

	for(int32_t c = 0; c < 256; c++) {
		for(int32_t shval = 0; shval < 8; shval++) {
			data.logoshow.colourLut[c+shval*256] =
				((((c & 0xE0) >> 5) - shval)   < 0 ? 0 : ((((c & 0xE0) >> 5) - shval)   << 5)) |
				((((c & 0x1C) >> 2) - shval)   < 0 ? 0 : ((((c & 0x1C) >> 2) - shval)   << 2)) |
				((((c & 0x03)) - (shval >> 1)) < 0 ? 0 : ((((c & 0x03)) - (shval >> 1))     ));
		}
	}

	int32_t drawc = 0;
        while(!UserButtonState())
	{
		WaitVBL();
		Bitmap *currframe;
		if(t&1)
		{
			SetFrameBuffer(framebuffer1);
			currframe=&frame2;
		}
		else
		{
			SetFrameBuffer(framebuffer2);
			currframe=&frame1;
			drawc++;
		}
		
		if((t/10)%400 < 200) {
                    DrawRLEBitmap(currframe, &Jupiter1, 0, 0);
                }
                else {
                    DrawRLEBitmap(currframe, &Jupiter3, 0, 0);
                }
		uint8_t* pixels = currframe->pixels;
                if((t/10)%400 < 100) {
                    int t1 = (t/10) % 200;
                    int t2 = (t1) % 100;
                    t2 = t2 < 50 ? t2 : 50;
                    DiagonalFadeIn(currframe->pixels, t2, 20, 1);
                }
                else if((t/10)%400 < 200) {
                    int t1 = (t/10) % 200;
                    int t2 = ((t1) % 100);
                    t2 = t2 < 50 ? t2 : 50;
                    DiagonalFadeOut(currframe->pixels, t2, 20, 1);
                }
                else if((t/10)%400 < 300) {
                    int t1 = (t/10-200) % 200;
                    int t2 = ((t1) % 100);
                    t2 = t2 < 50 ? t2 : 50;
                    CodaFadeIn(currframe->pixels, t2, 20, 1);
                }
                else {
                    int t1 = (t/10-200) % 200;
                    int t2 = ((t1) % 100);
                    t2 = t2 < 50 ? t2 : 50;
                    CodaFadeOut(currframe->pixels, t2, 20, 1);
                }
                
		t++;
	}

	while(UserButtonState());
}