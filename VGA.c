#include "VGA.h"
#include "Bits.h"

#include <stm32f4xx.h>

void InitializeVGA()
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2RSTR_TIM9RST,ENABLE);

	// Configure DAC pins.
	GPIOE->MODER=SetDoubleBits(GPIOE->MODER,0xff00,GPIO_Mode_OUT);
	GPIOE->OSPEEDR=SetDoubleBits(GPIOE->OSPEEDR,0xff00,GPIO_Speed_50MHz);
	GPIOE->OTYPER=SetBits(GPIOE->OTYPER,0xff00,GPIO_OType_PP);
	GPIOE->PUPDR=SetDoubleBits(GPIOE->PUPDR,0xff00,GPIO_PuPd_UP);

	GPIOE->BSRRH=0xff00;

	// Configure sync pins.
	GPIOB->MODER=SetDoubleBits(GPIOB->MODER,(1<<11)|(1<<12),GPIO_Mode_OUT);
	GPIOB->OSPEEDR=SetDoubleBits(GPIOB->OSPEEDR,(1<<11)|(1<<12),GPIO_Speed_50MHz);
	GPIOB->OTYPER=SetBits(GPIOB->OTYPER,(1<<11)|(1<<12),GPIO_OType_PP);
	GPIOB->PUPDR=SetDoubleBits(GPIOB->PUPDR,(1<<11)|(1<<12),GPIO_PuPd_UP);

	GPIOB->BSRRL=(1<<11)|(1<<12);

	// Configure timer.
	TIM9->CR1=TIM_CR1_ARPE;
	TIM9->DIER=TIM_DIER_UIE|TIM_DIER_CC1IE|TIM_DIER_CC2IE;
	TIM9->CCMR1=0;
	TIM9->CCER=0;
	TIM9->PSC=0;
	TIM9->ARR=5337; // 168 MHz/31.4686 kHz = 5338.65504
	TIM9->CCR1=633; // 168 MHz * 3.77 microseconds = 633.36
	TIM9->CCR2=950; // 168 MHz * (3.77 + 1.89) microseconds = 950.88

	// Enable timer 9 interrupt.
    NVIC->ISER[0]|=1<<24;

	// Enable timer.
	TIM9->CR1|=TIM_CR1_CEN;
}

static uint32_t line=0;
static uint32_t t=0;
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
		GPIOB->BSRRL=(1<<11);

		if(line<480)
		{
			for(int x=0;x<354;x++)
			GPIOE->ODR=(GPIOE->ODR&0xff)|(((x^line)+t)<<8);
		}
		else
		{
			GPIOE->BSRRH=0xff00; // Set signal to black.
		}

		line++;
		if(line==492) GPIOB->BSRRH=(1<<12); // Lower VSYNC.
		else if(line==494) GPIOB->BSRRL=(1<<12); // Raise VSYNC.
		else if(line==525) {line=0;t++;}
	}
}

