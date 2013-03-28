
#include "font.h"
#include "2dvector.h"
#include "warpfunc.h"

#include "Global.h" // for testing, TODO remove

#ifdef TESTING
#include <stdio.h>
#endif

void clip_text(char** text, point_t *pos, int size, const glyph_t* font, int xmin, int xmax)
{
    int x = pos->x * BEZ_SCALEDOWN;
    char* curtext = *text;
    int i=0;
    while(*curtext!= 0){
        const glyph_t* g = &font[((*curtext)-0x20)];
        if(x  + ((g->width*size)>>FONT_SIZE_LOG2) < xmin) // glyph too far left
        {
            // move pos and text
            pos->x = x/BEZ_SCALEDOWN;
            *text = curtext;
        }
        if(x > xmax) // too far right
        {
            *curtext = 0; // terminating zero
            return;
        }
        x += (g->width*size)>>FONT_SIZE_LOG2;
        curtext++;
        i++;
    }
    return;
}


void render_text(Bitmap *dest, char* text, point_t pos, int size, const glyph_t* font){
    pos = pscale(pos, BEZ_SCALEDOWN, 0); // scale pos to bezier coordinates
    while(*text != 0){
        glyph_t g = font[((*text)-0x20)];
        for(int i=0; i<g.datalen; ++i){
            bezier_t b = g.data[i];
            // transform b
            for(int j=0; j<3; ++j){
                point_t t;
                t = b.p[j];
                t = pscale(t, size, FONT_SIZE_LOG2); // scale
                t = padd(t, pos); // position
                b.p[j]=t;
            }
            bezier_draw(dest, b);
            
        }
        pos.x += (g.width*size)>>FONT_SIZE_LOG2;

        text++;
    }
}

void render_text_floodfilled(Bitmap *dest, char* text, point_t pos, int size, const glyph_t* font, uint8_t color_border, uint8_t color_fill){
    fillpixel_t* pixptr = data.greets.fillpixels;
    pos = pscale(pos, BEZ_SCALEDOWN, 0); // scale pos to bezier coordinates
    while(*text != 0){
        glyph_t g = font[((*text)-0x20)];
        for(int i=0; i<g.datalen; ++i){
            bezier_t b = g.data[i];
            // transform b
            for(int j=0; j<3; ++j){
                point_t t;
                t = b.p[j];
                t = pscale(t, size, FONT_SIZE_LOG2); // scale
                t = padd(t, pos); // position
                b.p[j]=t;
            }
            bezier_draw(dest, b);
            pixptr = bezier_fill_writepixels(dest, b, pixptr, color_border);
            
        }
        // fill every letter separately
        floodfill(dest, data.greets.fillpixels, pixptr, color_fill);
        pixptr = data.greets.fillpixels; // reset floodfill-stored-pixels-ptr to beginning
        pos.x += (g.width*size)>>FONT_SIZE_LOG2;

        // next letter
        text++;
    }
}

void render_text_warped_floodfilled(Bitmap *dest, char* text, point_t pos, int size, const glyph_t* font, warpfunc_t warpfunc, int t, uint8_t color_border, uint8_t color_fill){
    fillpixel_t* pixptr = data.greets.fillpixels;
    pos = pscale(pos, BEZ_SCALEDOWN, 0); // scale pos to bezier coordinates
    while(*text != 0){
        glyph_t g = font[((*text)-0x20)];
        for(int i=0; i<g.datalen; ++i){
            bezier_t b = g.data[i];
            // transform b
            for(int j=0; j<3; ++j){
                point_t p;
                p = b.p[j];
                p = pscale(p, size, FONT_SIZE_LOG2); // scale
                p = padd(p, pos); // position
                p = warpfunc(p,t);
                b.p[j]=p;
            }
            pixptr = bezier_fill_writepixels(dest, b, pixptr, color_border);
            
        }
        // fill every letter separately
        floodfill(dest, data.greets.fillpixels, pixptr, color_fill);
//        printf("%i\n",pixptr-data.greets.fillpixels);
        pixptr = data.greets.fillpixels; // reset floodfill-stored-pixels-ptr to beginning
        pos.x += (g.width*size)>>FONT_SIZE_LOG2;

        // next letter
        text++;
    }
}

int render_text_getbeziers(bezier_t *dest, int destsize, char* text, point_t pos, int size, const glyph_t* font){
    pos = pscale(pos, BEZ_SCALEDOWN, 0); // scale pos to bezier coordinates
    int count = 0;
    while(*text != 0){
        glyph_t g = font[((*text)-0x20)];
        for(int i=0; i<g.datalen; ++i){
            bezier_t b = g.data[i];
            // transform b
            for(int j=0; j<3; ++j){
                point_t t;
                t = b.p[j];
                t = pscale(t, size, FONT_SIZE_LOG2); // scale
                t = padd(t, pos); // position
                b.p[j]=t;
            }
            *dest = b;
            dest++;
            count++;
            if(count == destsize)
            {
                return count;
            }
        }
        pos.x += (g.width*size)>>FONT_SIZE_LOG2;

        text++;
    }
    return count;
}

void render_text_warped(Bitmap *dest, char* text, point_t pos, int size, const glyph_t* font, warpfunc_t warpfunc, int t){
    pos = pscale(pos, BEZ_SCALEDOWN, 0); // scale pos to bezier coordinates
    while(*text != 0){
        glyph_t g = font[((*text)-0x20)];
        for(int i=0; i<g.datalen; ++i){
            bezier_t b = g.data[i];
            // transform b
            for(int j=0; j<3; ++j){
                point_t p;
                p = b.p[j];
                p = pscale(p, size, FONT_SIZE_LOG2); // scale
                p = padd(p, pos); // position
                p = warpfunc(p,t);
                b.p[j]=p;
            }
            bezier_draw(dest, b);
        }
        pos.x += (g.width*size)>>FONT_SIZE_LOG2;

        text++;
    }
}

void render_text_warped_colored(Bitmap *dest, char* text, point_t pos, int size, const glyph_t* font, warpfunc_t warpfunc, int t, uint8_t color)
{
    pos = pscale(pos, BEZ_SCALEDOWN, 0); // scale pos to bezier coordinates
    while(*text != 0){
        glyph_t g = font[((*text)-0x20)];
        for(int i=0; i<g.datalen; ++i){
            bezier_t b = g.data[i];
            // transform b
            for(int j=0; j<3; ++j){
                point_t p;
                p = b.p[j];
                p = pscale(p, size, FONT_SIZE_LOG2); // scale
                p = padd(p, pos); // position
                p = warpfunc(p,t);
                b.p[j]=p;
            }
            bezier_draw_colored(dest, b, color);
        }
        pos.x += (g.width*size)>>FONT_SIZE_LOG2;

        text++;
    }
}

void render_text_partial_warped(Bitmap *dest, char* text, point_t pos, int size, const glyph_t* font, int percent, warpfunc_t warpfunc, int time){
    pos = pscale(pos, BEZ_SCALEDOWN, 0); // scale pos to bezier coordinates
    int reversed = 0;
    if(percent < 0){
        percent = -percent;
        reversed = 1;
    }
    while(*text != 0){
        glyph_t g = font[((*text)-0x20)];
        for(int k=0; k<GLYPHGROUPS; ++k){ // iterate groups in glyph
            // get amount of actually used paths
            int segments = g.grouplen[k];
            for(int i=0; i<g.grouplen[k]; ++i){
                bezier_t b = g.data[i + g.groupstart[k]];
                // transform b
                for(int j=0; j<3; ++j){
                    point_t t;
                    t = b.p[j];
                    t = pscale(t, size, FONT_SIZE_LOG2); // scale
                    t = padd(t, pos); // position
                    t = warpfunc(t, time);
                    b.p[j]=t;
                }
                // decide if to draw fully, partial or not
                int percent2 = percent>=100 ? percent-100 : percent;
                if(((i+1)*128)<(percent*segments)){
                    // draw fully (if starting)
                    if(!reversed)
                        bezier_draw(dest, b);
                } else if((i*128)<(percent*segments)){
                    // draw partial
                    int p = percent*segments - 128*i;
                    bezier_draw_partial(dest, b, p - 128*reversed);
                } else {
                    // draw nothing (if starting
                    if(reversed)
                        bezier_draw(dest, b);
                }
            }
        }
        pos.x += (g.width*size)>>FONT_SIZE_LOG2;

        text++;
    }
}

void render_text_partial(Bitmap *dest, char* text, point_t pos, int size, const glyph_t* font, int percent){
    pos = pscale(pos, BEZ_SCALEDOWN, 0); // scale pos to bezier coordinates
    int reversed = 0;
    if(percent < 0){
        percent = -percent;
        reversed = 1;
    }
    while(*text != 0){
        glyph_t g = font[((*text)-0x20)];
        for(int k=0; k<GLYPHGROUPS; ++k){ // iterate groups in glyph
            // get amount of actually used paths
            int segments = g.grouplen[k];
            for(int i=0; i<g.grouplen[k]; ++i){
                bezier_t b = g.data[i + g.groupstart[k]];
                // transform b
                for(int j=0; j<3; ++j){
                    point_t t;
                    t = b.p[j];
                    t = pscale(t, size, FONT_SIZE_LOG2); // scale
                    t = padd(t, pos); // position
                    b.p[j]=t;
                }
                // decide if to draw fully, partial or not
                int percent2 = percent>=100 ? percent-100 : percent;
                if(((i+1)*128)<(percent*segments)){
                    // draw fully (if starting)
                    if(!reversed)
                        bezier_draw(dest, b);
                } else if((i*128)<(percent*segments)){
                    // draw partial
                    int p = percent*segments - 128*i;
                    bezier_draw_partial(dest, b, p - 128*reversed);
                } else {
                    // draw nothing (if starting
                    if(reversed)
                        bezier_draw(dest, b);
                }
            }
        }
        pos.x += (g.width*size)>>FONT_SIZE_LOG2;

        text++;
    }
}

void get_text_points(point_t *dest, int *destsize, char* text, point_t pos, int size, const glyph_t* font, bool first){
    // gets a list of bezier points for text transitions
    // writes pairs of entpoint, controlpoint into dest.
    // destsize is the (max) number of pairs, and gets written with the actual number
    pos = pscale(pos, BEZ_SCALEDOWN, 0); // scale pos to bezier coordinates
    int destpos = 0;
    while(*text != 0){
        glyph_t g = font[((*text)-0x20)];
        for(int k=0; k<GLYPHGROUPS; ++k){ // iterate groups in glyph
            // get amount of actually used paths
            if(g.grouplen[k]==0){
                continue; // skip
            }
            bezier_t b;
            if(first){
                b = g.data[g.groupstart[k]];
            } else {
                b = g.data[g.groupstart[k]+g.grouplen[k]-1];
            }
            // transform b
            for(int j=0; j<3; ++j){
                point_t t;
                t = b.p[j];
                t = pscale(t, size, FONT_SIZE_LOG2); // scale
                t = padd(t, pos); // position
                b.p[j]=t;
            }
            point_t endpoint, ctrlpoint;
            if(first){
                endpoint = b.p[0];
            } else {
                endpoint = b.p[2];
            }
            ctrlpoint = b.p[1];

            //TODO: calculate normal, scale it, and place the control point so that the exiting bezier has a certain curvature
            ctrlpoint = psub(padd(endpoint, endpoint), ctrlpoint); 
            ctrlpoint = pnorm(endpoint, ctrlpoint, size*BEZ_SCALEDOWN/2); 
            // for now: we mirror the controlpoint on the endpoint
//            ctrlpoint = psub(padd(endpoint, endpoint), ctrlpoint); 
            dest[destpos*2] = endpoint;
            dest[destpos*2+1] = ctrlpoint;
            destpos++;
            if(destpos >= *destsize){
                return;
            }
        }
        pos.x += (g.width*size)>>FONT_SIZE_LOG2;

        text++;
    }
    *destsize = destpos; // write out number of pairs written
}

void get_text_points_warped(point_t *dest, int *destsize, char* text, point_t pos, int size, const glyph_t* font, bool first, warpfunc_t warpfunc, int time){
    // gets a list of bezier points for text transitions
    // writes pairs of entpoint, controlpoint into dest.
    // destsize is the (max) number of pairs, and gets written with the actual number
    pos = pscale(pos, BEZ_SCALEDOWN, 0); // scale pos to bezier coordinates
    int destpos = 0;
    while(*text != 0){
        glyph_t g = font[((*text)-0x20)];
        for(int k=0; k<GLYPHGROUPS; ++k){ // iterate groups in glyph
            // get amount of actually used paths
            if(g.grouplen[k]==0){
                continue; // skip
            }
            bezier_t b;
            if(first){
                b = g.data[g.groupstart[k]];
            } else {
                b = g.data[g.groupstart[k]+g.grouplen[k]-1];
            }
            // transform b
            for(int j=0; j<3; ++j){
                point_t t;
                t = b.p[j];
                t = pscale(t, size, FONT_SIZE_LOG2); // scale
                t = padd(t, pos); // position
                t = warpfunc(t, time);
                b.p[j]=t;
            }
            point_t endpoint, ctrlpoint;
            if(first){
                endpoint = b.p[0];
            } else {
                endpoint = b.p[2];
            }
            ctrlpoint = b.p[1];

            //TODO: calculate normal, scale it, and place the control point so that the exiting bezier has a certain curvature
            // for now: we mirror the controlpoint on the endpoint
            ctrlpoint = psub(padd(endpoint, endpoint), ctrlpoint); 
            ctrlpoint = pnorm(endpoint, ctrlpoint, size*BEZ_SCALEDOWN/2); 
            dest[destpos*2] = endpoint;
            dest[destpos*2+1] = ctrlpoint;
            destpos++;
            if(destpos >= *destsize){
                return;
            }
        }
        pos.x += (g.width*size)>>FONT_SIZE_LOG2;

        text++;
    }
    *destsize = destpos; // write out number of pairs written
}
static int cmp_ptpair(const void* p1, const void* p2){
    return(((point_t*)p1)->x - ((point_t*)p2)->x);
}

void make_transition(Bitmap *dest, point_t *startpts, int startsize, point_t *endpts, int endsize, int ratio){
    // sort by x:
    qsort(startpts, startsize, sizeof(point_t)*2, cmp_ptpair);
    qsort(endpts, endsize, sizeof(point_t)*2, cmp_ptpair);

    for(int i=0; i<startsize; ++i){
        // this handles startsize>endsize well. TODO: handle endsize>startsize
        
        point_t p1, p2, p3, p4, p5;
        p1 = startpts[i*2];
        p2 = startpts[i*2+1];
        p5 = endpts[(i*endsize/startsize)*2];
        p4 = endpts[(i*endsize/startsize)*2+1];
        p3 = pavg(p2, p4);
        int ratio1, ratio2;
        if(ratio>0){
            ratio1 = ratio*2;
            ratio2 = max(ratio*2-128, 0);
        } else {
            ratio1 = min(0, ratio*2+128);
            ratio2 = ratio*2;
        }
        bezier_t b;
        b.p[0]=p1;
        b.p[1]=p2;
        b.p[2]=p3;
        bezier_draw_partial(dest, b, ratio1);
        b.p[0]=p3;
        b.p[1]=p4;
        b.p[2]=p5;
        bezier_draw_partial(dest, b, ratio2);
    }
}

