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
        while(CurrentBitBinRow(songp) < 512)
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
		
                DrawRLEBitmap(currframe, &Jupiter1, 0, 0);

		uint8_t* pixels = currframe->pixels;
//                 if(t < 2000) {
                if(t<190) {
                    DiagonalFadeIn(currframe->pixels, t-50, 2, 1);
                }
                else {
                    DiagonalFadeOut(currframe->pixels, t-355, 2, 1);
                }
//                 }
//                 else {
//                     DiagonalFadeOut(currframe->pixels, t-500, 2, 1);
//                 }
                
		t++;
	}

	while(UserButtonState());
}