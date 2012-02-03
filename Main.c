#include <stdint.h>

#include "LED.h"
#include "Button.h"
#include "Accelerometer.h"
#include "VGA.h"
#include "Bits.h"

#include "arm_math.h"

void Delay(uint32_t time);

static uint8_t framebuffer[320*240];

int main()
{
	RCC_ClocksTypeDef RCC_Clocks;
	RCC_GetClocksFreq(&RCC_Clocks);
	SysTick_Config(RCC_Clocks.HCLK_Frequency/100);

	for(int y=0;y<240;y++)
	for(int x=0;x<320;x++)
	{
		int dx=x-159;
		int dy=y-119;
		int r2=dx*dx+dy*dy;
		q15_t r;
		arm_sqrt_q15(dx*dx+dy*dy,&r);
		framebuffer[x+y*320]=r>>6;
	}

	InitializeLEDs();
	InitializeUserButton();
	InitializeAccelerometer();
	DisableAccelerometerPins();
	InitializeVGA(framebuffer);

	SetLEDs(0x01);

/*	SetAccelerometerMainConfig(
		LIS302DL_LOWPOWERMODE_ACTIVE|
		LIS302DL_DATARATE_100|
		LIS302DL_XYZ_ENABLE|
		LIS302DL_FULLSCALE_2_3|
		LIS302DL_SELFTEST_NORMAL);

	SetLEDs(0x03);

	// Required delay for the MEMS Accelerometer: Turn-on time = 3/Output data Rate = 3/100 = 30ms
    Delay(30);

	SetLEDs(0x07);

	SetAccelerometerFilterConfig(
		LIS302DL_FILTEREDDATASELECTION_BYPASSED|
    	LIS302DL_HIGHPASSFILTER_LEVEL_1|
    	LIS302DL_HIGHPASSFILTERINTERRUPT_1_2);*/

	SetLEDs(0x0f);

	int8_t zero[3];
	ReadRawAccelerometerData(zero);

	int32_t x=0,y=0;

	int i=0;

	for(;;)
	{
		WaitVBL();

		SetLEDs(1<<((i++/3)&3));

		for(int j=0;j<sizeof(framebuffer);j++) framebuffer[j]--;
	}
}

extern volatile uint32_t SysTickCounter;
void Delay(uint32_t time)
{
	uint32_t end=SysTickCounter+time;
	while(SysTickCounter!=end);
}
