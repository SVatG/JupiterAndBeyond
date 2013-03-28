#include "PixelParticles.h"
#include "VGA.h"
#include "Random.h"
#include "Utils.h"
#include "Global.h"

#include "Graphics/Bitmap.h"
#include "Graphics/RLEBitmap.h"
#include "Graphics/Drawing.h"

#include "font.h"
#include "bezier.h"
#include "warpfunc.h"

#include "Profiling.h"

#include <string.h>

extern RLEBitmap JupiterCyborg;
extern glyph_t font_geo_glyph[];

int bezcount;
int bez_z=2000;
static int t=0;

typedef struct{
    char* text;
    int16_t starttime;
    int16_t ypos;
    int16_t speed;
    int16_t size;
    int16_t freezestart;
    int16_t freezelen;
} greets_t;

const greets_t greets[] = {
    // text             time y speed siz frt frl 
//    { "We greet:",      40, 80, 20, 12, 50, 80},
    { "Nuance",        100, 36, 16, 15, 73, 60},
    { "#mod_shrine",    170, 60, 16, 11, 60, 60},
    { "RNO",            250, 30, 24, 25, 60, 60},
    { "The Obsessed",   370, 65, 16, 10, 58, 60},
    { "   Maniacs",     370, 50, 16, 10, 54, 60},
    { "ubiktune",       470, 80, 16, 15, 63, 60},
    { "Mercury",         550, 52, 16, 15, 70, 50},
    { "coda's anus",    680, 40, 960,10, 1, 1},
    { "TRSi",           650, 25, 16, 22, 70, 60},
    { "lft",            710, 68, 16, 19, 70, 60},
    { "ARM",            770, 23, 16, 17, 80, 50},
    { "farbrausch",     840, 55, 16, 12, 60, 60},
    { "Royal Forces",   900, 35, 16, 12, 58, 60},
    { "k2",             960, 75, 19, 18, 65, 45},
    { "3State",         990, 43, 16, 15, 65, 42},
    { "Andromeda",      1020, 25, 22, 12, 46, 45},
    { "DC5",            1080, 50, 20, 19, 65, 45},
    { "Madwizards",     1130, 30, 18, 12, 55, 40},
    { "Approximate",    1150, 74, 24, 11, 40, 40}

    // k2
    // 3State
    // Andromeda
    // DC5
    // Madwizards
    // Approximate
};

void Starfield()
{
	uint8_t *framebuffer1=(uint8_t *)0x20000000;
	uint8_t *framebuffer2=(uint8_t *)0x20010000;
	memset(framebuffer1,RawRGB(0,0,0),320*200);
	memset(framebuffer2,RawRGB(0,0,0),320*200);

	SetVGAScreenMode320x200_60Hz(framebuffer1);

    Starfield_init();
#ifndef TESTING
        while(CurrentBitBinRow(songp) < 1568)
#else 
    while(1)
#endif
	{
		WaitVBL();

		uint8_t *source,*destination;
		if(t&1)
		{
			source=framebuffer1;
			destination=framebuffer2;
		}
		else
		{
			source=framebuffer2;
			destination=framebuffer1;
		}
		SetFrameBuffer(source);
//		uint32_t line1=VGALine;
//        profiling_startframe();
        Starfield_inner(source, destination);

//        profiling_endframe(destination + (199*320));
	}

	while(UserButtonState());
}


void Starfield_init()
{
	for(int i=0;i<NumberOfStars;i++)
	{
		data.stars.stars[i].x=-32768;
		data.stars.stars[i].y=0;
		data.stars.stars[i].z=RandomInteger()%4096;
	}
    point_t pos = {-250,50};
    bezcount = render_text_getbeziers(data.stars.bez, 400, "Foobar", pos, 10, font_geo_glyph); 
    for(int i=0; i<bezcount; ++i){
        data.stars.bez_z[i] = RandomInteger() % 2048 + 1024;
    }
    t=0;
}


point_t warp_perspective_starfield_old(point_t a, int t){
    return warp_perspective_generic(a, t, 
            (int[]){0,30,    220,0,     0, 70,    220,200}, 
            //(int[]){8*4,-2*4,     16*4,-4*4,      8*4, 2*4,    16*4,4*4});
            (int[]){0,0,     0,0,      0,0,0,0});
}
point_t warp_perspective_starfield(point_t a, int t){
    int x = a.x;
    int y = a.y;
    int distx = x+70*16;
    int scale = distx/(16);
    int distx2 = distx*distx/(128*16);//(128*16)*(128*16)/x;
    int x2 = distx2 - 70*16;
    int y2 = (y-40*16)*distx2/128/12 + 40*16;
//    x2 -= y2/4;
    point_t r;
    r.x=x2;
    r.y=y2;
    return r;
}

static const uint8_t palette[8][8]={
    { RawRGB(7,7,3),RawRGB(7,7,3),RawRGB(7,7,3),RawRGB(7,7,3),RawRGB(6,6,3),RawRGB(6,6,3),RawRGB(5,5,3),RawRGB(5,5,3) },
    { RawRGB(7,7,3),RawRGB(7,7,3),RawRGB(7,7,3),RawRGB(7,7,3),RawRGB(6,6,3),RawRGB(6,6,3),RawRGB(5,5,3),RawRGB(5,5,3) },
    { RawRGB(6,6,3),RawRGB(6,6,3),RawRGB(6,6,3),RawRGB(6,6,3),RawRGB(5,5,3),RawRGB(5,5,3),RawRGB(4,4,3),RawRGB(4,4,3) },
    { RawRGB(5,5,2),RawRGB(5,5,2),RawRGB(5,5,2),RawRGB(5,5,2),RawRGB(4,4,2),RawRGB(4,4,2),RawRGB(3,3,2),RawRGB(3,3,2) },
    { RawRGB(4,4,2),RawRGB(4,4,2),RawRGB(4,4,2),RawRGB(4,4,2),RawRGB(3,3,2),RawRGB(3,3,2),RawRGB(3,3,2),RawRGB(3,3,2) },
    { RawRGB(3,3,1),RawRGB(3,3,1),RawRGB(3,3,1),RawRGB(3,3,1),RawRGB(3,3,1),RawRGB(3,3,1),RawRGB(2,2,1),RawRGB(2,2,1) },
    { RawRGB(2,2,1),RawRGB(2,2,1),RawRGB(2,2,1),RawRGB(2,2,1),RawRGB(2,2,1),RawRGB(2,2,1),RawRGB(1,1,1),RawRGB(1,1,1) },
    { RawRGB(1,1,0),RawRGB(1,1,0),RawRGB(1,1,0),RawRGB(1,1,0),RawRGB(1,1,0),RawRGB(1,1,0),RawRGB(1,1,0),RawRGB(1,1,0) },
};


void Starfield_inner(uint8_t* source, uint8_t* destination)
{
    uint32_t *sourceptr=(uint32_t *)&source[0];
    uint32_t *destinationptr=(uint32_t *)&destination[0];

    if(t&1)
    {
        for(int y=0;y<200;y++)
        for(int x=0;x<320/4;x++)
        {
            uint32_t pixels=*sourceptr++;
            uint32_t isnotzero=pixels&0x25252525;
            isnotzero|=(pixels&0x4a4a4a4a)>>1;
            isnotzero|=(pixels&0x90909090)>>2;

            *destinationptr++=pixels-isnotzero;
        }
    }
    else
    {
        for(int y=0;y<200;y++)
        for(int x=0;x<320/4;x++)
        {
            uint32_t pixels=*sourceptr++;
            uint32_t isnotzero=pixels&0x24242424;
            isnotzero|=(pixels&0x48484848)>>1;
            isnotzero|=(pixels&0x90909090)>>2;

            *destinationptr++=pixels-isnotzero;
        }
    }

//		uint32_t line2=VGALine;

    Bitmap screen;
    InitializeBitmap(&screen,320,200,320,destination);


    for(int i=NumberOfStars-1;i>=0;i--)
    {
        int ox=100*data.stars.stars[i].x/data.stars.stars[i].z+159;
        int oy=100*data.stars.stars[i].y/data.stars.stars[i].z+99;

        data.stars.stars[i].x+=80/3;
        data.stars.stars[i].y-=20/3;
        data.stars.stars[i].z-=30/3;

        int x=100*data.stars.stars[i].x/data.stars.stars[i].z+159;
        int y=100*data.stars.stars[i].y/data.stars.stars[i].z+99;

        int depth=data.stars.stars[i].z>>8;
        if(depth<0) depth=0;
        if(depth<8 && data.stars.stars[i].z>10) {

            DrawLine(&screen,ox,oy,x,y,palette[depth][i&7]);
/*            if(i==1000)
            {
                DrawLine(&screen,ox,oy,x,y,7<<5);
                printf("%i %i %i %i %i\n",data.stars.stars[i].x,data.stars.stars[i].y,data.stars.stars[i].z,
                    x,y);
            }*/
        }

        if(x>=320 || y<0 || y>=200 || data.stars.stars[i].z<=40)
        {
            int z=10*sqrti(i*400*400+40*40)/SquareRootOfNumberOfStars;
            int y=RandomInteger()%2000;
            data.stars.stars[i].x=(0-159)*z/100;
            data.stars.stars[i].y=(y-999)*z/100/10;
            data.stars.stars[i].z=z;
        }
    }
    
    static int foo;
    foo++;
    if(foo>228){ foo = -128;}
    int bar=foo+50;
        
    bez_z -= 30/3;
    /*
    for(int i=0; i<bezcount; ++i){
        bezier_t* pb = &(data.stars.bez[i]);
        bezier_t b;
        for(int j=0; j<3; ++j)
        {
            pb->p[j].x += 80/3;
            pb->p[j].y -= 20/3;
            b.p[j].x = 100*pb->p[j].x/data.stars.bez_z[i]+159;
            b.p[j].y = 100*pb->p[j].y/data.stars.bez_z[i]+99;
            b.p[j].x *= 16;
            b.p[j].y *= 16;
        }
        bezier_draw(&screen, b);
        data.stars.bez_z[i] -= 30/3;
        if(data.stars.bez_z[i] < 10)
        {
            data.stars.bez_z[i] = 1000;
        }
    }
    */

    for(int i=0; i < sizeof(greets)/sizeof(greets_t); ++i)
    {
        if(t < greets[i].starttime) {
            continue;
        }
        int dt = t - greets[i].starttime; // delta-time
        int dt2=dt;
        int progress = max(dt * 256 / greets[i].freezestart - 128, 0); 
        if(dt >= greets[i].freezestart && dt < (greets[i].freezestart + greets[i].freezelen))
        {
            dt2 = greets[i].freezestart;
            progress = 128;
        }
        if(dt >= (greets[i].freezestart + greets[i].freezelen))
        {
            dt2 -= greets[i].freezelen;
            progress = -(dt2 - greets[i].freezestart)*128/greets[i].freezestart; 
        }
        int xpos = dt2 * greets[i].speed / 16;
        if(xpos > 220) {
            // moved outside frame
            continue;
        }
        point_t pos = { xpos-30, greets[i].ypos };
        //old: progress = dt * greets[i].speed / 16;
        render_text_partial_warped(&screen, greets[i].text, pos, greets[i].size,  font_geo_glyph, progress, warp_perspective_starfield, dt);
    }
    if(t>1300) t=0;

/*

    point_t pos = {foo*1.5,80};
    point_t pos2 = {80+foo*1.5,40};
    int baz=bar;
    if( baz<0) {baz=-128-baz;}
    render_text_partial_warped(&screen, "Test", pos, 20,  font_geo_glyph, baz, warp_perspective_starfield, bar);
    render_text_partial_warped(&screen, "Foobar", pos2, 15,  font_geo_glyph, baz, warp_perspective_starfield, bar);

*/
    DrawRLEBitmap(&screen,&JupiterCyborg,0,0);
    t++;
}
//		uint32_t line3=VGALine;

/*int diff1=line2-line1;
if(diff1<0) diff1+=480;
int diff2=line3-line2;
if(diff2<0) diff2+=480;

DrawHorizontalLine(&screen,0,0,diff1,RGB(0,255,0));
DrawHorizontalLine(&screen,0,199,diff2,RGB(0,255,0));*/

