
#ifndef __VGA_H__
#define __VGA_H__

#include <stdint.h>

typedef void HBlankInterruptFunction(void);

extern volatile uint32_t VGALine;
extern volatile uint32_t VGAFrame;

void InitializeVGA();

void SetBlankVGAScreenMode480();
void SetBlankVGAScreenMode400();
void SetBlankVGAScreenMode350();

void SetVGAScreenMode240(uint8_t *framebuffer,int pixelsperrow,int pixelclock);
void SetVGAScreenMode200_60Hz(uint8_t *framebuffer,int pixelsperrow,int pixelclock) ;
void SetVGAScreenMode200(uint8_t *framebuffer,int pixelsperrow,int pixelclock) ;
void SetVGAScreenMode175(uint8_t *framebuffer,int pixelsperrow,int pixelclock) ;

void SetVGAScreenMode160(uint8_t *framebuffer,int pixelsperrow,int pixelclock);
void SetVGAScreenMode133_60Hz(uint8_t *framebuffer,int pixelsperrow,int pixelclock);
void SetVGAScreenMode133(uint8_t *framebuffer,int pixelsperrow,int pixelclock);
void SetVGAScreenMode117(uint8_t *framebuffer,int pixelsperrow,int pixelclock);

void SetFrameBuffer(uint8_t *framebuffer);

static inline void SetVGAScreenMode320x240(uint8_t *framebuffer) { SetVGAScreenMode240(framebuffer,320,13); }
static inline void SetVGAScreenMode320x200_60Hz(uint8_t *framebuffer) { SetVGAScreenMode200_60Hz(framebuffer,320,13); }
static inline void SetVGAScreenMode320x200(uint8_t *framebuffer) { SetVGAScreenMode200(framebuffer,320,13); }
static inline void SetVGAScreenMode320x175(uint8_t *framebuffer) { SetVGAScreenMode175(framebuffer,320,13); }

static inline void SetVGAScreenMode212x160(uint8_t *framebuffer) { SetVGAScreenMode160(framebuffer,212,19); }
static inline void SetVGAScreenMode212x133_60Hz(uint8_t *framebuffer) { SetVGAScreenMode133_60Hz(framebuffer,212,19); }
static inline void SetVGAScreenMode212x133(uint8_t *framebuffer) { SetVGAScreenMode133(framebuffer,212,19); }
static inline void SetVGAScreenMode212x175(uint8_t *framebuffer) { SetVGAScreenMode117(framebuffer,212,19); }


static inline uint32_t VGAFrameCounter() { return VGAFrame; }


static inline void WaitVBL() { }
void profiling_startframe() ;
void profiling_endframe(uint8_t* foo) ;




#endif
