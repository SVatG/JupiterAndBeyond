#ifndef __VGA_H__
#define __VGA_H__

#include "System.h"
#include "stm32f4xx.h"

#include <stdint.h>

typedef void HBlankInterruptFunction(void);

extern uint32_t VGALine;
extern volatile uint32_t VGAFrame;
extern uint32_t VGAFrameBufferAddress;
extern uint32_t VGACurrentLineAddress;
extern uint32_t VGAPixelsPerRow;

void InitializeVGAScreenMode480();
void InitializeVGAScreenMode400();
void InitializeVGAScreenMode350();

void SetBlankVGAScreenMode480();
void SetBlankVGAScreenMode400();
void SetBlankVGAScreenMode350();

void SetVGAScreenMode240(uint8_t *framebuffer,int pixelsperrow,int pixelclock);
void SetVGAScreenMode200(uint8_t *framebuffer,int pixelsperrow,int pixelclock);
void SetVGAScreenMode175(uint8_t *framebuffer,int pixelsperrow,int pixelclock);

void SetVGAScreenMode160(uint8_t *framebuffer,int pixelsperrow,int pixelclock);
void SetVGAScreenMode133(uint8_t *framebuffer,int pixelsperrow,int pixelclock);
void SetVGAScreenMode117(uint8_t *framebuffer,int pixelsperrow,int pixelclock);

static inline void SetVGAScreenMode320x240(uint8_t *framebuffer) { SetVGAScreenMode240(framebuffer,320,13); }
static inline void SetVGAScreenMode320x200(uint8_t *framebuffer) { SetVGAScreenMode200(framebuffer,320,13); }
static inline void SetVGAScreenMode320x175(uint8_t *framebuffer) { SetVGAScreenMode175(framebuffer,320,13); }

static inline void SetVGAScreenMode212x160(uint8_t *framebuffer) { SetVGAScreenMode160(framebuffer,212,19); }
static inline void SetVGAScreenMode212x133(uint8_t *framebuffer) { SetVGAScreenMode133(framebuffer,212,19); }
static inline void SetVGAScreenMode212x175(uint8_t *framebuffer) { SetVGAScreenMode117(framebuffer,212,19); }

static inline void WaitVBL()
{
	uint32_t currframe=VGAFrame;
	while(VGAFrame==currframe);
}

static inline void SetFrameBuffer(uint8_t *framebuffer)
{
	VGAFrameBufferAddress=VGACurrentLineAddress=(uint32_t)framebuffer;
}

static inline uint32_t VGAFrameCounter() { return VGAFrame; }



// Functions for implementing your own video driving code.

void InitializeVGAPort();
void InitializeVGAHorizontalSync31kHz(InterruptHandler *handler);
void SetVGAHorizontalSync31kHz(InterruptHandler *handler);

#define VGAHorizontalSyncStartInterruptFlag 1 // Overflow interrupt
#define VGAHorizontalSyncEndInterruptFlag 2 // Output compare 1 interrupt
#define VGAVideoStartInterruptFlag 4 // Output compare 2 interrupt

static inline void RaiseVGAHSyncLine() { GPIOB->BSRRL=(1<<11); }
static inline void LowerVGAHSyncLine() { GPIOB->BSRRH=(1<<11); }
static inline void RaiseVGAVSyncLine() { GPIOB->BSRRL=(1<<12); }
static inline void LowerVGAVSyncLine() { GPIOB->BSRRH=(1<<12); }

static inline void SetVGASignalToBlack() { GPIOE->BSRRH=0xff00; }
static inline void SetVGASignal(uint8_t pixel) { ((uint8_t *)&GPIOE->ODR)[1]=pixel; }

static inline int HandleVGAHSync480()
{
	uint32_t sr=TIM9->SR;
	TIM9->SR=0;

	if(sr==VGAHorizontalSyncStartInterruptFlag) LowerVGAHSyncLine();
	else if(sr==VGAHorizontalSyncEndInterruptFlag) RaiseVGAHSyncLine();
	else // if(VGAVideoStartInterruptFlag)
	{
		VGALine++;
		if(VGALine<480)
		{
			return VGALine;
		}
		else if(VGALine==480)
		{
			VGAFrame++;
		}
		else if(VGALine==490)
		{
			LowerVGAVSyncLine();
		}
		else if(VGALine==492)
		{
			RaiseVGAVSyncLine();
		}
		else if(VGALine==524)
		{
			VGALine=0;
			VGACurrentLineAddress=VGAFrameBufferAddress;
		}
	}
	return -1;
}

static inline int HandleVGAHSync400()
{
	uint32_t sr=TIM9->SR;
	TIM9->SR=0;

	if(sr==VGAHorizontalSyncStartInterruptFlag) LowerVGAHSyncLine();
	else if(sr==VGAHorizontalSyncEndInterruptFlag) RaiseVGAHSyncLine();
	else // if(VGAVideoStartInterruptFlag)
	{
		VGALine++;
		if(VGALine<400)
		{
			return VGALine;
		}
		else if(VGALine==400)
		{
			VGAFrame++;
		}
		else if(VGALine==412)
		{
			RaiseVGAVSyncLine();
		}
		else if(VGALine==414)
		{
			LowerVGAVSyncLine();
		}
		else if(VGALine==448)
		{
			VGALine=0;
			VGACurrentLineAddress=VGAFrameBufferAddress;
		}
	}
	return -1;
}

static inline int HandleVGAHSync350()
{
	uint32_t sr=TIM9->SR;
	TIM9->SR=0;

	if(sr==VGAHorizontalSyncStartInterruptFlag) RaiseVGAHSyncLine();
	else if(sr==VGAHorizontalSyncEndInterruptFlag) LowerVGAHSyncLine();
	else // if(VGAVideoStartInterruptFlag)
	{
		VGALine++;
		if(VGALine<350)
		{
			return VGALine;
		}
		else if(VGALine==350)
		{
			VGAFrame++;
		}
		else if(VGALine==387)
		{
			LowerVGAVSyncLine();
		}
		else if(VGALine==389)
		{
			RaiseVGAVSyncLine();
		}
		else if(VGALine==448)
		{
			VGALine=0;
			VGACurrentLineAddress=VGAFrameBufferAddress;
		}
	}
	return -1;
}

#endif
