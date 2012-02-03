#include "VGA.h"
#include "Bits.h"

#include <stm32f4xx.h>

static uint32_t Line;
static volatile uint32_t Frame;
static uint32_t FrameBufferAddress;
static uint32_t CurrentLineAddress;

void InitializeVGA(uint8_t *framebuffer)
{
	// Initialize state.
	Line=0;
	Frame=0;
	FrameBufferAddress=(uint32_t)framebuffer;
	CurrentLineAddress=FrameBufferAddress;

	// Turn on peripherals.
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB|RCC_AHB1Periph_GPIOE|RCC_AHB1ENR_DMA2EN,ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1ENR_DMA2EN,ENABLE); // TODO: Remove
	RCC_APB2PeriphClockCmd(RCC_APB2RSTR_TIM8RST,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2RSTR_TIM9RST,ENABLE);

	// Configure DAC pins, and set to black.
	GPIOE->MODER=SetDoubleBits(GPIOE->MODER,0xff00,GPIO_Mode_OUT);
	GPIOE->OSPEEDR=SetDoubleBits(GPIOE->OSPEEDR,0xff00,GPIO_Speed_50MHz);
	GPIOE->OTYPER=SetBits(GPIOE->OTYPER,0xff00,GPIO_OType_PP);
	GPIOE->PUPDR=SetDoubleBits(GPIOE->PUPDR,0xff00,GPIO_PuPd_UP);
	GPIOE->BSRRH=0xff00;

	// Configure sync pins and drive them high.
	GPIOB->MODER=SetDoubleBits(GPIOB->MODER,(1<<11)|(1<<12),GPIO_Mode_OUT);
	GPIOB->OSPEEDR=SetDoubleBits(GPIOB->OSPEEDR,(1<<11)|(1<<12),GPIO_Speed_50MHz);
	GPIOB->OTYPER=SetBits(GPIOB->OTYPER,(1<<11)|(1<<12),GPIO_OType_PP);
	GPIOB->PUPDR=SetDoubleBits(GPIOB->PUPDR,(1<<11)|(1<<12),GPIO_PuPd_UP);
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
	NVIC->ISER[1]|=1<<(57-32);

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
	NVIC->ISER[0]|=1<<24;
	// TODO: Set high priority.

	// Enable HSYNC timer.
	TIM9->CR1|=TIM_CR1_CEN;
}

void TIM1_BRK_TIM9_IRQHandler()
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

void WaitVBL()
{
	uint32_t currframe=Frame;
	while(Frame==currframe);
}

void DMA2_Stream1_IRQHandler()
{
	GPIOE->BSRRH=0xff00; // Set signal to black.
	DMA2->LIFCR|=DMA_LIFCR_CTCIF1; // Clear interrupt flag.
	TIM8->CR1&=~TIM_CR1_CEN; // Stop pixel clock.
	DMA2_Stream1->CR&=~DMA_SxCR_EN; // Disable pixel DMA.

	// TODO: HBlank interrupt happens here.
}
