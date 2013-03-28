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
#include "TorusTunnel.h"
#include "Voxelscape.h"
#include "Greets.h"
#include "Credits.h"
#include "Environment.h"

#include <string.h>

static void AudioCallback(void *context,int buffer);
int16_t *buffers[2]={ (int16_t *)0x2001fa00,(int16_t *)0x2001fc00 };
extern BitBinNote *channels[8];
BitBinSong* songp;

union GlobalData data;

int main()
{
	InitializeLEDs();

	InitializeSystem();

	SysTick_Config(HCLKFrequency()/100);

	InitializeLEDs();
	InitializeUserButton();

	BitBinSong song;
	InitializeBitBinSong(&song,BitBin22kTable,8,1952,channels);
	SetBitBinSongLoops(&song,true);
        songp = &song;
        
	InitializeAudio(Audio22050HzSettings);
        SetAudioVolume(0xAA);
//	InitializeAudio(Audio44100HzSettings);
	PlayAudioWithCallback(AudioCallback,&song);

	InitializeVGA();

//         Part                    Length          Effect                                  Next Row total
//         --------------------------------------------------------------------------------------------------
//         INTRO
//         256             Metablobs() -> STARFIELD                256
//         128             Pukezoomer(), Fadeout                   384
//         128             LogoShow() Titlecard                    512
//         160             Reflector() -> add sync!!               672
//         Ticking
//         128             TorusTunnel()                           800
//         Happy
//         128             Voxelscape() -> OVERLAY                 928
//         128             Rasterize() -> OVERLAY                  1056
//         128             Fields() -> OVERLAY                     1184
//         Bubbly
//         117             PixelParticles() -> add sync?           1301
//         Mystery
//         240             Starfield();                            1541
//         128             Credits();                              1669
//         256             IDontEvenKnow();                        1925
        
        
        
        
        
	for(;;)
	{
                Pukezoomer();
		Environment();
                Metablobs();
                Pukezoomer();
                LogoShow();
                //Reflector();
                TorusTunnel();
                Voxelscape();
                RasterizeInit();
                Rasterize();
                Fields();
                PixelParticles();
                Starfield();
                Credits();
                IDontEvenKnow();
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
