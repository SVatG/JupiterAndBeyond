#include "LED.h"
#include "GPIO.h"

#include <stm32f4xx_rcc.h>

void InitializeLEDs()
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD,ENABLE);

	SetGPIOOutputMode(GPIOD,(1<<12)|(1<<13)|(1<<14)|(1<<15));
	SetGPIOPushPullOutput(GPIOD,(1<<12)|(1<<13)|(1<<14)|(1<<15));
	SetGPIOSpeed50MHz(GPIOD,(1<<12)|(1<<13)|(1<<14)|(1<<15));
	SetGPIOPullUpResistor(GPIOD,(1<<12)|(1<<13)|(1<<14)|(1<<15));
}

