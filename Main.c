#include <stdint.h>

#include "LED.h"
#include "Button.h"
#include "Accelerometer.h"

void Delay(uint32_t time);

int main()
{
	InitializeLEDs();
	InitializeUserButton();
	InitializeAccelerometer();

	RCC_ClocksTypeDef RCC_Clocks;
	RCC_GetClocksFreq(&RCC_Clocks);
	SysTick_Config(RCC_Clocks.HCLK_Frequency/100);
	SetLEDs(0x01);

	SetAccelerometerMainConfig(
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
    	LIS302DL_HIGHPASSFILTERINTERRUPT_1_2);

	SetLEDs(0x0f);

	if(!PingAccelerometer()) for(;;);

	int8_t zero[3];
	ReadRawAccelerometerData(zero);

	for(;;)
	{
		int8_t components[3];
		ReadRawAccelerometerData(components);

		if(components[0]>zero[0]) { TurnOnLEDs(8); TurnOffLEDs(2); }
		else { TurnOnLEDs(2); TurnOffLEDs(8); }

		if(components[1]>zero[1]) { TurnOnLEDs(4); TurnOffLEDs(1); }
		else { TurnOnLEDs(1); TurnOffLEDs(4); }

		Delay(10);
	}
}

extern volatile uint32_t SysTickCounter;
void Delay(uint32_t time)
{
	uint32_t end=SysTickCounter+time;
	while(SysTickCounter!=end);
}
