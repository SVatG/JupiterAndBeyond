
#include "warpfunc.h"
#include "2dvector.h"



point_t warp_nop(point_t a, int t)
{   // no transformation
    return a;
}

point_t warp_up_away(point_t a, int t){
    int x = a.x;
    int y = a.y;
    point_t r;

    y += (BEZ_SCALEDOWN/10 * 5) * t;
    x = (x-BEZ_SCALEDOWN*160) * (500 - y/BEZ_SCALEDOWN) / 500;
    y = y * (1000 - y/BEZ_SCALEDOWN) / 1000;
    x += BEZ_SCALEDOWN*160;
//    x += (BEZ_SCALEDOWN/10 * 2) * t;
    r.x=x; r.y=y; return r;
}
//#define BEZ_SCALEDOWN 1

point_t warp_perspective(point_t a, int t){    
    int x = a.x;
    int y = a.y;
    point_t r;
    point_t A = {0*BEZ_SCALEDOWN,0*BEZ_SCALEDOWN+t*400};
    point_t C = {30*BEZ_SCALEDOWN+t*50,180*BEZ_SCALEDOWN+t*200};
    point_t B = {320*BEZ_SCALEDOWN,10*BEZ_SCALEDOWN+t*420};
    point_t D = {(320-40)*BEZ_SCALEDOWN-t*80,200*BEZ_SCALEDOWN+t*220};

    point_t Z = psub(padd(A,D),padd(B,C));
    point_t dX = psub(B,A);
    point_t dY = psub(C,A);

    int xmax = BEZ_SCALEDOWN*320;
    int ymax = BEZ_SCALEDOWN*200;
    int xyscale = 4096; // NOT LARGER, or else it overflows int32_t
    int xyscale_log2 = 12;

    // scale x and y
    x = x * xyscale / xmax;
    y = y * xyscale / ymax;
    // quickfix
//    x += 2000;
    // calc xy(and scale)
    int xy = x * y / xyscale;

    r = pscale(Z, xy, xyscale_log2);
    r = padd(r, pscale(dX, x, xyscale_log2));
    r = padd(r, pscale(dY, y, xyscale_log2));
    r = padd(r, A);

    return r;

}

point_t warp_perspective_1(point_t a, int t){
    return warp_perspective_generic(a, t, 
            (int[]){0,0,    320,10,     30, 180,    280,200}, 
            (int[]){0,400,  0,420,      50, 200,    -80,220});
}

point_t warp_perspective_2(point_t a, int t){ // fast drift left
    return warp_perspective_generic(a, t, 
            (int[]){0,10,    320,0,     40, 200,    290,180}, 
            (int[]){-1000,400,  -800,420,      -1050, 200,    -880,220});
}

point_t warp_perspective_3(point_t a, int t){ // upside-down
    return warp_perspective_generic(a, t, 
            (int[]){280,200,30,180,     320,10,    0,0}, 
            (int[]){0,400,  0,420,      50, 200,    -80,220});
}

point_t warp_perspective_rotright(point_t a, int t){ // rotate right
//    t = (t-30)*2; //FIXME: reenable
    return warp_perspective_generic(a, t, 
            (int[]){0,0,    320,30,     30, 180,    290,200}, 
            (int[]){-500,700,  -500,-300,      400, 600,    400,-200});
}

point_t warp_perspective_4(point_t a, int t){
    return warp_perspective_generic(a, t, 
            (int[]){0,10,    320,0,     40, 200,    290,180}, 
            (int[]){0,400,  0,420,      50, 200,    -80,220});
}
