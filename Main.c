#include <stdint.h>

#include "System.h"
#include "LED.h"
#include "Button.h"
#include "Accelerometer.h"
#include "VGA.h"
#include "RCC.h"
#include "Sprites.h"
#include "Random.h"
#include "Utils.h"

#include "Graphics/Bitmap.h"
#include "Graphics/Drawing.h"

#include <arm_math.h>

static void Rotozoom();
static void Starfield();

static uint32_t sqrti(uint32_t n);

int main()
{
	InitializeLEDs();
	SetLEDs(0x01);

	InitializeSystem();

	SetLEDs(0x03);

	SysTick_Config(HCLKFrequency()/100);

	InitializeLEDs();
	InitializeUserButton();
	InitializeAccelerometer();
	DisableAccelerometerPins();

	for(;;)
	{
		Starfield();
		Rotozoom();
	}
}




static void RotozoomHSYNCHandler();

static uint32_t Line;
static volatile uint32_t Frame;
static volatile int32_t x0,y0,dx,dy;

static uint32_t PackCoordinates(int32_t x,int32_t y)
{
	x&=0x3ffff;
	y&=0x3ff80;
	return (y>>(12-17+6))|(x<<20)|(x>>12);
}

static volatile uint32_t Pos,Delta;
static uint8_t linetexture[480];

static void Rotozoom()
{
	Line=0;
	Frame=0;

	memset(linetexture,0,sizeof(linetexture));

	uint8_t *texture=(uint8_t *)0x20000000;

	for(int i=0;i<32;i++)
	{
		for(int y=0;y<64;y++)
		for(int x=0;x<64;x++)
		{
			int dx=(2*x+1)-64;
			int dy=(2*y+1)-64;
			int r=sqrti(dx*dx+dy*dy);

			int red=0,green=0,blue=0;

			if(r<2*i)
			{
				red=(2*i-r)/3;
				if(red>7)
				{
					green=red-8;
					red=7;
					if(green>7)
					{
						blue=green-8;
						green=7;

						if(blue>3) blue=3;
					}
				}
			}

			int offset=(y<<(17-6))|(i<<6)|x;
			if(offset<0x20000-0x200) texture[offset]=(red<<5)|(green<<2)|(blue);
		}
	}

	InitializeVGAPort();
	InitializeVGAHorizontalSync31kHz(RotozoomHSYNCHandler);

	SetLEDs(0x5);

	while(!UserButtonState())
	{
		uint32_t lastframe=Frame;
		while(Frame==lastframe); // Wait for VBL
		int t=Frame;

		int32_t angle=isin(t*9);
		int32_t scale=icos(t*17)+Fix(2);

		dx=imul(scale,icos(angle));
		dy=imul(scale,isin(angle));

		//dx&=0xffffff80;
		dy&=0xffffff80;

		x0=-dx*320-dy*240;
		y0=-dy*320+dx*240;
		Delta=PackCoordinates(dx,dy);

		for(int y=0;y<480;y++)
		{
			int pos=icos(t*20);
			linetexture[y]=(31*(isin(y*6+pos)+Fix(1))/8192)&31;
		}
	}

	while(UserButtonState());
}

static void RotozoomHSYNCHandler()
{
	switch(VGAHorizontalSyncInterruptType())
	{
		case VGAHorizontalSyncStartInterrupt:
			LowerVGAHSYNCLine();

			x0+=dy;
			y0-=dx;
			Pos=PackCoordinates(x0,y0);
		break;

		case VGAHorizontalSyncEndInterrupt:
			RaiseVGAHSYNCLine();
		break;

		case VGAVideoStartInterrupt:
			if(Line<480)
			{
				register uint32_t r0 __asm__("r0")=Pos;
				register uint32_t r1 __asm__("r1")=Delta;
				register uint32_t r2 __asm__("r2")=0x1f83f;
				register uint32_t r3 __asm__("r3")=0x20000000+(linetexture[Line]<<6);
				register uint32_t r4 __asm__("r4")=0x40021015;
				#define P \
				"	adcs	r0,r1		\n" \
				"	and		r5,r0,r2	\n" \
				"	ldrb	r6,[r3,r5]	\n" \
				"	strb	r6,[r4]		\n"
				
				__asm__ volatile(
				"	b		.start		\n"
				"	.align 4	\n"
				".start:	\n"
				P P P P  P P P P  P P P P  P P P P  P P P P  P P P P  P P P P  P P P P 
				P P P P  P P P P  P P P P  P P P P  P P P P  P P P P  P P P P  P P P P 
				P P P P  P P P P  P P P P  P P P P  P P P P  P P P P  P P P P  P P P P 
				P P P P  P P P P  P P P P  P P P P  P P P P  P P P P  P P P P  P P P P 

				P P P P  P P P P  P P P P  P P P P  P P P P  P P P P  P P P P  P P P P 
				P P P P  P P P P  P P P P  P P P P  P P P P  P P P P  P P P P  P P P P 
				P P P P  P P P P  P P P P  P P P P  P P P P  P P P P  P P P P  P P P P 
				P P P P  P P P P  P P P P  P P P P  P P P P  P P P P  P P P P  P P P P 

				P P P P  P P P P  P P P P  P P P P  P P P P  P P P P  P P P P  P P P P 
				P P P P  P P P P  P P P P  P P P P  P P P P  P P P P  P P P P  P P P P 
				P P P P  P P P P  P P P P  P P P P  P P P P  P P P P  P P P P  P P P P 
				P P P P  P P P P  P P P P  P P P P  P P P P  P P P P  P P P P  P P P P 

				P P P P  P P P P  P P P P  P P P P  P P P P  P P P P  P P P P  P P P P 
				P P P P  P P P P  P P P P  P P P P  P P P P  P P P P  P P P P  P P P P 
				P P P P  P P P P  P P P P  P P P P  P P P P  P P P P  P P P P  P P P P 
				P P P P  P P P P  P P P P  P P P P  P P P P  P P P P  P P P P  P P P P 
				
				P P P P  P P P P  P P P P  P P P P  P P P P  P P P P  P P P P  P P P P 
				P P P P  P P P P  P P P P  P P P P  P P P P  P P P P  P P P P  P P P P 
				P P P P  P P P P  P P P P  P P P P  P P P P  P P P P  P P P P  P P P P 
				P P P P  P P P P  P P P P  P P P P  P P P P  P P P P  P P P P  P P P P 

			
				".end:	\n"
				:
				: "r" (r0), "r" (r1), "r" (r2), "r" (r3), "r" (r4)
				:"r5","r6");

				((uint8_t *)&GPIOE->ODR)[1]=0;
			}
			else if(Line==480)
			{
				Frame++;
			}
			else if(Line==490)
			{
				LowerVGAVSYNCLine();
			}
			else if(Line==492)
			{
				RaiseVGAVSYNCLine();
			}
			else if(Line==524)
			{
				Line=-1;
			}
			Line++;
		break;
	}
}





static void Starfield()
{
	uint8_t *framebuffer1=(uint8_t *)0x20000000;
	uint8_t *framebuffer2=(uint8_t *)0x20010000;
	memset(framebuffer1,0,320*200);
	memset(framebuffer2,0,320*200);

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

	while(!UserButtonState())
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

	while(UserButtonState());
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
