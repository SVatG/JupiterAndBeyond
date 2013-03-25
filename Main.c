#include "System.h"
#include "LED.h"
#include "Button.h"
#include "VGA.h"
#include "RCC.h"
#include "Audio.h"
#include "Random.h"
#include "Utils.h"
#include "BitBin.h"

#include "Global.h"

#include "Fields.h"
#include "LogoShow.h"
#include "Mystery.h"
#include "PixelParticles.h"
#include "Pukezoomer.h"
#include "Rasterize.h"
#include "Starfield.h"
#include "Voxelscape.h"

#include <string.h>

static void AudioCallback(void *context,int buffer);
int16_t *buffers[2]={ (int16_t *)0x2001fa00,(int16_t *)0x2001fc00 };
extern BitBinNote *channels[8];

union GlobalData data;

int main()
{
	InitializeLEDs();

	InitializeSystem();

	SysTick_Config(HCLKFrequency()/100);

	InitializeLEDs();
	InitializeUserButton();

	BitBinSong song;
	InitializeBitBinSong(&song,BitBin22kTable,8,128,channels);
	SetBitBinSongLoops(&song,true);

	InitializeAudio(Audio22050HzSettings);
	PlayAudioWithCallback(AudioCallback,&song);

	InitializeVGA();

	for(;;)
	{
		LogoShow();
		RasterizeInit();
		Rasterize();
		Starfield();
		IDontEvenKnow();
		Voxelscape();
		PixelParticles();
		Pukezoomer();
		Fields();
	}
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
