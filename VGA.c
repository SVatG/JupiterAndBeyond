#include "VGA.h"
#include "GPIO.h"
#include "RCC.h"
#include "System.h"

#include "stm32f4xx.h"

static uint32_t Line;
static volatile uint32_t Frame;
static uint32_t FrameBufferAddress;
static uint32_t CurrentLineAddress;

static void HSYNCHandler();
static void DMACompleteHandler();

void InitializeVGA(uint8_t *framebuffer)
{
	// Initialize state.
	Line=0;
	Frame=0;
	FrameBufferAddress=(uint32_t)framebuffer;
	CurrentLineAddress=FrameBufferAddress;

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

	// Configure timer 8 as the pixel clock.
	TIM8->CR1=TIM_CR1_ARPE;
	TIM8->DIER=TIM_DIER_UDE; // Enable update DMA request.
	TIM8->PSC=0; // Prescaler = 1
	TIM8->ARR=12; // 13 counts per cycle.

	// Stop DMA 2 stream 1 and wait for it to become available.

	// DMA2 stream 1 channel 7 is triggered by timer 8. Stop it and configure interrupts.
	DMA2_Stream1->CR&=~DMA_SxCR_EN;
	DMA2->LISR|=DMA_LISR_TCIF1; // Enable transfer complete interrupt on DMA2 stream 1.
	InstallInterruptHandler(DMA2_Stream1_IRQn,DMACompleteHandler);
	EnableInterrupt(DMA2_Stream1_IRQn);

	// Configure timer 9 as the HSYNC timer.
	TIM9->CR1=TIM_CR1_ARPE;
	TIM9->DIER=TIM_DIER_UIE|TIM_DIER_CC1IE|TIM_DIER_CC2IE; // Enable update, compare 1 and 2 interrupts.
	TIM9->CCMR1=0;
	TIM9->CCER=0;
	TIM9->PSC=0; // Prescaler = 1
	TIM9->ARR=5337; // 168 MHz / 31.4686 kHz = 5338.65504
	TIM9->CCR1=633; // 168 MHz * 3.77 microseconds = 633.36 - sync pulse end
	TIM9->CCR2=950; // 168 MHz * (3.77 + 1.89) microseconds = 950.88 - back porch end

	// Enable HSYNC timer interrupt.
	InstallInterruptHandler(TIM1_BRK_TIM9_IRQn,HSYNCHandler);
	EnableInterrupt(TIM1_BRK_TIM9_IRQn);
	// TODO: Set high priority.

	// Enable HSYNC timer.
	TIM9->CR1|=TIM_CR1_CEN;
}

static void HSYNCHandler()
{
	uint32_t sr=TIM9->SR;
	TIM9->SR=0;

	if(sr&1) // Overflow interrupt - start of sync pulse
	{
		GPIOE->BSRRH=0xff00; // Set signal to black.
		GPIOB->BSRRH=(1<<11); // Lower HSYNC.
	}
	else if(sr&2) // Output compare 1 interrupt - end of sync pulse
	{
		GPIOB->BSRRL=(1<<11); // Raise HSYNC.
	}
	else if(sr&4) // Output compare 2 interrupt - start of video data
	{
		if(Line<480)
		{
			// Visible line. Configure and enable pixel DMA.
			DMA2_Stream1->CR=(7*DMA_SxCR_CHSEL_0)| // Channel 7
			(3*DMA_SxCR_PL_0)| // Priority 3
			(0*DMA_SxCR_PSIZE_0)| // PSIZE = 8 bit
			(0*DMA_SxCR_MSIZE_0)| // MSIZE = 8 bit
			DMA_SxCR_MINC| // Increase memory address
			(1*DMA_SxCR_DIR_0)| // Memory to peripheral
			DMA_SxCR_TCIE; // Transfer complete interrupt
			DMA2_Stream1->NDTR=320;
			DMA2_Stream1->PAR=((uint32_t)&GPIOE->ODR)+1;
			DMA2_Stream1->M0AR=CurrentLineAddress;
			DMA2_Stream1->CR|=DMA_SxCR_EN;

			// Start pixel clock.
			TIM8->CR1|=TIM_CR1_CEN;

			if(Line&1) CurrentLineAddress+=320;
		}

		Line++;
		if(Line==481)
		{
			Frame++;
			// TODO: VBlank interrupt.
		}
		else if(Line==492)
		{
			GPIOB->BSRRH=(1<<12); // Lower VSYNC.
		}
		else if(Line==494)
		{		
			GPIOB->BSRRL=(1<<12); // Raise VSYNC.
		}
		else if(Line==525)
		{
			Line=0;
			CurrentLineAddress=FrameBufferAddress;
		}
	}
}

static void DMACompleteHandler()
{
	GPIOE->BSRRH=0xff00; // Set signal to black.
	DMA2->LIFCR|=DMA_LIFCR_CTCIF1; // Clear interrupt flag.
	TIM8->CR1&=~TIM_CR1_CEN; // Stop pixel clock.
	DMA2_Stream1->CR&=~DMA_SxCR_EN; // Disable pixel DMA.

	// TODO: HBlank interrupt happens here.
}



void WaitVBL()
{
	uint32_t currframe=Frame;
	while(Frame==currframe);
}
