

#include "VGA.h"

#include "System.h"
#include "Profiling.h"

#include <string.h> //memset

int lastframe;
int startline;

/* usage:
 * call profiling_startframe before your frame-calculating/drawing operation,
 * then call profiling_endframe afterwards
 *
 * profiling_endframe takes a pointer to a framebuffer line to output a cpuload line.
 * Usually the last line of your current frame.
 *
 * Display: dropped frames are symbolized as white bars.
 * Cpuload in the current frame is shown as a bar scaling to the end.
 * That bar is green on 0 dropped frames, yellow on 1, red otherwise.
 */ 


void profiling_startframe()
{
    lastframe = VGAFrame;
    startline = VGALine;
}


// WARNING! only works for 400lines 60Hz
void profiling_endframe(uint8_t *outputline)
{
    const int linestotal = 525; // we assume 525 lines, which is 60Hz
    int endline = VGALine;
    int lines = endline - startline; 
    if(lines<0){
        lines += linestotal;
    }
    //FIXME: correctly handle endlines >440


    int frames = VGAFrame - lastframe; // frames dropped
    uint8_t colour = 7<<2; // no frames "dropped": green
    if(frames==1) colour = 7<<2 | 7<<5; // 1 frame: yellow
    if(frames>=2) colour =  7<<5; // 2 frames: red

    uint16_t width = (lines * (320 - frames*32)) / linestotal;
    width += frames*32; 
    if(width>=320){
        width = 319;
    }
    if(width<0){ width = 0; }

    // draw lost frames as white segments
    for(int j=0; j<frames; ++j){
        for(int i=0; i<20; ++i){
            outputline[j*32+i] = 0xff;
        }
        for(int i=20; i<32; ++i){
            outputline[j*32+i] = 0x00;
        }
    }

    for(int i=frames*32; i<width; ++i){
        outputline[i] = colour;
    }
    for(int i=width; i<320; ++i){
        outputline[i] = 0x00;
    }

}


    


