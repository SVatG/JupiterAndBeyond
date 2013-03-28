/**
 * Rasterizer
 */


#include <time.h>
#include <stdio.h>

#include <GL/gl.h>
#include <GL/glut.h>


#include "main.h"
#include "bezier.h"
#include "font.h"
#include "warpfunc.h"

#include "Global.h"

#include "Starfield.h"


#include "VGA.h"

void SetVGAScreenMode240(uint8_t *framebuffer,int pixelsperrow,int pixelclock) {}
void SetVGAScreenMode200_60Hz(uint8_t *framebuffer,int pixelsperrow,int pixelclock) {}
void SetVGAScreenMode200(uint8_t *framebuffer,int pixelsperrow,int pixelclock) {}
void SetVGAScreenMode175(uint8_t *framebuffer,int pixelsperrow,int pixelclock) {}
void profiling_startframe() {}
void profiling_endframe(uint8_t* foo) {}
void SetFrameBuffer(uint8_t *framebuffer) {}


//#include "VectorLibrary/VectorFixed.h"
//#include "VectorLibrary/MatrixFixed.h"


#define R(x) (IntToFixed( ((x) & (7 << 5)) >> 5))
#define G(x) (IntToFixed( ((x) & (7 << 2)) >> 2))
#define B(x) (IntToFixed( (x) & (3) ))

int32_t rotcnt = 0;
int32_t frameCnt = 0;
time_t lastTime = 0;

extern glyph_t font_enri_glyph[];
extern glyph_t revision_logo_glyph[];

union GlobalData data;

bool done=false;

#ifdef DEBUG
int callcount_bezier;
int callcount_setpixel;
#endif //DEBUG


uint8_t image[WIDTH*HEIGHT];
uint8_t image2[WIDTH*HEIGHT];
Bitmap frame;
Bitmap frame2;
point_t textpts1[200];
point_t textpts2[200];
Bitmap *currframe = &frame;
Bitmap *lastframe = &frame2;
bool benchmark = false;
bool slow=false;
int slowtick=0;

int tick,tock;
extern bool done;

void saveimage(Bitmap *frame, char* filename)
{
    FILE* fd = fopen(filename, "wb");
    fprintf(fd, "P6\n320 200\n255\n");
    uint8_t p;
    uint8_t rgb[3];
    for(int i=0; i<(320*200); ++i){
        p=frame->pixels[i];
        rgb[0]=p & (7<<5);
        rgb[1]=(p<<3) & (7<<5);
        rgb[2]=(p<<6) & (3<<6);
        fwrite(rgb, 1, 3, fd);
    }
    fclose(fd);
}

void display() {

    Bitmap *temp = currframe;
    currframe = lastframe;
    lastframe = temp;

    if(slowtick==0){
        // clear framebuffer
/*        for( int x = 0; x < WIDTH; x++ ) {
            for( int y = 0; y < HEIGHT; y++ ) {
                currframe->pixels[x+y*WIDTH] = 0;
            }
        }*/
    }

	time_t thisTime;
//    timespec ts;
//    clock_gettime(CLOCK_REALTIME, &ts);
	time(&thisTime);
	if(thisTime - lastTime >= 10) {
		printf("FPS: %d\n", frameCnt/10);
		time(&lastTime);
		frameCnt = 0;
	}
	frameCnt++;
    if(frameCnt%4 == 0){
//        tick++;
    }

/*    clock_t clk_start; 
    clk_start = clock();
*/

#ifdef DEBUG
    callcount_bezier=0;
    callcount_setpixel=0;
#endif //DEBUG

    if(!slow || slowtick==0){

    //    greets_inner(currframe);
//        logo_inner(currframe);
//        Starfield_inner(lastframe->pixels, currframe->pixels);
        credits_inner(currframe);
  /*      static int tick=0;
        tick++;
        point_t p = {20, 20};
        render_text_warped(currframe, "foobar", p, 70, font_enri_glyph, warp_up_away, tick);
*/
    
    
    }

    if(slow){
        slowtick++;
        if(slowtick>10){
            slowtick=0;
        }
    }

    if(done){
        //printf("done!\n");
    }

    if(!benchmark){
#ifdef DEBUG
        printf("%i beziers, %i drawpixels, %i pixels\n",callcount_bezier, callcount_setpixel, count_pixels());
#endif //DEBUG

        // copy out, upscaling
            
        static uint8_t imageDisp[WIDTH*HEIGHT*UPSCALE*UPSCALE];
        for( int x = 0; x < WIDTH; x++ ) {
            for( int y = 0; y < HEIGHT; y++ ) {
                for( int a=0; a<UPSCALE; a++){
                    for( int b=0; b<UPSCALE; b++){
                        imageDisp[x*UPSCALE+a+(y*UPSCALE+b)*WIDTH*UPSCALE] = currframe->pixels[x+(HEIGHT-1-y)*WIDTH];
                    }
                }
            }
        }
        
        glDrawPixels(WIDTH*UPSCALE, HEIGHT*UPSCALE, GL_RGB, GL_UNSIGNED_BYTE_3_3_2, imageDisp);
        glutSwapBuffers();

        // snapshot
        
       /* 
        char fn[100];
        snprintf(fn, 99, "cap_%04i.ppm", frameCnt);
        saveimage(currframe, fn);
        */
    }
}





int count_pixels(){
    int r=0;
    for(int i=0; i<WIDTH; i++){
        for(int j=0; j<HEIGHT; j++){
            if(currframe->pixels[j+HEIGHT*i] != 0){
                r++;
            }
        }
    }
    return(r);
}
        


int main(int argc, char** argv) {

	InitializeBitmap(&frame,320,200,320,image);
	InitializeBitmap(&frame2,320,200,320,image2);
    //logo_init();
    //greets_init();
    Starfield_init();
    credits_init();
    done=false;
//    slow=true;
    if(argc==1){ // no parameters: display
        glutInit(&argc, argv);
        glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
        glutInitWindowSize(WIDTH*UPSCALE, HEIGHT*UPSCALE);
        glutCreateWindow("fonttest");


        glutDisplayFunc(display);
        glutIdleFunc(display);
        glutMainLoop();
    } else { // benchmark
        benchmark = true;
        display();
        while(!done){
            display();
        }
    }
}
