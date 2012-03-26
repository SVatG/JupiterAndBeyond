#ifndef __AUDIO_H__
#define __AUDIO_H__

#include <stdint.h>
#include <stdbool.h>

typedef void AudioCallbackFunction(void *context,int buffer);

#define Audio8000HzSettings 256,5,12,1
#define Audio16000HzSettings 213,2,13,0
#define Audio32000HzSettings 213,2,6,1
#define Audio48000HzSettings 258,3,3,1
#define Audio96000HzSettings 344,2,3,1
#define Audio22050HzSettings 429,4,9,1
#define Audio44100HzSettings 271,2,6,0
#define AudioVGAHSyncSettings 419,2,13,0 // 31475.3606. Actual VGA timer is 31472.4616.

void InitializeAudio(int plln,int pllr,int i2sdiv,int i2sodd);

void MakeNoise();

void AudioOn();
void AudioOff();

void SetAudioVolume(int volume);

void PlayAudioWithCallback(AudioCallbackFunction *callback,void *context);
void StopAudio();

void ProvideAudioBuffer(void *samples,int numsamples);
bool ProvideAudioBufferWithoutBlocking(void *samples,int numsamples);

#endif
