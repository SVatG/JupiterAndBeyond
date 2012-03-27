#ifndef __VGA_H__
#define __VGA_H__

#include "System.h"
#include "stm32f4xx.h"

#include <stdint.h>

typedef void HBlankInterruptFunction(void);

void InitializeVGAScreenMode240(uint8_t *framebuffer,int pixelsperrow,int pixelclock);
void InitializeVGAScreenMode200(uint8_t *framebuffer,int pixelsperrow,int pixelclock);
void InitializeVGAScreenMode175(uint8_t *framebuffer,int pixelsperrow,int pixelclock);

void WaitVBL();
void SetFrameBuffer(uint8_t *framebuffer); // Only safe to call in the vertical blank!

static inline void IntializeVGAScreenMode320x240(uint8_t *framebuffer)
{
	InitializeVGAScreenMode240(framebuffer,320,13);
}

static inline void IntializeVGAScreenMode320x200(uint8_t *framebuffer)
{
	InitializeVGAScreenMode200(framebuffer,320,13);
}

static inline void IntializeVGAScreenMode320x175(uint8_t *framebuffer)
{
	InitializeVGAScreenMode175(framebuffer,320,13);
}



// Functions for implementing your own video driving code.

void InitializeVGAPort();
void InitializeVGAHorizontalSync31kHz(InterruptHandler *handler);

#define VGAHorizontalSyncStartInterrupt 1 // Overflow interrupt
#define VGAHorizontalSyncEndInterrupt 2 // Output compare 1 interrupt
#define VGAVideoStartInterrupt 4 // Output compare 2 interrupt

static inline uint32_t VGAHorizontalSyncInterruptType()
{
	uint32_t sr=TIM9->SR;
	TIM9->SR=0;
	return sr;
}

static inline void RaiseVGAHSYNCLine() { GPIOB->BSRRL=(1<<11); }
static inline void LowerVGAHSYNCLine() { GPIOB->BSRRH=(1<<11); }
static inline void RaiseVGAVSYNCLine() { GPIOB->BSRRL=(1<<12); }
static inline void LowerVGAVSYNCLine() { GPIOB->BSRRH=(1<<12); }

#endif
