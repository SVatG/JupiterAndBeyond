#ifndef __WARPFUNC_H_
#define __WARPFUNC_H_


#include "bezier.h"
#include "2dvector.h"

typedef point_t (*warpfunc_t)(point_t, int);


point_t warp_up_away(point_t a, int t);
point_t warp_perspective(point_t a, int t);


point_t warp_perspective_1(point_t a, int t);
point_t warp_perspective_2(point_t a, int t);
point_t warp_perspective_3(point_t a, int t);
point_t warp_perspective_4(point_t a, int t);
point_t warp_perspective_rotright(point_t a, int t);




static inline point_t warp_perspective_generic(point_t a, int t, int* coords, int* deltas){
    point_t A, B, C, D;
    point_t dA, dB, dC, dD;
    A.x = coords[0]; A.y = coords[1];
    B.x = coords[2]; B.y = coords[3];
    C.x = coords[4]; C.y = coords[5];
    D.x = coords[6]; D.y = coords[7];
    dA.x = deltas[0]; dA.y = deltas[1];
    dB.x = deltas[2]; dB.y = deltas[3];
    dC.x = deltas[4]; dC.y = deltas[5];
    dD.x = deltas[6]; dD.y = deltas[7];
//inline point_t warp_perspective_generic(point_t a, int t, point_t A, point_t B, point_t C, point_t D,
//        point_t dA, point_t dB, point_t dC, point_t dD){    
    int x = a.x;
    int y = a.y;
    point_t r;
    // scale by BEZ_SCALEDOWN, so that the user can enter points in screen coordinates
    A = pmul(A, BEZ_SCALEDOWN);
    B = pmul(B, BEZ_SCALEDOWN);
    C = pmul(C, BEZ_SCALEDOWN);
    D = pmul(D, BEZ_SCALEDOWN);
    // apply t-movement
    A = padd(pmul(dA,t),A);
    B = padd(pmul(dB,t),B);
    C = padd(pmul(dC,t),C);
    D = padd(pmul(dD,t),D);

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



#endif //__WARPFUNC_H_
