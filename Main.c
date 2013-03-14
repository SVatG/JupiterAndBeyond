#include <stdint.h>

#include "System.h"
#include "LED.h"
#include "Button.h"
#include "Accelerometer.h"
#include "VGA.h"
#include "RCC.h"
#include "Audio.h"
#include "Sprites.h"
#include "Random.h"
#include "Utils.h"
#include "BitBin.h"

#include "Fields.h"

#include "Graphics/Bitmap.h"
#include "Graphics/Drawing.h"

#include <arm_math.h>

static void PlasmaZoom();
static void Fields();
static void Rotozoom();
static void Starfield();
static void InitializeLEDFlow();
static void RunLEDFlow();
static void Epileptor();

static void AudioCallback(void *context,int buffer);
int16_t *buffers[2]={ (int16_t *)0x2001fa00,(int16_t *)0x2001fc00 };
extern BitBinNote *channels[8];

int main()
{
	InitializeLEDs();

	InitializeSystem();

	SysTick_Config(HCLKFrequency()/100);

	InitializeLEDs();
	InitializeUserButton();
	InitializeAccelerometer();

	BitBinSong song;
	InitializeBitBinSong(&song,BitBin22kTable,8,128,channels);
	SetBitBinSongLoops(&song,true);

	InitializeAudio(Audio22050HzSettings);
	PlayAudioWithCallback(AudioCallback,&song);

	InitializeVGA();

	for(;;)
	{
		PlasmaZoom();
		Fields();
		Starfield();
		Rotozoom();
		Epileptor();
	}
}



static void PlasmaZoom()
{
	uint8_t *framebuffer1=(uint8_t *)0x20000000;
	uint8_t *framebuffer2=(uint8_t *)0x20010000;
	memset(framebuffer1,0,320*200);
	memset(framebuffer2,0,320*200);

	SetVGAScreenMode320x200(framebuffer1);

	static uint8_t nextcolour[256];
	for(int i=0;i<256;i++)
	{
		nextcolour[i]=RGB(ExtractRed(i)+0x20,ExtractGreen(i)+0x20,ExtractBlue(i)+0x40);
	}

	int t=0;
	while(!UserButtonState())
	{
		WaitVBL();

		uint8_t *source,*destination;
		if(t&1)
		{
			source=framebuffer1;
			destination=framebuffer2;
			SetFrameBuffer(framebuffer1);
		}
		else
		{
			source=framebuffer2;
			destination=framebuffer1;
			SetFrameBuffer(framebuffer2);
		}

		//for(int i=0;i<100;i++) source[RandomInteger()%(320*200)]=RandomInteger();

		uint8_t *destination2=destination;
		uint32_t *destination32=(uint32_t *)destination;

		#define Ratio 16

		int xskips[320/Ratio+1]={0};
		int yskips[200/Ratio+1]={0};

		for(int i=0;i<320/Ratio;i++) xskips[i]=i*Ratio+RandomInteger()%Ratio;
		for(int i=0;i<200/Ratio;i++) yskips[i]=i*Ratio+RandomInteger()%Ratio;
		if(yskips[0]==0) yskips[0]=1;
		if(yskips[200/Ratio-1]==199) yskips[200/Ratio-1]=200;

		int xcenter=320/2;//+isin(t*20)/500;
		int ycenter=200/2;//+icos(t*20)/500;

		int sourcerow=ycenter/Ratio;

		int *yskipptr=yskips;
		for(int y=0;y<200;y++)
		{
			if(y==*yskipptr) { yskipptr++; destination+=320; continue; }

			uint8_t *sourceptr=&source[sourcerow*320+xcenter/Ratio];
			int *xskipptr=xskips;

			for(int x=0;x<320;x++)
			{
				if(x!=*xskipptr)
				{
					*destination++=*sourceptr++;
				}
				else
				{
					xskipptr++;

					uint8_t p1=sourceptr[-1];
					uint8_t p2=sourceptr[0];
					uint8_t halfp1=(p1>>1)&PixelAllButHighBits;
					uint8_t halfp2=(p2>>1)&PixelAllButHighBits;
					uint8_t carry=p1&p2&PixelLowBits;
					uint8_t new=halfp1+halfp2+carry;
					uint32_t r=RandomInteger();
					//r&=r>>16;
					//r&=r>>8;
					if(r&1) new=nextcolour[new];
					*destination++=new;
				}
			}

			sourcerow++;
		}

		yskipptr=yskips;
		for(int y=0;y<200;y++)
		{
			if(y!=*yskipptr) { destination32+=320/4; continue; }
			yskipptr++;

			uint32_t *sourceptr1=(uint32_t *)&destination32[-1*320/4];
			uint32_t *sourceptr2=(uint32_t *)&destination32[1*320/4];

			for(int i=0;i<320/4;i++)
			{
				uint32_t p1=*sourceptr1++;
				uint32_t p2=*sourceptr2++;
				uint32_t halfp1=(p1>>1)&((uint32_t)PixelAllButHighBits*0x01010101);
				uint32_t halfp2=(p2>>1)&((uint32_t)PixelAllButHighBits*0x01010101);
				uint32_t carry=p1&p2&(PixelLowBits*0x01010101);
				uint32_t new=halfp1+halfp2+carry;
				uint32_t r=RandomInteger();
				//r&=RandomInteger();
if(r&1)
{
	uint8_t c=(new>>0)&0xff;
	uint8_t nextc=nextcolour[c];
	new=(new&0xffffff00)|(c<<0);
}
if(r&2)
{
	uint8_t c=(new>>8)&0xff;
	uint8_t nextc=nextcolour[c];
	new=(new&0xffffff00)|(c<<8);
}
if(r&4)
{
	uint8_t c=(new>>16)&0xff;
	uint8_t nextc=nextcolour[c];
	new=(new&0xffffff00)|(c<<16);
}
if(r&8)
{
	uint8_t c=(new>>24)&0xff;
	uint8_t nextc=nextcolour[c];
	new=(new&0xffffff00)|(c<<24);
}

				*destination32++=new;
			}
		}

		for(int y=0;y<200;y++)
		for(int x=320/2-8;x<320/2+8;x++)
		{
			destination2[x+y*320]=0;
		}

		t++;
	}

	while(UserButtonState());
}



static void Fields()
{
	InitializeField();

	uint8_t *framebuffer1=(uint8_t *)0x20000000;
	uint8_t *framebuffer2=(uint8_t *)0x20010000;
	memset(framebuffer1,0,212*133);
	memset(framebuffer2,0,212*133);

	SetVGAScreenMode212x133(framebuffer1);

	for(int t=0;;t++)
	{
		if(UserButtonState()) break;

		SetLEDs(t>>3);

		WaitVBL();

		uint8_t *framebuffer;
		if(t&1)
		{
			framebuffer=framebuffer2;
			SetFrameBuffer(framebuffer1);
		}
		else
		{
			framebuffer=framebuffer1;
			SetFrameBuffer(framebuffer2);
		}

		switch((t>>8)&3)
		{
			case 0: DrawField(framebuffer,t); break;
			case 1: DrawField2(framebuffer,t); break;
			case 2: DrawField3(framebuffer,t); break;
			case 3: DrawField4(framebuffer,t); break;
		}
	}

	while(UserButtonState());
}





static void AudioCallback(void *context,int buffer)
{
	BitBinSong *song=context;

	int16_t *samples=buffers[buffer];
	RenderBitBinSamples(song,128,samples);
	for(int i=128;i>=0;i--)
	{
		samples[2*i+0]=samples[i];
		samples[2*i+1]=samples[i];
	}

	ProvideAudioBuffer(samples,256);
}




static void RotozoomHSyncHandler();

static volatile int32_t x0,y0,dx,dy;
static volatile uint32_t Pos,Delta;
static uint8_t linetexture[480];

static uint32_t PackCoordinates(int32_t x,int32_t y)
{
	x&=0x3ffff;
	y&=0x3ff80;
	return (y>>(12-17+6))|(x<<20)|(x>>12);
}

static void Rotozoom()
{
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

	SetVGAHorizontalSync31kHz(RotozoomHSyncHandler);

	SetLEDs(0x5);

	while(!UserButtonState())
	{
		WaitVBL();
		int t=VGAFrameCounter();

		SetLEDs(1<<((t/3)&3));

		int32_t angle=isin(t*9);
		int32_t scale=icos(t*17)+Fix(2);

		dx=imul(scale,icos(angle));
		dy=imul(scale,isin(angle));

		x0=-dx*320-dy*240;
		y0=-(dy&0xffffff80)*320+dx*240;
		Delta=PackCoordinates(dx,dy);

		for(int y=0;y<480;y++)
		{
			int pos=icos(t*20);
			linetexture[y]=(31*(isin(y*6+pos)+Fix(1))/8192)&31;
		}
	}

	while(UserButtonState());
}

static void RotozoomHSyncHandler()
{
	int line=HandleVGAHSync400();
	if(line<0) return;

	register uint32_t r0 __asm__("r0")=Pos;
	register uint32_t r1 __asm__("r1")=Delta;
	register uint32_t r2 __asm__("r2")=0x1f83f;
	register uint32_t r3 __asm__("r3")=0x20000000+(linetexture[line]<<6);
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
	#undef P

	".end:	\n"
	:
	: "r" (r0), "r" (r1), "r" (r2), "r" (r3), "r" (r4)
	:"r5","r6");

	SetVGASignalToBlack();

	x0+=dy;
	y0-=dx;
	Pos=PackCoordinates(x0,y0);
}





static void Starfield()
{
	uint8_t *framebuffer1=(uint8_t *)0x20000000;
	uint8_t *framebuffer2=(uint8_t *)0x20010000;
	memset(framebuffer1,0,320*200);
	memset(framebuffer2,0,320*200);

	SetVGAScreenMode320x200(framebuffer1);

	InitializeLEDFlow();

	#define NumberOfStars 1050
	static struct Star
	{
		int x,y,dx,f;
	} stars[NumberOfStars];

	for(int i=0;i<NumberOfStars;i++)
	{
		stars[i].x=(RandomInteger()%352-16)<<12;
		stars[i].y=RandomInteger()%200;

		int z=sqrti((NumberOfStars-1-i)*NumberOfStars)*1000/NumberOfStars;
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

		RunLEDFlow();

		Bitmap *currframe;
		if(frame&1) { currframe=&frame2; SetFrameBuffer(framebuffer1); }
		else { currframe=&frame1; SetFrameBuffer(framebuffer2); }

		ClearBitmap(currframe);

		for(int i=0;i<NumberOfStars;i++)
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





static int8_t zero[3];
static int32_t x=0,y=0;

static void InitializeLEDFlow()
{
	WaitVBL();

	SetLEDs(0x0f);

	SetAccelerometerMainConfig(
		LIS302DL_LOWPOWERMODE_ACTIVE|
		LIS302DL_DATARATE_100|
		LIS302DL_XYZ_ENABLE|
		LIS302DL_FULLSCALE_2_3|
		LIS302DL_SELFTEST_NORMAL);

	for(int i=0;i<20;i++) WaitVBL();

	SetAccelerometerFilterConfig(
		LIS302DL_FILTEREDDATASELECTION_BYPASSED|
    	LIS302DL_HIGHPASSFILTER_LEVEL_1|
    	LIS302DL_HIGHPASSFILTERINTERRUPT_1_2);

	SetLEDs(0);

	if(!PingAccelerometer()) for(;;) SetLEDs(0x05);

	ReadRawAccelerometerData(zero);
}

static void RunLEDFlow()
{
	int8_t components[3];
	ReadRawAccelerometerData(components);

	int32_t dx=components[0]-zero[0];
	int32_t dy=components[1]-zero[1];
	int32_t r=sqrti(dx*dx+dy*dy);
	dx=(dx<<12)/r;
	dy=(dy<<12)/r;

	x+=r*25;

//	x+=components[0]-zero[0];
//	y+=components[1]-zero[1];

	int leds=0;
	leds|=(((x+dx)>>14)&1)<<1;
	leds|=(((x-dx)>>14)&1)<<3;
	leds|=(((x+dy)>>14)&1)<<0;
	leds|=(((x-dy)>>14)&1)<<2;

	SetLEDs(leds);
}




static void DrawBlob(Bitmap *bitmap,int x0,int y0,int c);

static void Epileptor()
{
	static uint8_t palette[18]=
	{
		RawRGB(7,0,0),RawRGB(7,2,0),RawRGB(7,5,0),
		RawRGB(7,7,0),RawRGB(5,7,0),RawRGB(2,7,0),
		RawRGB(0,7,0),RawRGB(0,7,1),RawRGB(0,7,2),
		RawRGB(0,7,3),RawRGB(0,5,3),RawRGB(0,2,3),
		RawRGB(0,0,3),RawRGB(2,0,3),RawRGB(5,0,3),
		RawRGB(7,0,3),RawRGB(7,0,2),RawRGB(7,0,1),
	};

	static uint8_t replacements[256];

	uint8_t *framebuffer1=(uint8_t *)0x20000000;
	uint8_t *framebuffer2=(uint8_t *)0x20010000;
	memset(framebuffer1,0,320*200);
	memset(framebuffer2,0,320*200);

	for(int i=0;i<sizeof(palette);i++)
	{
		replacements[palette[i]]=palette[(i+1)%sizeof(palette)];
	}

	SetVGAScreenMode320x200(framebuffer1);

	Bitmap frame1,frame2;
	InitializeBitmap(&frame1,320,200,320,framebuffer1);
	InitializeBitmap(&frame2,320,200,320,framebuffer2);

	int t=0;

	while(!UserButtonState())
	{
		WaitVBL();

		Bitmap *currframe;
		if(t&1)
		{
			currframe=&frame2;
			SetFrameBuffer(framebuffer1);

			for(int j=0;j<320*200;j++)
			framebuffer2[j]=replacements[framebuffer1[j]];
		}
		else
		{
			currframe=&frame1;
			SetFrameBuffer(framebuffer2);

			for(int j=0;j<320*200;j++)
			framebuffer1[j]=replacements[framebuffer2[j]];
		}

		SetLEDs(1<<((t/3)&3));

		int32_t sina=isin(t*3);
		int32_t sinb=isin(10*t/11);

		for(int j=0;j<4;j++)
		{
			int32_t sin1=isin(sina+j*1024);
			int32_t cos1=icos(sina+j*1024);
			int32_t sin2=isin(sinb/3+j*1024+1421);
			int32_t cos2=icos(sinb/3+j*1024+1421);
			int32_t sin3=isin(t*23);
			int32_t sin4=isin(t*14);

			int x=160+((imul(sin1,sin4)+imul(sin2,sin3)/2)>>5);
			int y=100+((imul(cos1,sin4)+imul(cos2,sin3)/2)>>5);

			DrawBlob(currframe,x,y,palette[0]);
		}

		t++;
	}

	while(UserButtonState());
}

static void DrawBlob(Bitmap *bitmap,int x0,int y0,int c)
{
	static const int rowlengths[32]=
	{
		6,12,16,20,  22,24,26,26, 28,28,30,30, 30,32,32,32,
		32,32,32,30, 30,30,28,28, 26,26,24,22, 20,16,12,6
	};

	for(int row=0;row<32;row++)
	DrawHorizontalLine(bitmap,x0-rowlengths[row]/2,y0+row-16,rowlengths[row],c);
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
