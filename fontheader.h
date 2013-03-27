#ifndef __FONTHEADER_H_
#define __FONTHEADER_H_

//#define GLYPHGROUPS 32
#define GLYPHGROUPS 64

#include <stdint.h>
/*
typedef struct {
    uint16_t x;
    uint16_t y;
} point_t_base;

typedef union {
    uint32_t i;
//    point_t_base p;
    struct {
        uint16_t x;
        uint16_t y;
    };
} point_t;
*/
typedef struct {
    int16_t x;
    int16_t y;
} point_t;

typedef union {
    int16_t i[6];
    point_t p[3];
} bezier_t;

typedef struct {
    int16_t x;
    int16_t y;
    int8_t action;
    int8_t pixelcolor;
    int8_t fillcolor;
} fillpixel_t;
/*
 * actions:
 * 0 start fill
 * 1 end fill
 *
 */


typedef struct {
    unsigned char c;
    int width;
    const bezier_t* data;
    int datalen;
    uint16_t groupstart[GLYPHGROUPS];
    uint16_t grouplen[GLYPHGROUPS];
    int pathlen[GLYPHGROUPS];
} glyph_t;

typedef struct {
    const bezier_t* data;
    uint8_t datalen; 
} path_t;





#endif //__FONTHEADER_H_
