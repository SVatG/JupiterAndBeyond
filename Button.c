#include "Button.h"
#include "Bits.h"

#include <stm32f4xx_exti.h>

void InitializeUserButton()
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG,ENABLE);

	GPIOA->MODER=SetDoubleBits(GPIOA->MODER,(1<<0),GPIO_Mode_IN);
	GPIOA->PUPDR=SetDoubleBits(GPIOA->PUPDR,(1<<0),GPIO_PuPd_NOPULL);
}

void EnableUserButtonInterrupt()
{
	EXTI_InitTypeDef EXTI_InitStructure;
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA,EXTI_PinSource0);

	EXTI_Init(&(EXTI_InitTypeDef){
		.EXTI_Line=EXTI_Line0,
		.EXTI_Mode=EXTI_Mode_Interrupt,
		.EXTI_Trigger=EXTI_Trigger_Rising,
		.EXTI_LineCmd=ENABLE,
	});

	NVIC_Init(&(NVIC_InitTypeDef){
		.NVIC_IRQChannel=EXTI0_IRQn,
		.NVIC_IRQChannelPreemptionPriority=0x0f, // Lowest priority
		.NVIC_IRQChannelSubPriority=0x0f,
		.NVIC_IRQChannelCmd=ENABLE,
	});
}

