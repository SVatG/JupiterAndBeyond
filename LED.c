#include "LED.h"
#include "Bits.h"

void InitializeLEDs()
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD,ENABLE);

	GPIOD->MODER=SetDoubleBits(GPIOD->MODER,(1<<12)|(1<<13)|(1<<14)|(1<<15),GPIO_Mode_OUT);
	GPIOD->OSPEEDR=SetDoubleBits(GPIOD->OSPEEDR,(1<<12)|(1<<13)|(1<<14)|(1<<15),GPIO_Speed_50MHz);
	GPIOD->OTYPER=SetBits(GPIOD->OTYPER,(1<<12)|(1<<13)|(1<<14)|(1<<15),GPIO_OType_PP);
	GPIOD->PUPDR=SetDoubleBits(GPIOD->PUPDR,(1<<12)|(1<<13)|(1<<14)|(1<<15),GPIO_PuPd_UP);
}

