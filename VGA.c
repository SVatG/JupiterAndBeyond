#include "VGA.h"
#include "GPIO.h"
#include "RCC.h"

#include <stdlib.h>

static void InitializeState();
static void BlankHSyncHandler480();
static void BlankHSyncHandler400();
static void BlankHSyncHandler350();
static void PixelHSyncHandler240();
static void PixelHSyncHandler200();
static void PixelHSyncHandler175();
static void PixelHSyncHandler160();
static void PixelHSyncHandler133();
static void PixelHSyncHandler117();

static void InitializePixelDMA(int pixelclock,int pixelsperrow);
static void DMACompleteHandler();
static inline void StartPixelDMA();
static inline void StopPixelDMA();

uint32_t VGALine;
volatile uint32_t VGAFrame;
uint32_t VGAFrameBufferAddress;
uint32_t VGACurrentLineAddress;
uint32_t VGAPixelsPerRow;
uint16_t VGAThreeLineCounter;

static HBlankInterruptFunction *HBlankInterruptHandler;

void InitializeVGAScreenMode480()
{
	HBlankInterruptHandler=NULL;
	InitializeVGAPort();
	InitializeState();
	SetBlankVGAScreenMode480();
}

void InitializeVGAScreenMode400()
{
	HBlankInterruptHandler=NULL;
	InitializeVGAPort();
	InitializeState();
	SetBlankVGAScreenMode400();
}

void InitializeVGAScreenMode350()
{
	HBlankInterruptHandler=NULL;
	InitializeVGAPort();
	InitializeState();
	SetBlankVGAScreenMode350();
}

void SetBlankVGAScreenMode480()
{
	SetVGASignalToBlack();
	InitializeVGAHorizontalSync31kHz(BlankHSyncHandler480);
}

void SetBlankVGAScreenMode400()
{
	SetVGASignalToBlack();
	InitializeVGAHorizontalSync31kHz(BlankHSyncHandler400);
}

void SetBlankVGAScreenMode350()
{
	SetVGASignalToBlack();
	InitializeVGAHorizontalSync31kHz(BlankHSyncHandler350);
}

void SetVGAScreenMode240(uint8_t *framebuffer,int pixelsperrow,int pixelclock)
{
	WaitVBL();
	InitializePixelDMA(pixelclock,pixelsperrow);
	SetFrameBuffer(framebuffer);
	SetVGAHorizontalSync31kHz(PixelHSyncHandler240);
}

void SetVGAScreenMode200(uint8_t *framebuffer,int pixelsperrow,int pixelclock)
{
	WaitVBL();
	InitializePixelDMA(pixelclock,pixelsperrow);
	SetFrameBuffer(framebuffer);
	SetVGAHorizontalSync31kHz(PixelHSyncHandler200);
}

void SetVGAScreenMode175(uint8_t *framebuffer,int pixelsperrow,int pixelclock)
{
	WaitVBL();
	InitializePixelDMA(pixelclock,pixelsperrow);
	SetFrameBuffer(framebuffer);
	SetVGAHorizontalSync31kHz(PixelHSyncHandler175);
}

void SetVGAScreenMode160(uint8_t *framebuffer,int pixelsperrow,int pixelclock)
{
	WaitVBL();
	InitializePixelDMA(pixelclock,pixelsperrow);
	SetFrameBuffer(framebuffer);
	VGAThreeLineCounter=0;
	SetVGAHorizontalSync31kHz(PixelHSyncHandler160);
}

void SetVGAScreenMode133(uint8_t *framebuffer,int pixelsperrow,int pixelclock)
{
	WaitVBL();
	InitializePixelDMA(pixelclock,pixelsperrow);
	SetFrameBuffer(framebuffer);
	VGAThreeLineCounter=0;
	SetVGAHorizontalSync31kHz(PixelHSyncHandler133);
}

void SetVGAScreenMode117(uint8_t *framebuffer,int pixelsperrow,int pixelclock)
{
	WaitVBL();
	InitializePixelDMA(pixelclock,pixelsperrow);
	SetFrameBuffer(framebuffer);
	VGAThreeLineCounter=0;
	SetVGAHorizontalSync31kHz(PixelHSyncHandler117);
}




void InitializeVGAPort()
{
	// Turn on peripherals.
	EnableAHB1PeripheralClock(RCC_AHB1ENR_GPIOBEN|RCC_AHB1ENR_GPIOEEN|RCC_AHB1ENR_DMA2EN);
	EnableAPB2PeripheralClock(RCC_APB2ENR_TIM8EN|RCC_APB2ENR_TIM9EN|RCC_APB2ENR_SYSCFGEN);

	// Configure DAC pins, and set to black.
	SetGPIOOutputMode(GPIOE,0xff00);
	SetGPIOPushPullOutput(GPIOE,0xff00);
	SetGPIOSpeed50MHz(GPIOE,0xff00);
	SetGPIOPullUpResistor(GPIOE,0xff00);
	GPIOE->BSRRH=0xff00;

	// Configure sync pins and drive them high.
	SetGPIOOutputMode(GPIOB,(1<<11)|(1<<12));
	SetGPIOPushPullOutput(GPIOB,(1<<11)|(1<<12));
	SetGPIOSpeed50MHz(GPIOB,(1<<11)|(1<<12));
	SetGPIOPullUpResistor(GPIOB,(1<<11)|(1<<12));
	GPIOB->BSRRL=(1<<11)|(1<<12);
}

void InitializeVGAHorizontalSync31kHz(InterruptHandler *handler)
{
	// Configure timer 9 as the HSync timer.
	TIM9->CR1=TIM_CR1_ARPE;
	TIM9->DIER=TIM_DIER_UIE|TIM_DIER_CC1IE|TIM_DIER_CC2IE; // Enable update, compare 1 and 2 interrupts.
	TIM9->CCMR1=0;
	TIM9->CCER=0;
	TIM9->PSC=0; // Prescaler = 1
	TIM9->ARR=5337; // 168 MHz / 31.4686 kHz = 5338.65504
	TIM9->CCR1=633; // 168 MHz * 3.77 microseconds = 633.36 - sync pulse end
	TIM9->CCR2=950; // 168 MHz * (3.77 + 1.89) microseconds = 950.88 - back porch end

	SetVGAHorizontalSync31kHz(handler);

	// Enable HSync timer.
	TIM9->CR1|=TIM_CR1_CEN;
}

void SetVGAHorizontalSync31kHz(InterruptHandler *handler)
{
	// Enable HSync timer interrupt and set highest priority.
	InstallInterruptHandler(TIM1_BRK_TIM9_IRQn,handler);
	EnableInterrupt(TIM1_BRK_TIM9_IRQn);
	SetInterruptPriority(TIM1_BRK_TIM9_IRQn,0);
}

void SetHBlankInterruptHandler(HBlankInterruptFunction *handler)
{
	HBlankInterruptHandler=handler;
}




static void InitializeState(uint8_t *framebuffer,int pixelsperrow)
{
	VGALine=0xffffffff;
	VGAFrame=0;
}

static void BlankHSyncHandler480()
{
	HandleVGAHSync480();
}

static void BlankHSyncHandler400()
{
	HandleVGAHSync400();
}

static void BlankHSyncHandler350()
{
	HandleVGAHSync350();
}

static void PixelHSyncHandler240()
{
	int line=HandleVGAHSync480();
	if(line<0) return;

	StartPixelDMA();
	if(line&1) VGACurrentLineAddress+=VGAPixelsPerRow;
}

static void PixelHSyncHandler200()
{
	int line=HandleVGAHSync400();
	if(line<0) return;

	StartPixelDMA();
	if(line&1) VGACurrentLineAddress+=VGAPixelsPerRow;
}

static void PixelHSyncHandler175()
{
	int line=HandleVGAHSync350();
	if(line<0) return;

	StartPixelDMA();
	if(line&1) VGACurrentLineAddress+=VGAPixelsPerRow;
}



static void PixelHSyncHandler160()
{
	int line=HandleVGAHSync480();
	if(line<0) return;

	StartPixelDMA();
	if(VGAThreeLineCounter++==2)
	{
		VGACurrentLineAddress+=VGAPixelsPerRow;
		VGAThreeLineCounter=0;
	}
}

static void PixelHSyncHandler133()
{
	int line=HandleVGAHSync400();
	if(line<0) return;

	StartPixelDMA();
	if(VGAThreeLineCounter++==2)
	{
		VGACurrentLineAddress+=VGAPixelsPerRow;
		VGAThreeLineCounter=0;
	}
}

static void PixelHSyncHandler117()
{
	int line=HandleVGAHSync350();
	if(line<0) return;

	StartPixelDMA();
	if(VGAThreeLineCounter++==2)
	{
		VGACurrentLineAddress+=VGAPixelsPerRow;
		VGAThreeLineCounter=0;
	}
}




static void InitializePixelDMA(int pixelclock,int pixelsperrow)
{
	// Configure timer 8 as the pixel clock.
	TIM8->CR1=TIM_CR1_ARPE;
	TIM8->DIER=TIM_DIER_UDE; // Enable update DMA request.
	TIM8->PSC=0; // Prescaler = 1
	TIM8->ARR=pixelclock-1; // TODO: Should this be -1?

	// DMA2 stream 1 channel 7 is triggered by timer 8.
	// Stop it and configure interrupts.
	DMA2_Stream1->CR&=~DMA_SxCR_EN;
	InstallInterruptHandler(DMA2_Stream1_IRQn,DMACompleteHandler);
	EnableInterrupt(DMA2_Stream1_IRQn);
	SetInterruptPriority(DMA2_Stream1_IRQn,0);

	VGAPixelsPerRow=pixelsperrow;
}

static void DMACompleteHandler()
{
	SetVGASignalToBlack();
	DMA2->LIFCR|=DMA_LIFCR_CTCIF1; // Clear interrupt flag.
	StopPixelDMA();

	if(HBlankInterruptHandler) HBlankInterruptHandler();
	// TODO: VBlank interrupt? At lower priority?
}

static inline void StartPixelDMA()
{
	// Visible line. Configure and enable pixel DMA.
	DMA2_Stream1->CR=(7*DMA_SxCR_CHSEL_0)| // Channel 7
	(3*DMA_SxCR_PL_0)| // Priority 3
	(0*DMA_SxCR_PSIZE_0)| // PSIZE = 8 bit
	(0*DMA_SxCR_MSIZE_0)| // MSIZE = 8 bit
	DMA_SxCR_MINC| // Increase memory address
	(1*DMA_SxCR_DIR_0)| // Memory to peripheral
	DMA_SxCR_TCIE; // Transfer complete interrupt
	DMA2_Stream1->NDTR=VGAPixelsPerRow;
	DMA2_Stream1->PAR=((uint32_t)&GPIOE->ODR)+1;
	DMA2_Stream1->M0AR=VGACurrentLineAddress;
	DMA2_Stream1->CR|=DMA_SxCR_EN;

	// Start pixel clock.
    TIM8->CNT = 0;
	TIM8->CR1|=TIM_CR1_CEN;
}

static inline void StopPixelDMA()
{
	TIM8->CR1&=~TIM_CR1_CEN; // Stop pixel clock.
	DMA2_Stream1->CR=0; // Disable pixel DMA.
}

