#include <stdint.h>

#include "LED.h"
#include "Button.h"
#include "Accelerometer.h"
#include "VGA.h"
#include "RCC.h"
#include "Sprites.h"
#include "Random.h"

#include "Graphics/Bitmap.h"
#include "Graphics/Drawing.h"

#include <arm_math.h>

static uint8_t *framebuffer=(uint8_t *)0x20000000;

/*#define RGB(r,g,b) (((r)<<5)|((g)<<2)|(b))

static uint8_t palette[18]=
{
	RGB(7,0,0),RGB(7,2,0),RGB(7,5,0),
	RGB(7,7,0),RGB(5,7,0),RGB(2,7,0),
	RGB(0,7,0),RGB(0,7,1),RGB(0,7,2),
	RGB(0,7,3),RGB(0,5,3),RGB(0,2,3),
	RGB(0,0,3),RGB(2,0,3),RGB(5,0,3),
	RGB(7,0,3),RGB(7,0,2),RGB(7,0,1),
};*/

static uint8_t replacements[256];

int main()
{
	SysTick_Config(HCLKFrequency()/100);

	memset(framebuffer,0,320*240);

	InitializeLEDs();
	InitializeUserButton();
	InitializeAccelerometer();
	DisableAccelerometerPins();
	InitializeVGA(framebuffer);

	Bitmap screen;
	InitializeBitmap(&screen,320,240,320,framebuffer);

	int i=0;

	const int numstars=128;
	struct Star
	{
		int x,y,dx,f;
	} stars[numstars];

	for(int i=0;i<numstars;i++)
	{
		stars[i].x=(RandomInteger()%352-16)<<12;
		stars[i].y=RandomInteger()%240;
		int z=RandomInteger()%100;
		stars[i].dx=2*4096*150/(z+50);
		stars[i].f=6-(z*6)/100;
	}

	const RLEBitmap *frames[7]={ &Star1,&Star2,&Star3,&Star4,&Star5,&Star6,&Star7 };

	for(;;)
	{
		WaitVBL();

		ClearBitmap(&screen);

		SetLEDs(1<<((i++/3)&3));

		for(int i=0;i<numstars;i++)
		{
			DrawRLEBitmap(&screen,frames[stars[i].f],stars[i].x>>12,stars[i].y);
			stars[i].x-=stars[i].dx;
			if(stars[i].x<=-16<<12)
			{
				stars[i].x=(320+16)<<12;
				stars[i].y=RandomInteger()%240;
			}
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
