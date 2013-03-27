#ifndef __FONT_H_
#define __FONT_H_

#include "fontheader.h"
#include "warpfunc.h"

#include "Graphics/Bitmap.h"

#define FONT_SIZE_LOG2 7

void render_text(Bitmap *dest, char* text, point_t pos, int size, const glyph_t* font);
int render_text_getbeziers(bezier_t *dest, int destsize, char* text, point_t pos, int size, const glyph_t* font);
void render_text_partial(Bitmap *dest, char* text, point_t pos, int size, const glyph_t* font, int percent);

void get_text_points(point_t *dest, int *destsize, char* text, point_t pos, int size, const glyph_t* font, bool first);

void render_text_partial_warped(Bitmap *dest, char* text, point_t pos, int size, const glyph_t* font, int percent, warpfunc_t warpfunc, int time);
void render_text_warped(Bitmap *dest, char* text, point_t pos, int size, const glyph_t* font, warpfunc_t warpfunc, int t);
void get_text_points_warped(point_t *dest, int *destsize, char* text, point_t pos, int size, const glyph_t* font, bool first, warpfunc_t warpfunc, int time);

void make_transition(Bitmap *dest, point_t *startpts, int startsize, point_t *endpts, int endsize, int percent);
//void text_to_paths(path_t *dest, char* text, point_t pos, int size, const glyph_t* font);
//
#endif //__FONT_H_
