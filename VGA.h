#ifndef __VGA_H__
#define __VGA_H__

#include "System.h"

#include <stdint.h>

void InitializeVGAScreenMode240(uint8_t *framebuffer,int pixelsperrow,int pixelclock);
void InitializeVGAScreenMode200(uint8_t *framebuffer,int pixelsperrow,int pixelclock);
void InitializeVGAScreenMode175(uint8_t *framebuffer,int pixelsperrow,int pixelclock);

void InitializeVGAPort();
void InitializeVGAHorizontalSync31kHz(InterruptHandler *handler);

void WaitVBL();

static inline void IntializeVGAScreenMode320x240(uint8_t *framebuffer)
{
	InitializeVGAScreenMode240(framebuffer,320,13);
}

#endif
