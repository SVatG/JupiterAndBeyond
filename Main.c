#include <stdint.h>

#include "System.h"
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

static uint32_t sqrti(uint32_t n);

int main()
{
	InitializeLEDs();
	SetLEDs(0x01);

	InitializeSystem();

	SetLEDs(0x03);

	SysTick_Config(HCLKFrequency()/100);

	uint8_t *framebuffer1=(uint8_t *)0x20000000;
	uint8_t *framebuffer2=(uint8_t *)0x20010000;
	memset(framebuffer1,0,320*200);
	memset(framebuffer2,0,320*200);

	InitializeLEDs();
	InitializeUserButton();
	InitializeAccelerometer();
	DisableAccelerometerPins();

	SetLEDs(0x07);
	IntializeVGAScreenMode320x200(framebuffer1);
	SetLEDs(0x0f);

	const int numstars=450;
	struct Star
	{
		int x,y,dx,f;
	} stars[numstars];

	for(int i=0;i<numstars;i++)
	{
		stars[i].x=(RandomInteger()%352-16)<<12;
		stars[i].y=RandomInteger()%200;

		int z=sqrti((numstars-1-i)*numstars)*1000/numstars;
		stars[i].dx=6000*1200/(z+200);

		stars[i].f=(6-(z*7)/1000)+(RandomInteger()%6)*7;
	}

	const RLEBitmap *sprites[7*6]={
		&Star1_0,&Star2_0,&Star3_0,&Star4_0,&Star5_0,&Star6_0,&Star7_0,
		&Star1_1,&Star2_1,&Star3_1,&Star4_1,&Star5_1,&Star6_1,&Star7_1,
		&Star1_2,&Star2_2,&Star3_2,&Star4_2,&Star5_2,&Star6_2,&Star7_2,
		&Star1_3,&Star2_3,&Star3_3,&Star4_3,&Star5_3,&Star6_3,&Star7_3,
		&Star1_4,&Star2_4,&Star3_4,&Star4_4,&Star5_4,&Star6_4,&Star7_4,
		&Star1_5,&Star2_5,&Star3_5,&Star4_5,&Star5_5,&Star6_5,&Star7_5,
	};

	Bitmap frame1,frame2;
	InitializeBitmap(&frame1,320,200,320,framebuffer1);
	InitializeBitmap(&frame2,320,200,320,framebuffer2);

	int frame=0;

	for(;;)
	{
		WaitVBL();

		Bitmap *currframe;
		if(frame&1) { currframe=&frame2; SetFrameBuffer(framebuffer1); }
		else { currframe=&frame1; SetFrameBuffer(framebuffer2); }

		ClearBitmap(currframe);

		SetLEDs(1<<((frame/3)&3));

		for(int i=0;i<numstars;i++)
		{
			DrawRLEBitmap(currframe,sprites[stars[i].f],
			(stars[i].x>>12)-16,stars[i].y-16);

			stars[i].x-=stars[i].dx;
			if(stars[i].x<=-16<<12)
			{
				stars[i].x=(320+16)<<12;
				stars[i].y=RandomInteger()%200;
				stars[i].f=(stars[i].f%7)+(RandomInteger()%6)*7;
			}
		}

		frame++;
	}
}



static uint32_t sqrti(uint32_t n)
{
	uint32_t s,t;

	#define sqrtBit(k) \
	t = s+(1UL<<(k-1)); t <<= k+1; if (n >= t) { n -= t; s |= 1UL<<k; }

	s=0;
	if(n>=1<<30) { n-=1<<30; s=1<<15; }
	sqrtBit(14); sqrtBit(13); sqrtBit(12); sqrtBit(11); sqrtBit(10);
	sqrtBit(9); sqrtBit(8); sqrtBit(7); sqrtBit(6); sqrtBit(5);
	sqrtBit(4); sqrtBit(3); sqrtBit(2); sqrtBit(1);
	if(n>s<<1) s|=1;

	#undef sqrtBit

	return s;
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
