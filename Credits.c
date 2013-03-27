
#include "Credits.h"
#include "font.h"
#include "FullscreenBitmaps.h"
#include "Global.h"

#include <string.h>
#include "VGA.h"
#include "Profiling.h"


extern glyph_t font_led_glyph[];


void Credits()
{
	uint8_t *framebuffer1=(uint8_t *)0x20000000;
	uint8_t *framebuffer2=(uint8_t *)0x20010000;
	memset(framebuffer1,RawRGB(0,0,0),320*200);
	memset(framebuffer2,RawRGB(0,0,0),320*200);

	SetVGAScreenMode320x200_60Hz(framebuffer1);


	Bitmap frame1,frame2;
	InitializeBitmap(&frame1,320,200,320,framebuffer1);
	InitializeBitmap(&frame2,320,200,320,framebuffer2);
	Bitmap *currframe = &frame1;
	Bitmap *lastframe = &frame2;

    Starfield_init();

	while(!UserButtonState())
	{
		WaitVBL();
        SetFrameBuffer(lastframe->pixels);
        
		credits_inner(currframe);

        // swap
        Bitmap *temp = lastframe;
        lastframe = currframe;
        currframe = temp;
	}

	while(UserButtonState());
}



void credits_inner(Bitmap* screen)
{
    profiling_startframe();
    DrawRLEBitmap(screen,&Jupiter3,0,0);
    point_t pos = {50, 40};
    render_text(screen, "Foobar!", pos, 30, font_led_glyph);
    point_t pos2 = {20, 120};
    render_text(screen, "WaHa06x36", pos2, 25, font_led_glyph);
    profiling_endframe(&(screen->pixels[199*320]));
}
    


