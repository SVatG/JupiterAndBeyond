#include <stdint.h>

#include "LED.h"
#include "Button.h"
#include "Accelerometer.h"
#include "VGA.h"

#include <stm32f4xx_rcc.h>
#include <arm_math.h>

void DrawBlob(uint8_t *framebuffer,int x0,int y0,int c);

void Delay(uint32_t time);

static uint8_t framebuffer[320*240];

#define RGB(r,g,b) (((r)<<5)|((g)<<2)|(b))

static uint8_t palette[18]=
{
	RGB(7,0,0),RGB(7,2,0),RGB(7,5,0),
	RGB(7,7,0),RGB(5,7,0),RGB(2,7,0),
	RGB(0,7,0),RGB(0,7,1),RGB(0,7,2),
	RGB(0,7,3),RGB(0,5,3),RGB(0,2,3),
	RGB(0,0,3),RGB(2,0,3),RGB(5,0,3),
	RGB(7,0,3),RGB(7,0,2),RGB(7,0,1),
};

static uint8_t replacements[256];

int main()
{
	RCC_ClocksTypeDef RCC_Clocks;
	RCC_GetClocksFreq(&RCC_Clocks);
	SysTick_Config(RCC_Clocks.HCLK_Frequency/100);

	for(int i=0;i<sizeof(palette);i++)
	{
		replacements[palette[i]]=palette[(i+1)%sizeof(palette)];
	}

	memset(framebuffer,0,sizeof(framebuffer));

	InitializeLEDs();
	InitializeUserButton();
	InitializeAccelerometer();
	DisableAccelerometerPins();
	InitializeVGA(framebuffer);

	int i=0;

	for(;;)
	{
		WaitVBL();

		SetLEDs(1<<((i++/3)&3));

		for(int j=0;j<sizeof(framebuffer);j++)
		framebuffer[j]=replacements[framebuffer[j]];

		for(int j=0;j<4;j++)
		{
			q31_t sina,cosa;
			arm_sin_cos_q31(i*2234934+0x95122323,&sina,&cosa);
			q31_t sinb,cosb;
			arm_sin_cos_q31(i*788929+0x11322323,&sinb,&cosb);

			q31_t sin1,cos1;
			arm_sin_cos_q31(i*(sina/78)+(j<<30),&sin1,&cos1);
			q31_t sin2,cos2;
			arm_sin_cos_q31(i*(sinb/123)+(j<<30)+0x12424234,&sin2,&cos2);
			q31_t sin3,cos3;
			arm_sin_cos_q31(i*7234904+0x53224234,&sin3,&cos3);
			q31_t sin4,cos4;
			arm_sin_cos_q31(i*3123904+0xd2324234,&sin4,&cos4);

			int x=160-16+((sin1>>20)*(sin4>>20)>>15)+((sin2>>20)*(sin3>>20)>>16);
			int y=120-16+((cos1>>20)*(sin4>>20)>>15)+((cos2>>20)*(sin3>>20)>>16);

			DrawBlob(framebuffer,x,y,palette[0]);
		}
	}
}

void DrawBlob(uint8_t *framebuffer,int x0,int y0,int c)
{
	static const int rowlengths[32]=
	{
		6,12,16,20,  22,24,26,26, 28,28,30,30, 30,32,32,32,
		32,32,32,30, 30,30,28,28, 26,26,24,22, 20,16,12,6
	};

	for(int row=0;row<32;row++)
	{
if(y0+row<0) continue;
if(y0+row>=240) continue;
		int startx=16-rowlengths[row]/2;
		for(int x=0;x<rowlengths[row];x++)
		{
if(x0+startx+x<0) continue;
if(x0+startx+x>=320) continue;
			framebuffer[x0+startx+x+(y0+row)*320]=c;
		}
	}
}

volatile uint32_t SysTickCounter=0;

void Delay(uint32_t time)
{
	uint32_t end=SysTickCounter+time;
	while(SysTickCounter!=end);
}

void SysTick_Handler()
{  
	SysTickCounter++;
}
