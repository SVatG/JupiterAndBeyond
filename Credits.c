
#include "Credits.h"
#include "font.h"
#include "FullscreenBitmaps.h"
#include "Global.h"

#include <string.h>
#include "VGA.h"
#include "Profiling.h"
#include "warpfunc.h"


extern glyph_t font_led_glyph[];

static int t;
static int startframe;

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

        while(CurrentBitBinRow(songp) < 1696)
	{
		WaitVBL();
        if(VGAFrame&1){
            WaitVBL();
        }
        SetFrameBuffer(lastframe->pixels);
        
		credits_inner(currframe);

        // swap
        Bitmap *temp = lastframe;
        lastframe = currframe;
        currframe = temp;
	}

	while(UserButtonState());
}

void credits_init()
{
    t=0;
#ifndef TESTING
    startframe = VGAFrame;
#endif
}

// input coordinates:
// x:  0 is middle of goggle
//  -2048 is left border, +2048 right border
// y: 0 is middle, +-400
point_t warp_goggles(point_t a, int t){
    // screw t
    int x = a.x;
    int y = a.y;
    // limit
    const int LIMIT=1760;
    if(x<-LIMIT){
        x = -LIMIT;
    }
    if(x>LIMIT){
        x = LIMIT;
    }
    //bulge middle
    y = y * icos(x/2) * 10 / 4096/8;
    // squash left/right
    x = isin(x/2)*7/16;

    // rotate
//    const int R_COS = 3798; //22°
//    const int R_SIN = 1534;
    const int R_COS = 3823; //21°
    const int R_SIN = 1468;
    x = x*R_COS/4096 - y*R_SIN/4096;
    y = x*R_SIN/4096 + y*R_COS/4096;


    point_t r;
    // move to correct position
    r.x = x + 145*16;
    r.y = y + 132*16;
    return r;
}

#ifdef TESTING
extern bool done;
#endif

void credits_inner(Bitmap* screen)
{
    t++;
    profiling_startframe();
    DrawRLEBitmap(screen,&Jupiter3,0,0);
//    render_text_warped_colored(screen, "Foobar!", pos, 30, font_led_glyph, warp_goggles, 0, RawRGB(0,3,0));//, RawRGB(0,7,0));
//    point_t pos2 = {20, 110};
//    render_text_warped_colored(screen, "WAHa_O6x36", pos2, 25, font_led_glyph, warp_goggles, 0, RawRGB(0,3,0));//, RawRGB(0,7,0));
    
#ifdef TESTING
    point_t pos = {-(t)*3+100, -22};
#else
    point_t pos = {-((VGAFrame-startframe))*3+100, -22};
#endif

    //char* rawtext = "Code:  WAHa_O6x36  halcy  ryx    Graphics:  Forcer^TRSi    Music:  coda";
    //char* rawtext = "WAHa_O6x36  halcy  ryx  Forcer^TRSi  coda";
    char* rawtext = "WAHa_O6x36  halcy  ryx  Forcer  coda";
    char textbuf[sizeof(rawtext)+1000]; 
    strcpy(textbuf, rawtext);
    char* text = &textbuf[0];

    clip_text(&text, &pos, 25, font_led_glyph, -1760, 1760);
//    printf("%4i %s\n",pos.x, text);
    
    render_text_warped_floodfilled(screen, text, pos, 25, font_led_glyph, warp_goggles, 0, RawRGB(4,1,0), RawRGB(6,3,0));//, RawRGB(0,7,0));
    
    profiling_endframe(&(screen->pixels[199*320]));
#ifdef TESTING
    if(*(text+1)==0) {done=true;}
#endif
}
    


