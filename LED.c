#include "LED.h"

void InitializeLEDs()
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD,ENABLE);

	GPIOD->MODER&=~(0xff<<24);
	GPIOD->MODER|=(GPIO_Mode_OUT*0x55)<<24;

	GPIOD->OSPEEDR&=~(0xff<<24);
	GPIOD->OSPEEDR|=(GPIO_Speed_50MHz*0x55)<<24;

	GPIOD->OTYPER&=~(0x0f<<12);
	GPIOD->OTYPER|=(GPIO_OType_PP*0x0f)<<12;

	GPIOD->PUPDR&=~(0xff<<24);
	GPIOD->PUPDR|=(GPIO_PuPd_UP*0x55)<<24;
}

