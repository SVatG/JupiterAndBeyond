//#include "main.h"
#include "bezier.h"
#include "2dvector.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define WIDTH 320
#define HEIGHT 200
/*
void bezier_draw_path_partial(Bitmap *dest, bezier_t *data, int len, int percent){
    for(int i=0; i<len; ++i){
        point_t a,b,p;
        int x,y;
        point_t* bez = (point_t*) (data + i);
        for(int t=0; t<BEZ_STEPS; ++t){
            a = padd(bez[0], pscale_bez(psub(bez[1],bez[0]),t));
            b = padd(bez[1], pscale_bez(psub(bez[2],bez[1]),t));
            p = padd(a, pscale_bez(psub(b,a),t));
            bezier_setpixel(dest, p,255);

        }
#ifdef SHOW_CONTROL_POINTS        
        bezier_setpixel(dest, bez.p[2],7<<5);
        bezier_setpixel(dest, bez.p[1],7<<2);
#endif //SHOW_CONTROL_POINTS
    }
}*/

void bezier_draw_partial(Bitmap *dest, bezier_t bez, int ratio){    
    // ratio is between 0 and 128. Or negative for drawing only the end, not the beginning.
    if(ratio<0){         
        // turn around bez
        ratio = -ratio;
        point_t tmp;
        tmp = bez.p[0];
        bez.p[0] = bez.p[2];
        bez.p[2] = tmp;
    }
    if(ratio>=128){
        bezier_draw(dest, bez);
        return;
    }

    // calculate sub-curve
    point_t p1 = padd(pscale(bez.p[0], 128-ratio, 7), pscale(bez.p[1], ratio, 7));
    point_t p2 = padd(pscale(bez.p[1], 128-ratio, 7), pscale(bez.p[2], ratio, 7));
    point_t pm = padd(pscale(p1, 128-ratio, 7), pscale(p2, ratio, 7));
    
    bezier_t bez2;
    bez.p[1] = p1;
    bez.p[2] = pm;
    bezier_draw(dest, bez);
}


void bezier_draw(Bitmap *dest, bezier_t bez){
    static bezier_t stack[15];
    int stackpos = 0;
    stack[0] = bez;
    //TODO: draw last point of curve

    while(stackpos >=0){
        bezier_t *b = &stack[stackpos--];
        int len = plen(b->p[0],b->p[1])+plen(b->p[1],b->p[2]);
        if(len>BEZ_SCALEDOWN*2){
            point_t p1, p2, pm, bp2;
            p1 = pavg(b->p[0], b->p[1]);
            p2 = pavg(b->p[1], b->p[2]);
            pm = pavg(p1,p2);
            bezier_t *t;
            t = &stack[++stackpos];
            t->p[0] = b->p[0];
            t->p[1] = p1;
            bp2 = b->p[2]; // rescue value before writing there
            t->p[2] = pm;
            t = &stack[++stackpos];
            t->p[0] = pm;
            t->p[1] = p2;
            t->p[2] = bp2;
        } else {
            bezier_setpixel(dest, b->p[0], 0xff);// RawRGB(5,5,2));
            bezier_setpixel(dest, b->p[1], 0xff);//RawRGB(5,5,2));
        }
    }
//    bezier_setpixel(dest, bez.p[0],255);
//    bezier_setpixel(dest, bez.p[2],255);
}

void bezier_draw_colored(Bitmap *dest, bezier_t bez, uint8_t color){
    static bezier_t stack[15];
    int stackpos = 0;
    stack[0] = bez;
    //TODO: draw last point of curve

    while(stackpos >=0){
        bezier_t *b = &stack[stackpos--];
        int len = plen(b->p[0],b->p[1])+plen(b->p[1],b->p[2]);
        if(len>BEZ_SCALEDOWN*2){
            point_t p1, p2, pm, bp2;
            p1 = pavg(b->p[0], b->p[1]);
            p2 = pavg(b->p[1], b->p[2]);
            pm = pavg(p1,p2);
            bezier_t *t;
            t = &stack[++stackpos];
            t->p[0] = b->p[0];
            t->p[1] = p1;
            bp2 = b->p[2]; // rescue value before writing there
            t->p[2] = pm;
            t = &stack[++stackpos];
            t->p[0] = pm;
            t->p[1] = p2;
            t->p[2] = bp2;
        } else {
            bezier_setpixel(dest, b->p[0], color);
            bezier_setpixel(dest, b->p[1], color);
        }
    }
}


void bezier_setpixel(Bitmap *dest, point_t p, uint8_t colour){
    int x,y, distance;
    x = p.x / BEZ_SCALEDOWN;
    y = p.y / BEZ_SCALEDOWN;
//    printf("draw: (%i,%i)\n", (int)x, (int)y);
    if(x>=WIDTH || x<0){
 //       printf("x: %i \n",(int)x);
        return;
    }
    if(y>=HEIGHT|| y<0){
//        printf("y: %i \n",(int)y);
        return;
    }
    distance = abs((p.x % BEZ_SCALEDOWN)-BEZ_SCALEDOWN/2);
    distance += abs((p.y % BEZ_SCALEDOWN)-BEZ_SCALEDOWN/2);
/*    static int ctr;
    ctr++;
    if(ctr>10000){
        printf("%i\n",distance);
        ctr=0;
    }*/
    int intensity = 9-(distance * 4) / BEZ_SCALEDOWN;
    if(intensity<0)
        intensity=0;
    if(intensity>8)
        intensity=8;
    
//    image[p.x+(p.y)*WIDTH]=colour;
    uint8_t oldpixel = dest->pixels[x+(HEIGHT-y-1)*WIDTH];
    int colour_decomposed = (colour * ((1<<(8+16))|(1<<(6+8))|(1<<3))) & 0x03070700;
    int oldpixel_decomposed = (oldpixel * ((1<<(8+16))|(1<<(6+8))|(1<<3))) & 0x03070700;
    int colour_scaled = (colour_decomposed*intensity+0x03010100)>>3;
    int newcolour = (colour_scaled) & 0x03070700; //FIXME: add oldpixel here or something
    int newcolour2 = ((newcolour * ((1)|(1<<(2+8))|(1<<(5+16)))) & 0xff000000)>>24;
//    printf("%x %x %x %x\n",colour, newcolour, colour_scaled, newcolour);
    dest->pixels[x+(HEIGHT-y-1)*WIDTH] = newcolour2;
#ifdef DEBUG
    callcount_setpixel++;
#endif //DEBUG
}

/*int bezier_len(bezier_t b){
   int r = pdist(p.b[0],b.p[1]);
   r += pdist(p.b[1],b.p[2]);
   return(r);
}*/


inline void bezier_setpixel_additive(Bitmap* dest, int x, int y, int color)
{
    uint8_t a = dest->pixels[x+(HEIGHT-y-1)*WIDTH];
    uint8_t b = color;
    // add
	uint8_t halfa=(a>>1)&(PixelAllButHighBits);
    uint8_t halfb=(b>>1)&(PixelAllButHighBits);
	uint8_t carry=a&b&(PixelLowBits);
	dest->pixels[x+(HEIGHT-y-1)*WIDTH] = halfa+halfb+carry + RawRGB(0,0,0);
}    



fillpixel_t* bezier_fill_writepixels(Bitmap *dest, bezier_t bez, fillpixel_t* pixelptr, uint8_t color){
    // TODO: maxpixels
    static bezier_t stack[15];
    int stackpos = 0;
    stack[0] = bez;
    //TODO: draw last point of curve

    while(stackpos >=0){
        bezier_t *b = &stack[stackpos--];
        int len = plen(b->p[0],b->p[1])+plen(b->p[1],b->p[2]);
        if(len>BEZ_SCALEDOWN*2){
            point_t p1, p2, pm, bp2;
            p1 = pavg(b->p[0], b->p[1]);
            p2 = pavg(b->p[1], b->p[2]);
            pm = pavg(p1,p2);
            bezier_t *t;
            t = &stack[++stackpos];
            t->p[0] = b->p[0];
            t->p[1] = p1;
            bp2 = b->p[2]; // rescue value before writing there
            t->p[2] = pm;
            t = &stack[++stackpos];
            t->p[0] = pm;
            t->p[1] = p2;
            t->p[2] = bp2;
        } else {
            static int lastx, lasty;
            bool rising = ( b->p[2].y > (b->p[0].y));
            for(int i=0; i<2; ++i){                
                int x = b->p[i].x / BEZ_SCALEDOWN;
                int y = b->p[i].y / BEZ_SCALEDOWN;
                if(x>=WIDTH || x<0){
                    continue;
                }
                if(y>=HEIGHT|| y<0){
                    continue;
                }
                if(x==lastx && y==lasty){
                    continue;
                }
                lastx = x;  lasty = y;
                pixelptr->y = y; 
                pixelptr->x = x; 
                pixelptr->action = rising ? 0 : 1;
                pixelptr++;
                // paint border pixels
                //bezier_setpixel(dest, b->p[i], color);
                
                bezier_setpixel_additive(dest, x, y, color);
                //dest->pixels[x+(HEIGHT-y-1)*WIDTH] = color;
                if(len<BEZ_SCALEDOWN){
                    break; // don't draw second pixel when bezier is tiny.
                }
            }
//            bezier_setpixel(dest, b->p[0], color);
//            bezier_setpixel(dest, b->p[1], color);
        }
    }
    return pixelptr;
}

static int cmp_fillpixel(const void* p1, const void* p2){
    fillpixel_t* fp1 = (fillpixel_t*)p1;
    fillpixel_t* fp2 = (fillpixel_t*)p2;
    //return ((fp1->y*65536 + fp1->x - 2*fp1->action) - (fp2->y*65536 + fp2->x - 2*fp2->action));
    return ((fp1->y*65536 | fp1->x*4) - 1*fp1->action) - (((fp2->y*65536 | fp2->x*4) - 1*fp2->action));
//    return ((fp1->y*65536 | fp1->x)) - (((fp2->y*65536 | fp2->x)));

//    return *((int32_t*)p1) -  *((int32_t*)p2);
}

void floodfill(Bitmap *dest, fillpixel_t* start, fillpixel_t* end, uint8_t color){
    
    // sort
    qsort(start, end-start, sizeof(fillpixel_t), cmp_fillpixel);

    // sentinel
    end->x=WIDTH;
    end->y=HEIGHT;
    end->action=1; // (the loop will stop/iterate on every endpixel, but not on every startpixel)

    fillpixel_t* pptr = start;
    while((pptr)<=end){
        // require a start followed by end, on the same line
        if( (pptr->action != 0) || ((pptr+1)->action != 1) || (pptr->y != (pptr+1)->y) ){ 
            pptr++;
            continue;
        }
        int xstart = pptr->x + 1; // xstart indexes the first pixel to be touched
        int y = pptr->y;
        pptr++;
        int xend = pptr->x; // the pixel pointed to by xend will not be touched
//        memset(dest->pixels+xstart+(HEIGHT-y-1)*WIDTH, color, xend-xstart);
        if(xend-xstart >= 10){ // quick-and-dirty hack for greet floodfill
            pptr++;
            continue;
        }
        for(int i=xstart; i<xend; i++){
            bezier_setpixel_additive(dest, i, y, color);
   //         dest->pixels[i+(HEIGHT-y-1)*WIDTH] = color; // do floodfill
        }
        if(xend-xstart > 10)
        {
            printf("%i %i %i\n",xstart, xend, y);
        }
        pptr++; 
    }
}
            
        



